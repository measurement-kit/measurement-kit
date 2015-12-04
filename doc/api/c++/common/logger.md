# NAME
Logger -- Log messages processor

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

mk::Logger logger;          // Log on stderr warn messages only

logger.set_verbose(-1);     // Same as above
logger.set_verbose(0);      // Only log warning messages
logger.set_verbose(1);      // Log all messages

// Set custom log function
logger.on_log([](const char *log_line) {
    // Do something with it
});
logger.on_log(nullptr);     // Set log function that discards all logs

logger.debug("Format string: %s", "but also arguments");
logger.info("Just like printf");
logger.warn("Use this for important messages");

mk::Logger *root = mk::Logger::default();
```

# DESCRIPTION

The `Logger` class specifies how logs are processed. You can change the
function that receives logs. You can change the verbosity level.

Typically there is a root logger (accessible with `Logger::global()`) and
a logger specific of each network test.

# HISTORY

The `Logger` class appeared in MeasurementKit 0.1.
