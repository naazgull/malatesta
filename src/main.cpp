#include <fstream>
#include <iostream>
#include <string>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <malatesta/malatesta.h>

int
main(int argc, char* argv[]) {
    std::string _usage{
        "usage: malatesta -w <local dir>,<full qualified remote dir> -x <exclude dirs regex> -f <included files regex>\n"
        "       -w\tlocal and remote uri pair - multiple occurrences accepted\n"
        "       -x\tdirectory pattern to be excluded from watches - multiple occurrences accepted\n"
        "       -f\tfile pattern to include in watches - multiple occurrences accepted\n"
    };
    if (argc < 3) {
        std::cout << _usage << std::endl << std::flush;
        return 1;
    }

    malatesta::stream _stream;
    malatesta::observer _watch;
    std::vector<std::string> _local_uri_params;
    int _opt{ -1 };
    opterr = 0;
    while ((_opt = getopt(argc, argv, "w:x:f:")) != -1) {
        switch (_opt) {
            case 'w': {
                std::string _opt_val{ const_cast<char const*>(optarg) };
                std::string _local_uri{ _opt_val.substr(0, _opt_val.find(",")) };
                std::string _remote_uri{ _opt_val.substr(_opt_val.find(",") + 1) };
                _local_uri_params.push_back(_local_uri);
                _stream.add(_local_uri, _remote_uri);
                std::cout << "watch: " << _local_uri << " -> " << _remote_uri << " [ ok ]"
                          << std::endl
                          << std::flush;
                break;
            }
            case 'x': {
                std::string _opt_val{ const_cast<char const*>(optarg) };
                _watch.add_exclusion(_opt_val);
                break;
            }
            case 'f': {
                std::string _opt_val{ const_cast<char const*>(optarg) };
                _watch.add_filter(_opt_val);
                break;
            }
            case '?':
            default: {
                std::cout << _usage << std::endl << std::flush;
                return 1;
            }
        }
    }

    if (optind != argc) {
        std::cout << _usage << std::endl << std::flush;
        return 1;
    }

    for (auto _local_uri : _local_uri_params)
        _watch.add_watch(_local_uri);

    _watch.hook(
      { malatesta::observer::event_type::CHANGE, malatesta::observer::event_type::CREATION },
      [&_stream](
        malatesta::observer::event_type _type, std::string _dir, std::string _file) -> bool {
          bool _tried{ false };
          for (;;) {
              try {
                  if (_tried) {
                      _stream.mkdir(_dir);
                      std::cout << "exec: " << _stream.last_cmd() << " [ ok ]" << std::endl
                                << std::flush;
                  }
                  _stream.cp(_dir, _file);
                  std::cout << "exec: " << _stream.last_cmd() << " [ ok ]" << std::endl
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

    _watch.hook(
      malatesta::observer::event_type::REMOVAL,
      [&_stream](
        malatesta::observer::event_type _type, std::string _dir, std::string _file) -> bool {
          try {
              _stream.rm(_dir, _file);
              std::cout << "exec: " << _stream.last_cmd() << " [ ok ]" << std::endl << std::flush;
              return true;
          }
          catch (std::exception& _e) {
              std::cout << "exec: " << _e.what() << " [ fail ]" << std::endl << std::flush;
          }
          return false;
      });
    _watch.listen();
    return 0;
}
