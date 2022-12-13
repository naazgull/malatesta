#include <zapata/events/engine.h>

namespace malatesta {
class add_filter : public zpt::events::process {
  public:
    add_filter(zpt::message _received);
    virtual ~add_filter() = default;

    auto blocked() const -> bool;
    auto operator()(zpt::events::dispatcher& _dispatcher) -> zpt::events::state;
};
} // namespace malatesta
