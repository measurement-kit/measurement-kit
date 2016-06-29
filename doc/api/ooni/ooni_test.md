# NAME
ooni_test &mdash; Generic OONI test

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/ooni.hpp>

namespace mk {
namespace ooni {

class OoniTest : public NetTest {
  public:
    std::string test_name;
    std::string test_version;
    std::string probe_ip = "127.0.0.1";
    std::string probe_asn = "AS0";
    std::string probe_cc = "ZZ";
    bool needs_input = false;

    OoniTest();
    OoniTest(std::string input_filepath);
    OoniTest(std::string input_filepath, Settings settings);
};

} // namespace ooni
} // namespace mk
```

# STABILITY

2 - Stable

# DESCRIPTION

The `OoniTest` class overrides certain aspects of the `NetTest` class. Specifically,
it adds the following attributes:

- *test_name*: OONI test name
- *test_version*: OONI test version
- *probe_ip*: IP of the client
- *probe_asn*: autonomous system number of the client
- *probe_cc*: country code of the client
- *needs_input*: whether this OONI test requires an input file path

This class defines three constructors. The first form of the constructor default initializes
the object. The second form of the constructor allows to initialize the input file path.
The third form of the constructor allows to initialize both the input file path and the settings.

This class also overrides the `begin()` and `end()` methods of `NetTest`. The algorithm
followed by `begin()` is the following:

- obtain client IP address, country code and autonomous system number (ASN)
- open the report
- for each input (or, if there is not input, once):
    - run measurement with input (using empty string if there is no input)
- callback

The algorithm followed by `end()` is the following:

- close the report
- submit the report to collector
- callback

The `OoniTest` class honours the following settings:

- *"save_real_probe_ip"* (bool): if `false` the `probe_ip` added to the report would be
  `127.0.0.1`, otherwise it would be the real client IP address.

- *"geoip_country_path"* (string): path to GeoIP country file, used to compute the
  country code of a client (if this is missing, country code defaults to `ZZ`).

- *"geoip_asn_path"* (string): path to GeoIP ASN file, used to compute the
  ASN of a client (if this is missing, country code defaults to `AS0`).

- *"collector_base_url"* (string): base URL of the collector, otherwise the default one
  would be used.

- all other settings relevant to `dns` and `net`.

# HISTORY

The `OoniTest` class appeared in MeasurementKit 0.2.0.
