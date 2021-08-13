#include <malatesta/app.h>
#include <iostream>
#include <fstream>
#include <string>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <cstdlib>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

malatesta::app::app(int _argc, char** _argv)
  : __argc{ _argc }
  , __argv{ _argv } {
    this->process_params();
}

auto
malatesta::app::start() -> app& {
    this->__watch.hook(
      { malatesta::observer::event_type::CHANGE,
        malatesta::observer::event_type::CREATION,
        malatesta::observer::event_type::MOVE_IN },
      [this](malatesta::observer::event_type _type, std::string _dir, std::string _file) -> bool {
          bool _tried{ false };
          for (;;) {
              try {
                  if (_tried) {
                      this->__stream.mkdir(_dir);
                      std::cout << malatesta::timestamp() << " "
                                << "exec: [" << _type << "] " << this->__stream.last_cmd_text()
                                << " [ ok ]" << std::endl
                                << std::flush;
                  }
                  this->__stream.cp(_dir, _file);
                  std::cout << malatesta::timestamp() << " "
                            << "exec: [" << _type << "] " << this->__stream.last_cmd_text()
                            << " [ ok ]" << std::endl
                            << std::flush;
                  return true;
              }
              catch (malatesta::remote_failure_exception& _e) {
                  if (_tried) {
                      std::cout << malatesta::timestamp() << " "
                                << "exec: [" << _type << "] " << _e.what() << " [ fail ]"
                                << std::endl
                                << std::flush;
                      break;
                  }
                  _tried = true;
              }
              catch (malatesta::dir_not_found_exception& _e) {
                  std::cout << malatesta::timestamp() << " "
                            << "exec: [" << _type << "] " << _e.what() << " [ fail ]" << std::endl
                            << std::flush;
                  break;
              }
          }
          return false;
      });

    this->__watch.hook(
      { malatesta::observer::event_type::REMOVAL, malatesta::observer::event_type::MOVE_OUT },
      [this](malatesta::observer::event_type _type, std::string _dir, std::string _file) -> bool {
          try {
              this->__stream.rm(_dir, _file);
              std::cout << malatesta::timestamp() << " "
                        << "exec: [" << _type << "] " << this->__stream.last_cmd_text() << " [ ok ]"
                        << std::endl
                        << std::flush;
              return true;
          }
          catch (std::exception& _e) {
              std::cout << malatesta::timestamp() << " "
                        << "exec: [" << _type << "] " << _e.what() << " [ fail ]" << std::endl
                        << std::flush;
          }
          return false;
      });

    this->__watch.listen();
    return (*this);
}

auto
malatesta::app::process_params() -> app& {
    if (this->__argc < 2)
        throw malatesta::wrong_parameter_exception();

    std::vector<std::string> _local_uri_params;
    int _opt{ -1 };
    opterr = 0;
    while ((_opt = getopt(this->__argc, this->__argv, "sw:x:f:")) != -1) {
        switch (_opt) {
            case 's': {
                this->__watch.send_signal();
                throw malatesta::dont_start_exception();
            }
            case 'w': {
                if (this->__argc < 3)
                    throw malatesta::wrong_parameter_exception();
                std::string _opt_val{ const_cast<char const*>(optarg) };
                std::string _local_uri{ _opt_val.substr(0, _opt_val.find(",")) };
                std::string _remote_uri{ _opt_val.substr(_opt_val.find(",") + 1) };
                _local_uri_params.push_back(_local_uri);
                this->__stream.add(_local_uri, _remote_uri);
                std::cout << malatesta::timestamp() << " "
                          << "watch: " << _local_uri << " -> " << _remote_uri << " [ ok ]"
                          << std::endl
                          << std::flush;
                break;
            }
            case 'x': {
                if (this->__argc < 5)
                    throw malatesta::wrong_parameter_exception();
                std::string _opt_val{ const_cast<char const*>(optarg) };
                this->__watch.add_exclusion(_opt_val);
                break;
            }
            case 'f': {
                if (this->__argc < 5)
                    throw malatesta::wrong_parameter_exception();
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
