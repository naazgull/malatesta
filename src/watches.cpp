#include <malatesta/watches.h>

malatesta::add_watch::add_watch(zpt::message _received)
  : zpt::events::process{ _received } {}

auto malatesta::add_watch::blocked() const -> bool { return false; }

auto malatesta::add_watch::operator()(zpt::events::dispatcher& _dispatcher) -> zpt::events::state {
    return zpt::events::finish;
}
