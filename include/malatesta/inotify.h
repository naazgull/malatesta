#pragma once

#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <vector>
#include <map>
#include <string>
#include <tuple>
#include <functional>

namespace malatesta {

class inotify_closed_exception : public std::exception {
  public:
    inotify_closed_exception() = default;
    virtual ~inotify_closed_exception() = default;

    auto what() -> char const*;
};

class remote_failure_exception : public std::exception {
  public:
    remote_failure_exception(std::string _cmd);
    virtual ~remote_failure_exception() = default;

    auto what() -> char const*;

  private:
    std::string __what{ "" };
};

class observer {
  public:
    enum class event_type : int { CHANGE, REMOVAL, CREATION, UNKOWN };
    using event_handler =
      std::function<bool(event_type _ev_type, std::string _dir, std::string _file)>;

    observer();
    virtual ~observer();

    auto add(std::string path) -> void;
    auto hook(event_type _ev_type, event_handler _handler) -> void;
    auto hook(std::initializer_list<event_type> _ev_types, event_handler _handler) -> void;
    auto listen() -> void;

  private:
    std::vector<std::tuple<std::string, int>> __file_descriptors;
    std::map<event_type, std::vector<event_handler>> __event_handlers;
    int __inotify_descriptor{ 0 };

    auto handle(event_type _ev_type, std::string _dir, std::string _file) -> void;
};

class stream {
  public:
    stream(std::string _local_uri, std::string _remote_uri);
    virtual ~stream() = default;

    auto cp(std::string _dir, std::string _file) -> void;
    auto rm(std::string _dir, std::string _file) -> void;
    auto mkdir(std::string _dir) -> void;
    auto last_cmd() -> std::string;

  private:
    std::string __local_dir{ "" };
    std::string __remote_user_host{ "" };
    std::string __remote_dir{ "" };
    std::string __last_cmd{ "" };
};
}
