#include <zapata/events/engine.h>

namespace malatesta {
class add_watch : public zpt::events::process {
  public:
    add_watch(zpt::message _received);
    virtual ~add_watch() = default;

    auto blocked() const -> bool;
    auto operator()(zpt::events::dispatcher& _dispatcher) -> zpt::events::state;
};
} // namespace malatesta
