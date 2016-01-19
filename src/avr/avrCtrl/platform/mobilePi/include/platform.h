#pragma once

#include "config.h"
#include <crumb_portmap.h>
#include <motor_portmap.h>

namespace platform
{
    using namespace mobilePi;
    using namespace crumb;
}

setLoggingDevice( platform::LoggingDevice );

#include <avr-halib/logging/logging.h>

using avr_halib::logging::log;
using platform::Morpheus;
