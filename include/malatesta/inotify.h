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

auto
timestamp() -> std::string;

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

class dont_start_exception : public std::exception {
  public:
    dont_start_exception() = default;
    virtual ~dont_start_exception() = default;
};

class wrong_parameter_exception : public std::exception {
  public:
    wrong_parameter_exception() = default;
    virtual ~wrong_parameter_exception() = default;

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

class event_set {
  public:
    event_set() = default;
    event_set(ssize_t _size);
    event_set(const event_set&) = delete;
    event_set(event_set&&) = delete;
    virtual ~event_set() = default;

    auto operator=(const event_set&) -> event_set& = delete;
    auto operator=(event_set&&) -> event_set& = delete;

    class iterator {
      public:
        using difference_type = std::ptrdiff_t;
        using value_type = const struct inotify_event*;
        using pointer = const struct inotify_event*;
        using reference = const struct inotify_event*;
        using iterator_category = std::forward_iterator_tag;

        explicit iterator(char* _buffer, ssize_t _position, ssize_t _size);
        iterator(const iterator& _rhs);
        virtual ~iterator() = default;

        // BASIC ITERATOR METHODS //
        auto operator=(const iterator& _rhs) -> iterator&;
        auto operator++() -> iterator&;
        auto operator*() const -> reference;
        // END / BASIC ITERATOR METHODS //
        // INPUT ITERATOR METHODS //
        auto operator++(int) -> iterator;
        auto operator->() const -> pointer;
        auto operator==(iterator _rhs) const -> bool;
        auto operator!=(iterator _rhs) const -> bool;
        // END / INPUT ITERATOR METHODS //

        // OUTPUT ITERATOR METHODS //
        // reference operator*() const; <- already defined
        // iterator operator++(int); <- already defined
        // END / OUTPUT ITERATOR METHODS //
        // FORWARD ITERATOR METHODS //
        // Enable support for both input and output iterator <- already enabled
        // END / FORWARD ITERATOR METHODS //

      private:
        char* __buffer{ nullptr };
        char* __pointer{ nullptr };
        const struct inotify_event* __current{ nullptr };
        ssize_t __size{ 0 };
    };

    auto begin() -> iterator;
    auto end() -> iterator;
    operator char*();
    operator ssize_t();
    auto operator=(ssize_t _size) -> event_set&;
    auto capacity() const -> size_t;

  private:
    char __buffer[4096] __attribute__((aligned(__alignof__(struct inotify_event)))) = { 0 };
    ssize_t __size{ 0 };
};

class observer {
  public:
    enum class event_type : int { CHANGE, REMOVAL, CREATION, MOVE_OUT, MOVE_IN, UNKOWN };
    using event_handler =
      std::function<bool(event_type _ev_type, std::string _dir, std::string _file)>;

    observer();
    virtual ~observer();

    auto add_exclusion(std::string path) -> observer&;
    auto add_filter(std::string path) -> observer&;
    auto add_watch(std::string path, bool _recursive = true) -> observer&;
    auto pause() -> malatesta::observer&;
    auto unpause() -> malatesta::observer&;
    auto send_signal() -> malatesta::observer&;
    auto hook(event_type _ev_type, event_handler _handler) -> observer&;
    auto hook(std::initializer_list<event_type> _ev_types, event_handler _handler) -> observer&;
    auto listen() -> void;

    friend auto operator<<(std::ostream& out, malatesta::observer::event_type const& in)
      -> std::ostream&;

  private:
    std::vector<std::regex> __excluded_dirs;
    std::vector<std::regex> __file_filters;
    std::vector<std::tuple<std::string, int>> __file_descriptors;
    std::map<event_type, std::vector<event_handler>> __event_handlers;
    int __inotify_descriptor{ 0 };
    int __sem{ 0 };
    bool __paused{ false };

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
    auto last_cmd_text() const -> std::string;

  private:
    std::vector<std::string> __local_uri;
    std::map<std::string, std::tuple<std::string, std::string>> __remote_uri;
    std::string __last_cmd{ "" };
    std::string __last_cmd_text{ "" };

    auto find(std::string _dir) const -> std::tuple<std::string, std::string, std::string>;
};
}
