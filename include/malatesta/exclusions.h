#include <zapata/events/engine.h>

namespace malatesta {
class add_exclusion : public zpt::events::process {
  public:
    add_exclusion(zpt::message _received);
    virtual ~add_exclusion() = default;

    auto blocked() const -> bool;
    auto operator()(zpt::events::dispatcher& _dispatcher) -> zpt::events::state;
};
} // namespace malatesta
