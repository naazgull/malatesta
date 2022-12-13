#include <malatesta/controls.h>

malatesta::pause::pause(zpt::message _received)
  : zpt::events::process{ _received } {}

auto malatesta::pause::blocked() const -> bool { return false; }

auto malatesta::pause::operator()(zpt::events::dispatcher& _dispatcher)
  -> zpt::events::state {
    return zpt::events::finish;
}

malatesta::resume::resume(zpt::message _received)
  : zpt::events::process{ _received } {}

auto malatesta::resume::blocked() const -> bool { return false; }

auto malatesta::resume::operator()(zpt::events::dispatcher& _dispatcher)
  -> zpt::events::state {
    return zpt::events::finish;
}

malatesta::get_status::get_status(zpt::message _received)
  : zpt::events::process{ _received } {}

auto malatesta::get_status::blocked() const -> bool { return false; }

auto malatesta::get_status::operator()(zpt::events::dispatcher& _dispatcher)
  -> zpt::events::state {
    return zpt::events::finish;
}
