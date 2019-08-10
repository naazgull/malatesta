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
#include <regex>

namespace malatesta {

class dir_not_found_exception : public std::exception {
  public:
    dir_not_found_exception(std::string _dir);
    virtual ~dir_not_found_exception() = default;

    auto what() -> char const*;

  private:
    std::string __what{ "" };
};

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

    auto add_exclusion(std::string path) -> observer&;
    auto add_filter(std::string path) -> observer&;
    auto add_watch(std::string path) -> observer&;
    auto hook(event_type _ev_type, event_handler _handler) -> observer&;
    auto hook(std::initializer_list<event_type> _ev_types, event_handler _handler) -> observer&;
    auto listen() -> void;

  private:
    std::vector<std::regex> __excluded_dirs;
    std::vector<std::regex> __file_filters;
    std::vector<std::tuple<std::string, int>> __file_descriptors;
    std::map<event_type, std::vector<event_handler>> __event_handlers;
    int __inotify_descriptor{ 0 };

    auto handle(event_type _ev_type, std::string _dir, std::string _file) -> void;
    auto is_excluded(std::string _dir) const -> bool;
    auto is_included(std::string _file) const -> bool;
};

class stream {
  public:
    stream() = default;
    virtual ~stream() = default;

    auto add(std::string _local_uri, std::string _remote_uri) -> stream&;
    auto cp(std::string _dir, std::string _file) -> stream&;
    auto rm(std::string _dir, std::string _file) -> stream&;
    auto mkdir(std::string _dir) -> stream&;
    auto last_cmd() const -> std::string;

  private:
    std::vector<std::string> __local_uri;
    std::map<std::string, std::tuple<std::string, std::string>> __remote_uri;
    std::string __last_cmd{ "" };

    auto find(std::string _dir) const -> std::tuple<std::string, std::string, std::string>;
};
}
