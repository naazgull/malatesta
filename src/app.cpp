#include <malatesta/app.h>
#include <iostream>
#include <string>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <cstdlib>
#include <filesystem>

malatesta::app::app(int _argc, char** _argv)
  : __argc{ _argc }
  , __argv{ _argv } {
    this->process_params().load_env_variable();
}

auto
malatesta::app::block(std::string _dir, std::string _file) -> bool {
    if (_dir == this->__tmp_dir) {
        if (_dir + std::string{ "/" } + _file == this->__block_file)
            this->__blocked = true;
        return true;
    }
    return this->__blocked;
}

auto
malatesta::app::unblock(std::string _dir, std::string _file) -> bool {
    if (_dir == this->__tmp_dir) {
        if (_dir + std::string{ "/" } + _file == this->__block_file)
            this->__blocked = false;
        return true;
    }
    return this->__blocked;
}

auto
malatesta::app::start() -> app& {
    if (this->__pause_resume == 1) {
        std::string _cmd{ std::string{ "touch " } + this->__block_file };
        if (std::system(_cmd.data()) != 0)
            ;
        return (*this);
    }
    if (this->__pause_resume == 2) {
        std::string _cmd{ std::string{ "rm -rf " } + this->__block_file };
        if (std::system(_cmd.data()) != 0)
            ;
        return (*this);
    }

    this->__watch.hook(
      { malatesta::observer::event_type::CHANGE, malatesta::observer::event_type::CREATION },
      [this](malatesta::observer::event_type _type, std::string _dir, std::string _file) -> bool {
          if (this->block(_dir, _file))
              return true;

          bool _tried{ false };
          for (;;) {
              try {
                  if (_tried) {
                      this->__stream.mkdir(_dir);
                      std::cout << "exec: " << this->__stream.last_cmd() << " [ ok ]" << std::endl
                                << std::flush;
                  }
                  this->__stream.cp(_dir, _file);
                  std::cout << "exec: " << this->__stream.last_cmd() << " [ ok ]" << std::endl
                            << std::flush;
                  return true;
              }
              catch (malatesta::remote_failure_exception& _e) {
                  if (_tried) {
                      std::cout << "exec: " << _e.what() << " [ fail ]" << std::endl << std::flush;
                      break;
                  }
                  _tried = true;
              }
              catch (malatesta::dir_not_found_exception& _e) {
                  std::cout << "exec: " << _e.what() << " [ fail ]" << std::endl << std::flush;
                  break;
              }
          }
          return false;
      });

    this->__watch.hook(
      malatesta::observer::event_type::REMOVAL,
      [this](malatesta::observer::event_type _type, std::string _dir, std::string _file) -> bool {
          if (this->unblock(_dir, _file))
              return true;

          try {
              this->__stream.rm(_dir, _file);
              std::cout << "exec: " << this->__stream.last_cmd() << " [ ok ]" << std::endl
                        << std::flush;
              return true;
          }
          catch (std::exception& _e) {
              std::cout << "exec: " << _e.what() << " [ fail ]" << std::endl << std::flush;
          }
          return false;
      });

    this->__watch.listen();
    return (*this);
}

auto
malatesta::app::process_params() -> app& {
    std::vector<std::string> _local_uri_params;
    int _opt{ -1 };
    opterr = 0;
    while ((_opt = getopt(this->__argc, this->__argv, "prw:x:f:")) != -1) {
        switch (_opt) {
            case 'p': {
                std::cout << "pausing malatesta " << std::endl << std::flush;
                this->__pause_resume = 1;
                return (*this);
            }
            case 'r': {
                std::cout << "resuming malatesta " << std::endl << std::flush;
                this->__pause_resume = 2;
                return (*this);
            }
            case 'w': {
                std::string _opt_val{ const_cast<char const*>(optarg) };
                std::string _local_uri{ _opt_val.substr(0, _opt_val.find(",")) };
                std::string _remote_uri{ _opt_val.substr(_opt_val.find(",") + 1) };
                _local_uri_params.push_back(_local_uri);
                this->__stream.add(_local_uri, _remote_uri);
                std::cout << "watch: " << _local_uri << " -> " << _remote_uri << " [ ok ]"
                          << std::endl
                          << std::flush;
                break;
            }
            case 'x': {
                std::string _opt_val{ const_cast<char const*>(optarg) };
                this->__watch.add_exclusion(_opt_val);
                break;
            }
            case 'f': {
                std::string _opt_val{ const_cast<char const*>(optarg) };
                this->__watch.add_filter(_opt_val);
                break;
            }
            case '?':
            default: {
                throw malatesta::wrong_parameter_exception();
            }
        }
    }

    if (optind != this->__argc)
        throw malatesta::wrong_parameter_exception();

    for (auto _local_uri : _local_uri_params)
        this->__watch.add_watch(_local_uri);

    return (*this);
}

auto
malatesta::app::load_env_variable() -> app& {
    const char* _env_var = std::getenv("HOME");
    if (_env_var == nullptr)
        _env_var = "/tmp";
    this->__tmp_dir = std::string{ _env_var } + std::string{ "/.malatesta" };
    std::string _cmd{ std::string{ "mkdir -p " } + this->__tmp_dir };
    if (std::system(_cmd.data()) != 0)
        ;
    this->__block_file = this->__tmp_dir + std::string{ "/malatesta-lock" };
    this->__watch.add_watch(this->__tmp_dir, false);
    return (*this);
}
