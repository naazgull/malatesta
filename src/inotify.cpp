#include <malatesta/inotify.h>
#include <malatesta/app.h>
#include <iostream>
#include <sstream>
#include <dirent.h>
#include <ctime>
#include <iomanip>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <fcntl.h>
#include <thread>

auto
malatesta::timestamp() -> std::string {
    std::time_t _t = std::time(nullptr);
    std::ostringstream _out;
    _out << std::put_time(std::localtime(&_t), "%b %e %H:%M:%S") << std::flush;
    return _out.str();
}

malatesta::dir_not_found_exception::dir_not_found_exception(std::string _dir)
  : __what{ std::string{ "could not find dir: " } + _dir } {}

auto
malatesta::dir_not_found_exception::what() -> char const* {
    return this->__what.c_str();
}

auto
malatesta::inotify_closed_exception::what() -> char const* {
    return "inotify descriptor is corrupted";
}

auto
malatesta::wrong_parameter_exception::what() -> char const* {
    return malatesta::app::USAGE.data();
}

malatesta::remote_failure_exception::remote_failure_exception(std::string _cmd)
  : __what{ std::string{ "while executing remote command: " } + _cmd } {}

auto
malatesta::remote_failure_exception::what() -> char const* {
    return this->__what.c_str();
}

malatesta::event_set::event_set(ssize_t _size)
  : __size{ _size } {}

auto
malatesta::event_set::begin() -> iterator {
    return iterator{ this->__buffer, 0, this->__size };
}

auto
malatesta::event_set::end() -> iterator {
    return iterator{ this->__buffer, this->__size, this->__size };
}

malatesta::event_set::operator char*() {
    return this->__buffer;
}

malatesta::event_set::operator ssize_t() {
    return this->__size;
}

auto
malatesta::event_set::operator=(ssize_t _size) -> event_set& {
    this->__size = std::max(_size, 0L);
    return (*this);
}

auto
malatesta::event_set::capacity() const -> size_t {
    return 4096;
}

malatesta::event_set::iterator::iterator(char* _buffer, ssize_t _position, ssize_t _size)
  : __buffer{ _buffer }
  , __pointer{ __buffer + _position }
  , __current{ reinterpret_cast<const struct inotify_event*>(__pointer) }
  , __size{ _size } {}

malatesta::event_set::iterator::iterator(const iterator& _rhs) {
    (*this) = _rhs;
}

auto
malatesta::event_set::iterator::operator=(const iterator& _rhs) -> iterator& {
    this->__buffer = _rhs.__buffer;
    this->__pointer = _rhs.__pointer;
    this->__current = _rhs.__current;
    this->__size = _rhs.__size;
    return (*this);
}

auto
malatesta::event_set::iterator::operator++() -> iterator& {
    if (this->__pointer - this->__buffer >= this->__size) {
        return (*this);
    }

    this->__pointer += sizeof(struct inotify_event) + this->__current->len;
    this->__current = reinterpret_cast<const struct inotify_event*>(this->__pointer);
    return (*this);
}

auto malatesta::event_set::iterator::operator*() const -> reference {
    return this->__current;
}

auto
malatesta::event_set::iterator::operator++(int) -> iterator {
    iterator _to_return = (*this);
    ++(*this);
    return _to_return;
}

auto malatesta::event_set::iterator::operator-> () const -> pointer {
    return this->__current;
}

auto
malatesta::event_set::iterator::operator==(iterator _rhs) const -> bool {
    return this->__buffer == _rhs.__buffer && this->__pointer == _rhs.__pointer;
}

auto
malatesta::event_set::iterator::operator!=(iterator _rhs) const -> bool {
    return !((*this) == _rhs);
}

malatesta::observer::observer() {
    this->__inotify_descriptor = inotify_init();
    fcntl(
      this->__inotify_descriptor, F_SETFL, fcntl(this->__inotify_descriptor, F_GETFL) | O_NONBLOCK);

    char _self_exe_name[512] = { 0 };
    if (readlink("/proc/self/exe", _self_exe_name, 511) != 0)
        ;
    key_t _key = ftok(_self_exe_name, 1);
    this->__sem = semget(_key, 1, 0777 | IPC_CREAT);
    if (this->__sem > 0) {
        sembuf _ops[] = { { 0, 0 } };
        semop(this->__sem, _ops, 1);
    }
}

malatesta::observer::~observer() {
    ::close(this->__inotify_descriptor);
}

auto
malatesta::observer::add_exclusion(std::string _regex) -> malatesta::observer& {
    std::cout << malatesta::timestamp() << " "
              << "add: exclusion " << _regex << std::endl
              << std::flush;
    this->__excluded_dirs.push_back(std::regex{ _regex, std::regex::icase });
    return *this;
}

auto
malatesta::observer::add_filter(std::string _regex) -> malatesta::observer& {
    std::cout << malatesta::timestamp() << " "
              << "add: filter " << _regex << std::endl
              << std::flush;
    this->__file_filters.push_back(std::regex{ _regex, std::regex::icase });
    return *this;
}

auto
malatesta::observer::add_watch(std::string _path, bool _recursive) -> malatesta::observer& {
    DIR* _dir{ opendir(_path.data()) };
    if (_dir == nullptr)
        throw malatesta::dir_not_found_exception(_path);

    this->__file_descriptors.push_back(std::make_tuple(
      _path,
      inotify_add_watch(
        this->__inotify_descriptor, _path.c_str(), IN_MODIFY | IN_DELETE | IN_CREATE)));

    if (_recursive) {
        while (dirent* _entry = readdir(_dir)) {
            std::string _d_entry{ _entry->d_name };
            if (_d_entry == "." || _d_entry == ".." || this->is_excluded(_d_entry))
                continue;
            if (_entry->d_type == DT_DIR)
                this->add_watch(_path + std::string{ "/" } + _d_entry);
        }
    }
    closedir(_dir);
    return *this;
}

auto
malatesta::observer::pause() -> malatesta::observer& {
    for (auto [_name, _wd] : this->__file_descriptors) {
        inotify_rm_watch(this->__inotify_descriptor, _wd);
    }
    return *this;
}

auto
malatesta::observer::unpause() -> malatesta::observer& {
    for (auto& _wd : this->__file_descriptors) {
        std::get<1>(_wd) = inotify_add_watch(
          this->__inotify_descriptor, std::get<0>(_wd).c_str(), IN_MODIFY | IN_DELETE | IN_CREATE);
    }
    return *this;
}

auto
malatesta::observer::send_signal() -> malatesta::observer& {
    sembuf _ops[] = { { 0, 1 } };
    semop(this->__sem, _ops, 1);
    return *this;
}

auto
malatesta::observer::hook(malatesta::observer::event_type _ev_type,
                          malatesta::observer::event_handler _handler) -> malatesta::observer& {
    std::vector<malatesta::observer::event_handler> _list = { _handler };
    auto [_it, _inserted] = this->__event_handlers.insert(std::make_pair(_ev_type, _list));
    if (!_inserted) {
        _it->second.push_back(_handler);
    }
    return *this;
}

auto
malatesta::observer::hook(std::initializer_list<malatesta::observer::event_type> _ev_types,
                          malatesta::observer::event_handler _handler) -> malatesta::observer& {
    for (auto _ev_type : _ev_types)
        this->hook(_ev_type, _handler);
    return *this;
}

auto
malatesta::observer::listen() -> void {
    malatesta::event_set _buf;

    for (;;) {

        _buf = ::read(this->__inotify_descriptor, _buf, _buf.capacity());

        for (auto _event : _buf) {
            std::string _file;
            if (_event->len)
                _file.assign(_event->name);
            if (!this->is_included(_file))
                continue;

            malatesta::observer::event_type _ev_type{ malatesta::observer::event_type::UNKOWN };
            if (_event->mask & IN_MODIFY) {
                _ev_type = malatesta::observer::event_type::CHANGE;
            }
            else if (_event->mask & IN_DELETE) {
                _ev_type = malatesta::observer::event_type::REMOVAL;
            }
            else if (_event->mask & IN_CREATE) {
                _ev_type = malatesta::observer::event_type::CREATION;
            }
            else
                continue;

            std::string _target;
            for (auto _wd : this->__file_descriptors) {
                auto [_name, _fd] = _wd;
                if (_fd == _event->wd)
                    _target.assign(_name);
            }

            this->handle(_ev_type, _target, _file);
        }

        int _val = semctl(this->__sem, 0, GETVAL);
        if (_val > 0) {
            sembuf _ops[] = { { 0, -1 } };
            semop(this->__sem, _ops, 1);
            this->__paused = !this->__paused;
            if (this->__paused) {
                this->pause();
                std::cout << malatesta::timestamp() << " "
                          << "control: paused watches [ ok ]" << std::endl
                          << std::flush;
            }
            else {
                this->unpause();
                std::cout << malatesta::timestamp() << " "
                          << "control: resumed watches [ ok ]" << std::endl
                          << std::flush;
            }
        }

        if (_buf == 0L) {
            std::this_thread::sleep_for(std::chrono::duration<int, std::milli>{ 100 });
        }
    }
}

auto
malatesta::observer::handle(malatesta::observer::event_type _ev_type,
                            std::string _dir,
                            std::string _file) -> void {
    auto _found = this->__event_handlers.find(_ev_type);
    if (_found != this->__event_handlers.end()) {
        for (auto _handler : _found->second) {
            if (!_handler(_ev_type, _dir, _file))
                break;
        }
    }
}

auto
malatesta::observer::is_excluded(std::string _dir) const -> bool {
    for (auto _regex : this->__excluded_dirs) {
        if (std::regex_match(_dir, _regex))
            return true;
    }
    return false;
}

auto
malatesta::observer::is_included(std::string _file) const -> bool {
    if (this->__file_filters.size() == 0 || _file == "malatesta-lock")
        return true;

    for (auto _regex : this->__file_filters) {
        if (std::regex_match(_file, _regex))
            return true;
    }
    return false;
}

auto
malatesta::stream::add(std::string _local_uri, std::string _remote_uri) -> malatesta::stream& {
    try {
        this->find(_local_uri);
    }
    catch (malatesta::dir_not_found_exception& e) {
        this->__local_uri.push_back(_local_uri);
        size_t _idx = _remote_uri.find(":");
        this->__remote_uri.insert(std::make_pair(
          _local_uri, std::make_tuple(_remote_uri.substr(0, _idx), _remote_uri.substr(_idx + 1))));
    }
    return *this;
}

auto
malatesta::stream::cp(std::string _dir, std::string _file) -> malatesta::stream& {
    auto [_local_dir, _remote_user_host, _remote_dir] = this->find(_dir);
    std::string _suffix{ _dir.substr(_local_dir.length()) };

    std::ostringstream _oss;
    _oss << "scp " << _dir << "/" << _file << std::flush;
    this->__last_cmd_text.assign(_oss.str());

    _oss << " " << _remote_user_host << ":" << _remote_dir << _suffix << "/" << _file << std::flush;

    this->__last_cmd.assign(_oss.str());
    if (std::system(this->__last_cmd.data()) != 0)
        throw malatesta::remote_failure_exception(this->__last_cmd);

    return *this;
}

auto
malatesta::stream::rm(std::string _dir, std::string _file) -> malatesta::stream& {
    auto [_local_dir, _remote_user_host, _remote_dir] = this->find(_dir);
    std::string _suffix{ _dir.substr(_local_dir.length()) };

    std::ostringstream _oss;
    _oss << "rm -rfv " << _remote_dir << _suffix << "/" << _file << std::flush;
    this->__last_cmd_text.assign(_oss.str());
    _oss.str("");

    _oss << "ssh " << _remote_user_host << "  \"" << this->__last_cmd_text << "\"" << std::flush;
    this->__last_cmd.assign(_oss.str());
    if (std::system(this->__last_cmd.data()) != 0)
        throw malatesta::remote_failure_exception(this->__last_cmd);

    return *this;
}

auto
malatesta::stream::mkdir(std::string _dir) -> malatesta::stream& {
    auto [_local_dir, _remote_user_host, _remote_dir] = this->find(_dir);
    std::string _suffix{ _dir.substr(_local_dir.length()) };

    std::ostringstream _oss;
    _oss << "mkdir -p " << _remote_dir << _suffix << std::flush;
    this->__last_cmd_text.assign(_oss.str());
    _oss.str("");

    _oss << "ssh " << _remote_user_host << " \"" << this->__last_cmd_text << "/\"" << std::flush;
    this->__last_cmd.assign(_oss.str());
    if (std::system(this->__last_cmd.data()) != 0)
        throw malatesta::remote_failure_exception(this->__last_cmd);

    return *this;
}

auto
malatesta::stream::last_cmd() const -> std::string {
    return this->__last_cmd;
}

auto
malatesta::stream::last_cmd_text() const -> std::string {
    return this->__last_cmd_text;
}

auto
malatesta::stream::find(std::string _dir) const
  -> std::tuple<std::string, std::string, std::string> {
    std::string _local_dir{ "" };

    for (auto _item : this->__local_uri) {
        std::cout << _dir << " == " << _item << std::endl << std::flush;
        if (_dir.find(_item) == 0)
            _local_dir.assign(_item);
    }

    if (_local_dir.length() == 0)
        throw malatesta::dir_not_found_exception(_dir);

    auto _found = this->__remote_uri.find(_local_dir);
    if (_found == this->__remote_uri.end())
        throw malatesta::dir_not_found_exception(_dir);

    auto [_remote_user_host, _remote_dir] = _found->second;

    return std::make_tuple(_local_dir, _remote_user_host, _remote_dir);
}
