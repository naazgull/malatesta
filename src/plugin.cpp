#include <zapata/transport.h>
#include <zapata/startup.h>
#include <zapata/rest.h>
#include <malatesta/filters.h>
#include <malatesta/exclusions.h>
#include <malatesta/watches.h>
#include <malatesta/controls.h>

extern "C" auto _zpt_load_(zpt::plugin& _plugin) -> void {
    zlog("Registering malatesta events", zpt::info);
    auto _resolver = zpt::global_cast<zpt::rest::resolver>(zpt::REST_RESOLVER());
    
    _resolver->add<malatesta::add_filter>(zpt::Post, "/filters");
    _resolver->add<malatesta::add_exclusion>(zpt::Post, "/exclusions");
    _resolver->add<malatesta::add_watch>(zpt::Post, "/watches");
    _resolver->add<malatesta::pause>(zpt::Post, "/controls/pause");
    _resolver->add<malatesta::resume>(zpt::Post, "/controls/resume");
    _resolver->add<malatesta::get_status>(zpt::Get, "/controls/status");
}

extern "C" auto _zpt_unload_(zpt::plugin& _plugin) -> void {
    zlog("Disposing malatesta events", zpt::info);
}
