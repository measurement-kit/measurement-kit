# NAME
orchestrate -- code to talk to OONI's orchestrator

# LIBRARY
MeasurementKit (`libmeasurement_kit`, `-lmeasurement_kit`).

# SYNOPSIS
```C++
#include <measurement_kit/ooni.hpp>

namespace mk {
namespace ooni {
namespace orchestrate {

std::string production_registry_url();
std::string testing_registry_url();

std::string production_events_url();
std::string testing_events_url();

class Auth {
  public:
    std::string auth_token;
    std::time_t expiry_time = {};
    bool logged_in = false;
    std::string username;
    std::string password;

    static std::string make_password();
    Error load(const std::string &filepath) noexcept;
    Error loads(const std::string &data) noexcept;
    Error dump(const std::string &filepath) noexcept;
    std::string dumps() noexcept;
    bool is_valid() const noexcept;
};

class Client {
  public:
    Var<Logger> logger = Logger::global();
    Settings settings = {};
    std::string available_bandwidth;
    std::string device_token;
    std::string events_url = production_events_url();
    std::string language;
    std::string network_type;
    std::string geoip_country_path;
    std::string geoip_asn_path;
    std::string platform;
    std::string probe_asn;
    std::string probe_cc;
    std::string probe_family;
    std::string registry_url = production_registry_url();
    std::string software_name = "measurement_kit";
    std::string software_version = MK_VERSION;
    std::vector<std::string> supported_tests;

    void register_probe(
          std::string &&, Callback<Error &&, Auth &&> &&callback) const;

    void find_location(
          Callback<Error &&, std::string &&, std::string &&> &&callback) const;

    void update(Auth &&, Callback<Error &&, Auth &&> &&callback) const;

    void list_tasks(
          Auth &&,
          Callback<Error &&, Auth &&, std::vector<Task> &&> &&callback) const;
};

class Task {
  public:
    std::string events_url = production_events_url();
    std::string task_id;

    void get(Auth &&,
             Callback<Error &&, Auth &&, std::string &&> &&callback) const;

    void accept(Auth &&, Callback<Error &&, Auth &&> &&callback) const;

    void reject(Auth &&, Callback<Error &&, Auth &&> &&callback) const;

    void done(Auth &&, Callback<Error &&, Auth &&> &&callback) const;
};

} // namespace orchestrate
} // namespace ooni
} // namespace mk
```

# STABILITY

1 - Experimental

# DESCRIPTION

The orchestrator client communicates with OONI's orchestrator. That is a set
of services telling clients which tests it is most optimal to run, and with
what inputs. This allows OONI to maximize censorship coverage given the probe's
current geographic location and internet service provider.

The `production_registry_url`, `testing_registry_url`, `production_events_url`,
and `testing_events_url` functions return, respectively, the production and
testing URLs of the orchestrator's registry and events web services used by
OONI.

The `Auth` class contains authentication information. This class is passed
around by most APIs, because the JWT authentication token may be modified as
a result of every call to the API. If you want to store authentication data
in a persistent way, there are methods to do that. More specifically:

The `make_password()` method is a static method that creates a random
password for you. This may be useful to you when you are about to call
the `Client::register_probe()` method, as described below.

The `load` method loads authentication information from the specified file.

The `loads` methods loads authentication information encoded in JSON
from the specified string.

The `dump` method stores authentication information into the specified file.

The `dumps` method returns authentication information encoded as JSON.

The `is_valid` method returns true if we're logged in and the authentication
token is not expired, false otherwise.

In general, you should store authentication information on persistent storage
only after you register a probe. This can be done with `Client::register_probe`
method. When your application starts up, you will then typically load the
authentication information from persistent storage. Most if not all remote
API calls MAY update the token contained in the `Auth` structure. This is why
most APIs take `Auth` in by *move* and their callback has an `Auth` struct
parameter that is *moved out*. You don't need to sync up the updated structure
onto permanent storage, however, because the only change that can occur when
APIs are called is that the authentication token is updated.

The `Client` class is the one you use to start interacting with the
orchestrator services. Once you have constructed a client with the
default constructor, you can configure it by settings these attributes:

The `logger` attribute can be override to use a custom logger. By default, the
global MeasurementKit logger is used.

The `settings` attribute allows to specify optional settings. By default, an
empty settings object is used, meaning that defaults will always be used.

The `available_bandwidth` attribute indicates how much bandwidth you would
like to use for automatic tests run through orchestration. By default, this
attribute is empty, meaning that no bandwidth quota will be enforced.

The `device_token` attribute uniquely identifies the app inside a device, and
is only meaningful in the mobile context. This token is generated by the mobile
OS for the current application and is used to identify it.

The `events_url` attribute contains the URL used to query the `events` web
service of the orchestrator. By default, this attribute is set to the production
events service URL used by OONI.

The `language` attribute tells the orchestrator the language used by
this probe. By default this attribute is unspecified.

The `network_type` attribute tells the orchestrator the type of the network
the probe is connected to (i.e. Wi-Fi or mobile). By default this attribute is
unspecified.

The `geoip_country_path` and `geoip_asn_path` are used by the orchestrator
client to guess the country and autonomous system number, if they are not
provided by the API user (see below). By default these attributes are empty,
meaning that MK will fail to geolocate the user.

The `platform` attribute tells the orchestrator the platform in
which the probe runs (e.g., `android`, `ios`). If not set, this
attribute will be filled automatically by MeasurementKit when the
actual request for the orchestrator is prepared.

The `probe_asn` attribute tells the orchestrator the autonomous
system number (ASN) of the ISP that the probe is connected to. If
not set, this attribute will be filled automatically by MeasurementKit
when the actual request for the orchestrator is prepared.

The `probe_cc` attribute tells the orchestrator the country code
of the country in which the probe currently is. If not set, this
attribute will be filled automatically by MeasurementKit when the
actual request for the orchestrator is prepared.

The `probe_family` attribute allows to bind a set of different
probes to a common family of probes. This attribute is currently
reserved for future use.

The `registry_url` attribute contains the URL used to query the
`registry` web service of the orchestrator. By default, this attribute
is set to the production registry service URL used by OONI.

The `software_name` attribute is the name of the application using
MeasurementKit OONI's orchestrator. By default is `measurement_kit`
and you most certainly want to change it to the name of your
application.

The `software_version` attribute is the version of the application
using MeasurementKit OONI's orchestrator. By default is the current
version of MeasurementKit and you most certainly want to change it
to the version of your application.

The `supported_tests` attribute is the vector of test names supported
by OONI that the application would like to support. If not set,
this field is filled by MeasurementKit with the list of implemented
tests when preparing the request for the orchestrator.

Once the orchestrator client is configured, you can use its methods to send
requests to the orchestrator server. The following methods are available:

The `register_probe` method registers the current probe with the
orchestrator system. The first argument is the password to use to
identify the probe; if the password is empty, `Auth::make_password()`
will be used to generate a password automatically.  The second
argument is a callback.  The callback receives two parameters: the
error that occurred (if any), and an authentication structure that
contains authentication information (your username; the password
you chose or a random password, depending on how you used the API;
a JWT authentication token). You typically want to store such
information into a persistent storage, and to load it next to invoke
again the API.

The `find_location` method uses MeasurementKit internal functionality
to query for the current autonomous system number and country code. You
can optionally use this function to fill the `probe_asn` and `probe_cc`
fields of client. The first argument passed to the callback is the
error that occurred, if any, The second argument is the probe's ASN.
The third argument is the probe country code.

The `update` method updates the orchestrator's system knowledge of the
state of a probe (e.g. network type, location, ISP). If the probe is
not registered yet, this method will fail. The first parameter
is an authentication structure whose content is used to login with
the orchestrator services and call the proper `update` API.  The second
argument is a callback taking two arguments. The first callback argument
is the error that occurred, if any. The second callback argument is a
possibly-updated authentication structure.

The `list_tasks` method assumes that a probe is already registered
and gets the list of tasks to run. The first argument is an authentication
structure. The second argument is a callback. The callback receives three
parameters: whether there was an error, a possibly updated `Auth`
structure, and the list of tasks.

Each task is an instance of the `Task` class.

The `list_tasks` callback receives as argument already configured `Task`
instances. In particular, this means that the following attributes are set:

The `events_url` attribute should point to the URL to be used to contact the
`events` web service of the orchestrator. By default this is initialized by
the orchestrator code to point to the production events URL.

The `task_id` attribute should contain the ID of the task. By default this is
initialized by the orchestrator code to be the ID of the task as returned by
the orchestrator services.

The `Task` instance could then be used to perform the following operations:

The `get` method retrieves from the `events` web service the JSON data
associated to the task. The semantic of this data depends on the type of
task. The arguments are an authentication structure and a callback.
The callback receives three parameters. The `Error` parameter indicates
whether there was an error. The `Auth` parameter is a possibly updated
authentication structure.  The `string` parameter is a serialized
JSON containing task data.

The `accept` method accepts the task. The arguments are an authentication
structure and a callback. The callback takes two parameters. The `Error`
parameter indicates whether there was an error. The `Auth` parameter is a
possibly-updated authentication structure.

The `reject` method rejects the task. The arguments are an authentication
structure and a callback. The callback takes two parameters. The `Error`
parameter indicates whether there was an error. The `Auth` parameter is a
possibly-updated authentication structure.

The `done` method informs the orchestrator that a task is done. The
arguments are an authentication structure and a callback. The callback
takes two parameters. The `Error` parameter indicates whether there
was an error. The `Auth` parameter is a possibly-updated authentication
structure.

# GUARANTEES

1. It is safe to let the orchestrator client die, even with slow
   operations like `register_probe` or `update` pending.

2. Callbacks are always delayed, even in case of immediate errors.

# CAVEATS

1. The implementation will schedule any slow operation on a background thread
   used for I/O. As a consequence, depending on the thread from which you
   schedule slow operations, their final callbacks MAY be called from another
   thread context.

2. Slow operations operates on a copy of the internal state, so attribute
   changes performed after a slow operation is started will have no effect
   on such slow operations.

# EXAMPLE

See `example/ooni/oorchestrate.cpp`.

# BUGS

As of MK v0.7.0-alpha, the only implemented operations are `register_probe`
and `update`. All other operations throw `NotImplementedError`.

# HISTORY

The `orchestrate` module appeared in MeasurementKit 0.7.0.
