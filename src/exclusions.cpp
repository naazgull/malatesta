#include <malatesta/exclusions.h>

malatesta::add_exclusion::add_exclusion(zpt::message _received)
  : zpt::events::process{ _received } {}

auto malatesta::add_exclusion::blocked() const -> bool { return false; }

auto malatesta::add_exclusion::operator()(zpt::events::dispatcher& _dispatcher)
  -> zpt::events::state {
    return zpt::events::finish;
}
