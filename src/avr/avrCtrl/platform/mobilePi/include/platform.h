#pragma once

#include "config.h"

namespace platform
{
    using namespace mobilePi;
    using namespace crumb;
}

setLoggingDevice( platform::LoggingDevice );

#include <avr-halib/logging/logging.h>

using avr_halib::logging::log;
using platform::Morpheus;
