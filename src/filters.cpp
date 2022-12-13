#include <malatesta/filters.h>

malatesta::add_filter::add_filter(zpt::message _received)
  : zpt::events::process{ _received } {}

auto malatesta::add_filter::blocked() const -> bool { return false; }

auto malatesta::add_filter::operator()(zpt::events::dispatcher& _dispatcher) -> zpt::events::state {
    return zpt::events::finish;
}

