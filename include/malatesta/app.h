#pragma once

#include <malatesta/config.h>
#include <malatesta/inotify.h>

namespace malatesta {

class app {
  public:
    app(int _argc, char** _argv);
    virtual ~app() = default;

    auto block(std::string _dir, std::string _file) -> bool;
    auto unblock(std::string _dir, std::string _file) -> bool;
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
    short __pause_resume{ 0 };
    malatesta::stream __stream;
    malatesta::observer __watch;
    std::string __tmp_dir;
    std::string __block_file;
    bool __blocked{ false };

    auto process_params() -> app&;
    auto load_env_variable() -> app&;
};

}
