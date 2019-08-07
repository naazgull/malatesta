#include <fstream>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <malatesta/malatesta.h>

int
main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: malatesta <local dir> <full qualified remote dir>" << std::endl
                  << std::flush;
        return 1;
    }

    std::string _local_uri{ argv[1] };
    std::string _remote_uri{ argv[2] };

    malatesta::stream _stream{ _local_uri, _remote_uri };
    malatesta::observer _watch;
    _watch.add(_local_uri);

    _watch.hook(
      { malatesta::observer::event_type::CHANGE, malatesta::observer::event_type::CREATION },
      [&_stream](
        malatesta::observer::event_type _type, std::string _dir, std::string _file) -> bool {
          bool _tried{ false };
          for (;;) {
              try {
                  if (_tried)
                      _stream.mkdir(_dir);
                  _stream.cp(_dir, _file);
                  std::cout << "* " << _stream.last_cmd() << " [ ok ]" << std::endl << std::flush;
                  return true;
              }
              catch (malatesta::remote_failure_exception& _e) {
                  std::cout << _e.what() << " [ fail ]" << std::endl << std::flush;
                  if (_tried) {
                      break;
                  }
                  _tried = true;
              }
          }
          return false;
      });

    _watch.hook(malatesta::observer::event_type::REMOVAL,
                [&_stream](malatesta::observer::event_type _type,
                           std::string _dir,
                           std::string _file) -> bool {
                    try {
                        _stream.rm(_dir, _file);
                        std::cout << "* " << _stream.last_cmd() << " [ ok ]" << std::endl << std::flush;
                        return true;
                    }
                    catch (malatesta::remote_failure_exception& _e) {
                        std::cout << _e.what() << " [ fail ]" << std::endl << std::flush;
                    }
                    return false;
                });
    _watch.listen();
    return 0;
}
