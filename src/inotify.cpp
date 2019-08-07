#include <malatesta/inotify.h>
#include <iostream>
#include <sstream>
#include <dirent.h>

auto
malatesta::inotify_closed_exception::what() -> char const* {
    return "inotify file descriptor is corrupted";
}

malatesta::remote_failure_exception::remote_failure_exception(std::string _cmd)
  : __what{ std::string{ "failure while executing remote command: " } + _cmd } {}

auto
malatesta::remote_failure_exception::what() -> char const* {
    return this->__what.c_str();
}

malatesta::observer::observer() {
    this->__inotify_descriptor = inotify_init();
}

malatesta::observer::~observer() {
    ::close(this->__inotify_descriptor);
}

auto
malatesta::observer::add(std::string _path) -> void {
    DIR* _dir{ opendir(_path.data()) };
    if (_dir == nullptr)
        return;

    this->__file_descriptors.push_back(std::make_tuple(
      _path,
      inotify_add_watch(
        this->__inotify_descriptor, _path.c_str(), IN_MODIFY | IN_DELETE | IN_CREATE)));

    while (dirent* _entry = readdir(_dir)) {
        std::string _d_entry{ _entry->d_name };
        if (_d_entry[0] == '.')
            continue;
        if (_entry->d_type == DT_DIR)
            this->add(_path + std::string{ "/" } + _d_entry);
    }
    closedir(_dir);
}

auto
malatesta::observer::hook(malatesta::observer::event_type _ev_type,
                          malatesta::observer::event_handler _handler) -> void {
    std::vector<malatesta::observer::event_handler> _list = { _handler };
    auto [_it, _inserted] = this->__event_handlers.insert(std::make_pair(_ev_type, _list));
    if (!_inserted) {
        _it->second.push_back(_handler);
    }
}

auto
malatesta::observer::hook(std::initializer_list<malatesta::observer::event_type> _ev_types,
                          malatesta::observer::event_handler _handler) -> void {
    for (auto _ev_type : _ev_types)
        this->hook(_ev_type, _handler);
}

auto
malatesta::observer::listen() -> void {
    char _buf[4096] __attribute__((aligned(__alignof__(struct inotify_event))));

    for (;;) {

        ssize_t _len = ::read(this->__inotify_descriptor, _buf, sizeof _buf);
        if (_len < 0)
            throw malatesta::inotify_closed_exception();

        const struct inotify_event* _event{ nullptr };
        for (char* _ptr = _buf; _ptr < _buf + _len;
             _ptr += sizeof(struct inotify_event) + _event->len) {
            _event = reinterpret_cast<const struct inotify_event*>(_ptr);

            std::string _file;
            if (_event->len)
                _file.assign(_event->name);
            if (_file[0] == '.' || _file.find(".#") != std::string::npos)
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

malatesta::stream::stream(std::string _local_uri, std::string _remote_uri)
  : __local_dir{ _local_uri } {
    size_t _idx = _remote_uri.find(":");
    this->__remote_user_host.assign(_remote_uri.substr(0, _idx));
    this->__remote_dir.assign(_remote_uri.substr(_idx + 1));
}

auto
malatesta::stream::cp(std::string _dir, std::string _file) -> void {
    std::string _suffix{ _dir.substr(this->__local_dir.length()) };

    std::ostringstream _oss;
    _oss << "scp " << _dir << "/" << _file << " " << this->__remote_user_host << ":"
         << this->__remote_dir << _suffix << "/" << _file << std::flush;
    this->__last_cmd.assign(_oss.str());
    if (std::system(this->__last_cmd.data()) != 0)
        throw malatesta::remote_failure_exception(this->__last_cmd);
}

auto
malatesta::stream::rm(std::string _dir, std::string _file) -> void {
    std::string _suffix{ _dir.substr(this->__local_dir.length()) };

    std::ostringstream _oss;
    _oss << "ssh " << this->__remote_user_host << "  \"rm -rfv " << this->__remote_dir << _suffix
         << "/" << _file << "\"" << std::flush;
    this->__last_cmd.assign(_oss.str());
    if (std::system(this->__last_cmd.data()) != 0)
        throw malatesta::remote_failure_exception(this->__last_cmd);
}

auto
malatesta::stream::mkdir(std::string _dir) -> void {
    std::string _suffix{ _dir.substr(this->__local_dir.length()) };

    std::ostringstream _oss;
    _oss << "ssh " << this->__remote_user_host << " \"mkdir -p " << this->__remote_dir << _suffix
         << "/\"" << std::flush;
    this->__last_cmd.assign(_oss.str());
    if (std::system(this->__last_cmd.data()) != 0)
        throw malatesta::remote_failure_exception(this->__last_cmd);
}

auto
malatesta::stream::last_cmd() -> std::string {
    return this->__last_cmd;
}
