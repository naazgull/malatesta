#include <zapata/events/engine.h>

namespace malatesta {
class pause : public zpt::events::process {
  public:
    pause(zpt::message _received);
    virtual ~pause() = default;

    auto blocked() const -> bool;
    auto operator()(zpt::events::dispatcher& _dispatcher) -> zpt::events::state;
};
class resume : public zpt::events::process {
  public:
    resume(zpt::message _received);
    virtual ~resume() = default;

    auto blocked() const -> bool;
    auto operator()(zpt::events::dispatcher& _dispatcher) -> zpt::events::state;
};
class get_status : public zpt::events::process {
  public:
    get_status(zpt::message _received);
    virtual ~get_status() = default;

    auto blocked() const -> bool;
    auto operator()(zpt::events::dispatcher& _dispatcher) -> zpt::events::state;
};
} // namespace malatesta
