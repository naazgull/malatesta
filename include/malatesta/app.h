#pragma once

#include <malatesta/config.h>
#include <malatesta/inotify.h>

namespace malatesta {

class app {
  public:
    app(int _argc, char** _argv);
    virtual ~app() = default;

    auto blocked() -> bool;
    auto start() -> app&;

    inline static const std::string USAGE{
        "malatesta (v" PACKAGE_VERSION ")\n"
        "usage:\n  malatesta -w <local dir>,<full qualified remote dir> -x <exclude dirs regex> -f "
        "<included files regex>\n"
        "       -w\tlocal and remote uri pair - multiple occurrences accepted\n"
        "       -x\tdirectory pattern to be excluded from watches - multiple occurrences accepted\n"
        "       -f\tfile pattern to include in watches - multiple occurrences accepted\n\n"
        "  malatesta -p\n"
        "       -p\tpause the observer\n\n"
        "  malatesta -r\n"
        "       -r\tresume the observer\n\n"
    };

  private:
    int __argc{ 0 };
    char** __argv{ nullptr };
    malatesta::stream __stream;
    malatesta::observer __watch;
    std::string __tmp_dir;
    std::string __block_file;
    bool __blocked{ false };
    int __pause{ 0 };

    auto process_params() -> app&;
    auto setup_semaphore() -> app&;
};

}
