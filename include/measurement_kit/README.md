# Measurement Kit API

Measurement Kit exposes a simple C like API suitable to be used via
[FFI](https://en.wikipedia.org/wiki/Foreign_function_interface).

We also implemented the following higher-level, easier-to-use APIs on top of
this basic C-like FFI-friendly API:

- [Android Java API](https://github.com/measurement-kit/android-libs);

- [Golang API](https://github.com/measurement-kit/go-measurement-kit);

- [C++14 API](cxx14.hpp) (also suitable to be used via ObjectiveC).

We encourage you to select the most programmer-friendly API available.

## Introduction and synopsis

Measurement Kit is a network measurement engine. By default, as mentioned,
it exposes a FFI friendly C like API. To use this API, include
`<measurement_kit/ffi.h>`. See also the API documentation available
at [codedocs.xyz/measurement-kit/measurement-kit](
https://codedocs.xyz/measurement-kit/measurement-kit).

The FFI API allows you to run _tasks_. In most cases tasks are network
measurement tests, like [OONI's Web Connectivity](
https://github.com/ooni/spec/blob/master/test-specs/ts-017-web-connectivity.md
) or the [Network Diagnostic Test](
https://github.com/ooni/spec/blob/master/test-specs/ts-022-ndt.md).

To _start_ a task you call `mk_task_start` by passing it specific
_settings_ as a serialized JSON string.  All settings are optional,
except for the `name` of the task.  Ideally, you should have high
level code where the settings are a class that gets serialized to
a JSON string. This is, for example, what we do in the higher-level
[C++14 API](cxx14.hpp).

Once started, a task will emit _events_. There are several kind of
events, the most common of which is `"log"` that identifies a log
line emitted by the task. Another event is `"status.update.performance"`,
which provides network performance information while a task for
measuring network performance is being run. See below for more
information on the specific events.

The task runs in a separate thread and posts events on a queue. You extract
events from such queue using `mk_task_wait_for_next_event`. This is a _blocking_
function that returns when a new event is posted into the queue. To
process an event, use `mk_event_serialize` to obtain its JSON serialization,
then parse the JSON into some high level data structure, and process it. See,
again, the [C++14 API](cxx14.hpp) events processing code to have an idea of
how this could be implemented. (Or, if we have already written an API that
does that works for your use case, perhaps just use such API.)

You should loop processing events until `mk_task_is_done` returns nonzero. At
that point, the task is done, and attempting to extract further events from
the queue with `mk_task_wait_for_next_event` will immediately return the dummy
`status.terminated` event (equivalent to `EOF` for the task queue).

Since the FFI API is basically a C API, you need to manually manage memory
by freeing events and the task, once you are done with them. To this end, use
respectively, `mk_event_destroy` and `mk_task_destroy`.

## Examples

You can find working examples of usage of the FFI API inside the
[example/ffi](../../example/ffi) directory. Another usage example of
the FFI API is [cxx14.hpp](cxx14.hpp). In such file, we wrap the FFI API
into a more-user-friendly C++14 interface, by mapping each event to
a class. Specifically, it may be of interest to read how we
process events in the `run()` and `process_event()` functions.

## Tasks

The following tasks are defined (case matters):

- `"Dash"`: [Neubot's DASH test](
https://github.com/ooni/spec/blob/master/test-specs/ts-021-dash.md).

- `"CaptivePortal"`: [OONI's captive portal test](
  https://github.com/ooni/spec/blob/master/test-specs/ts-010-captive-portal.md).

- `"DnsInjection"`: [OONI's DNS injection test](
  https://github.com/ooni/spec/blob/master/test-specs/ts-012-dns-injection.md).

- `"FacebookMessenger"`: [OONI's Facebook Messenger test](
  https://github.com/ooni/spec/blob/master/test-specs/ts-019-facebook-messenger.md).

- `"HttpHeaderFieldManipulation"`: [OONI's HTTP header field manipulation test](
  https://github.com/ooni/spec/blob/master/test-specs/ts-006-header-field-manipulation.md).

- `"HttpInvalidRequestLine"`: [OONI's HTTP invalid request line test](
  https://github.com/ooni/spec/blob/master/test-specs/ts-007-http-invalid-request-line.md).

- `"MeekFrontedRequests"`: [OONI's meek fronted requests test](
  https://github.com/ooni/spec/blob/master/test-specs/ts-014-meek-fronted-requests.md).

- `"MultiNdt"`: [the multi NDT network performance test](
  https://github.com/ooni/spec/blob/master/test-specs/ts-022-ndt.md).

- `"Ndt"`: [the NDT network performance test](
  https://github.com/ooni/spec/blob/master/test-specs/ts-022-ndt.md).

- `"TcpConnect"`: [OONI's TCP connect test](
   https://github.com/ooni/spec/blob/master/test-specs/ts-008-tcp-connect.md).

- `"Telegram"`: [OONI's Telegram test](
  https://github.com/ooni/spec/blob/master/test-specs/ts-020-telegram.md).

- `"WebConnectivity"`: [OONI's Web Connectivity test](
  https://github.com/ooni/spec/blob/master/test-specs/ts-017-web-connectivity.md).

- `"Whatsapp"`: [OONI's WhatsApp test](
  https://github.com/ooni/spec/blob/master/test-specs/ts-018-whatsapp.md).

By following the links provided above, you can read detailed documentation on
the purpose of each task. The documentation also describes the JSON schema used
by each task to represent its results. As mentioned below, tasks results are
emitted as a serialized JSON under the `"measurement"` event.

## Settings

The task settings is a JSON like:

```JSON
{
  "annotations": {
    "an_annotation": "with its value",
    "another_annotation": "with its specific value"
  },
  "disabled_events": [
    "status.queued",
    "status.started"
  ],
  "inputs": [
    "www.google.com",
    "www.x.org"
  ],
  "input_filepaths": [
    "/path/to/file",
    "/path/to/another/file"
  ],
  "log_filepath": "logfile.txt",
  "log_level": "INFO",
  "name": "WebConnectivity",
  "options": {
    "bouncer_base_url": "",
    "collector_base_url": "",
    "dns/nameserver": "",
    "dns/engine": "system",
    "geoip_asn_path": "",
    "geoip_country_path": "",
    "ignore_bouncer_error": 1,
    "ignore_open_report_error": 1,
    "max_runtime": -1.0,
    "net/ca_bundle_path": "",
    "net/timeout": 10.0,
    "no_bouncer": 0,
    "no_collector": 0,
    "no_asn_lookup": 0,
    "no_cc_lookup": 0,
    "no_ip_lookup": 0,
    "no_file_report": 0,
    "no_resolver_lookup": 0,
    "probe_asn": "",
    "probe_cc": "",
    "probe_ip": "",
    "randomize_input": 1,
    "save_real_probe_asn": 1,
    "save_real_probe_cc": 1,
    "save_real_probe_ip": 0,
    "save_real_resolver_ip": 1,
    "software_name": "measurement_kit",
    "software_version": "<current-mk-version>"
  },
  "output_filepath": "results.njson"
}
```

The only mandatory key is `name`, which identifies the task. All the other
keys are optional. Above we have shown the most commonly used `options`, that
are described in greater detail below. The value we included for options
is their default value (_however_, the value of non-`options` settings _is not_
the default value, rather is a meaningful example). The following keys
are available:

- `"annotations"`: (object; optional) JSON object containing key, value string
  mappings that are copied verbatim in the measurement result file;

- `"disabled_events"`: (array; optional) strings array containing the names of
  the events that you are not interested into. All the available event
  names are described below. By default all events are enabled;

- `"inputs"`: (array; optional) array of strings to be passed to the task as
  input. If the task does not take any input, this is ignored. If the task
  requires input and you provide neither `"inputs"` nor `"input_filepaths"`,
  the task will fail;

- `"input_filepaths"`: (array; optional) array of files containing input
  strings, one per line, to be passed to the task. These files are read and
  their content is merged with the one of the `inputs` key.

- `"log_filepath"`: (string; optional) name of the file where to
  write logs. By default logs are written on `stderr`;

- `"log_level"`: (string; optional) how much information you want to see
  written in the log file and emitted by log-related events.

- `"name"`: (string; mandatory) name of the task to run. The available
  task names have been described above;

- `"options"`: (object; optional) options modifying the task behavior, as
  an object mapping string keys to string, int or float values. As mentioned,
  the above example shows the default value of options. Also, there are no
  boolean options, but we use `int` with boolean semantics, i.e., we use
  `0` to indicate `false`, and nonzero to indicate `true`;

- `"output_filepath"`: (string; optional) file where you want MK to
  write measurement results, as a sequence of lines, each line being
  the result of a measurement serialized as JSON. If you do not specify
  an output file, Measurement Kit will write the test results in a
  file in the current working directory.

## Log levels

The available log levels are:

- `"ERR"`: an error message

- `"WARNING"`: a warning message

- `"INFO"`: an informational message

- `"DEBUG"`: a debugging message

- `"DEBUG2"`: a really specific debugging message

When you specify a log level in the settings, only messages with a log level
equal or greater than the specified one are emitted. For example, if you
specify `"INFO"`, you will only see `"ERR"`, `"WARNING"`, and `"INFO"` logs.

## Options

Options can be `string`, `float`, or `int`. There is not boolean type, and
we use `int`s with boolean semantics in some cases, with the usual convention
that `0` means false and non-`0` means true.

These are the available options:

- `"bouncer_base_url"`: (string) base URL of OONI bouncer, by default set to
  the empty string. If empty, the OONI bouncer will be used;

- `"collector_base_url"`: (string) base URL of OONI collector, by default set
  to the empty string. If empty, the OONI collector will be used;

- `"dns/nameserver"`: (string) nameserver to be used with non-`system` DNS
  engines. Can or cannot include an optional port number. By default, set
  to the empty string;

- `"dns/engine"`: (string) what DNS engine to use. By default, set to
  `"system"`, meaning that `getaddrinfo()` will be used to resolve domain
  names. Can also be set to `"libevent"`, to use libevent's DNS engine.
  In such case, you must provide a `"dns/nameserver"` as well;

- `"geoip_asn_path"`: (string) path to the GeoIP ASN database file. By
  default not set;

- `"geoip_country_path"`: (string) path to the GeoIP Country database
  file. By default not set;

- `"ignore_bouncer_error"`: (int) whether to ignore an error in contacting
  the OONI bouncer. By default set to `1` so that bouncer errors will
  be ignored;

- `"ignore_open_report_error"`: (int) whether to ignore an error when opening
  the report with the OONI collector. By default set to `1` so that errors
  will be ignored;

- `"max_runtime"`: (float) number of seconds after which the test will
  be stopped. Works _only_ for tests taking input. By default set to `-1.0`
  so that there is no maximum runtime for tests with input;

- `"net/ca_bundle_path"`: (string) path to the CA bundle path to be used
  to validate SSL certificates. Not necessary on platforms where we use
  LibreSSL, because in such case we include a default CA bundle directly
  inside of Measurement Kit (Android, iOS, Windows). If you compile a
  Measurement Kit for yourself, then YMMV;

- `"net/timeout"`: (float) number of seconds after which network I/O
  operations will timeout. By default set to `10.0` seconds;

- `"no_bouncer"`: (int) whether to use a bouncer. By default set to
  `0`, meaning that a bouncer will be used;

- `"no_collector"`: (int) whether to use a collector. By default set to
  `0`, meaning that a collector will be used;

- `"no_asn_lookup"`: (int) whether to lookup the Autonomous System Number
  in which we're running. Requires the `"geoip_asn_path"` to be set. By
  default set to `1`, meaning that we'll attempt ASN lookup;

- `"no_cc_lookup"`: (int) whether to lookup the code of the country (CC) in
  which we are. Requires the `"geoip_country_path"` to be set. By default,
  set to `1`, meaning that we'll attempt CC lookup;

- `"no_ip_lookup"`: (int) whether to lookup our IP address. By default set
  to `1`, meaning that we'll try. Seting this to `0` prevents us from
  looking up also the ASN and the CC and will also prevent us from attempting
  to scrub the user IP address from the results of many OONI tests;

- `"no_file_report"`: (int) whether to write a report (i.e. measurement
  result) file on disk. By default set to `0`, meaning that we'll try;

- `"no_resolver_lookup"`: (int) whether to lookup the IP address of the
  resolver used. By default set to `0`, meaning that we'll try;

- `"probe_asn"`: (string) ASN in which we are. Set this if you already
  looked up for the ASN. Setting this to a non-empty string will disable
  the ASN lookup. By default it's an empty string;

- `"probe_cc"`: (string) Country code. Set this is you already looked up
  the country code. Setting this to a non-empty string will disable CC
  lookup. By default it's an empty string;

- `"probe_ip"`: (string) Probe IP. Set this if you already know our
  IP. Setting this to a non-empty string will disable the probe IP
  lookup. By default it's an empty string;

- `"randomize_input"`: (int) whether to randomize input. By default set to
  `1`, meaning that we'll randomize input;

- `"save_real_probe_asn"`: (int) whether to save the ASN. By default set
  to `1`, meaning that we will save it;

- `"save_real_probe_cc"`: (int) whether to save the CC. By default set to `1`,
  meaning that we will save it;

- `"save_real_probe_ip"`: (int) whether to save our IP. By default set to `0`,
  meaning that we will not save it;

- `"save_real_resolver_ip"`: (int) whether to save the resolver IP. By default
  set to `1`, meaning that we'll save it;

- `"software_name"`: (string) name of the app. By default set to
  `"measurement_kit"`;

- `"software_version"`: (string) version of the app. By default set to the
  current version of Measurement Kit.

## Events

An event is a JSON object like:

```JSON
  {
    "key": "<key>",
    "value": {}
  }
```

Where `"value"` is a JSON object with an event-specific structure, and `"key"`
is a string. Below we describe all the possible event keys, along with the
"value" JSON structure. Unless otherwise specified, an event key can be emitted
an arbitrary number of times during the lifecycle of a task. Unless otherwise
specified, all the keys introduced below where added in MK v0.9.0.

- `"failure.asn_lookup"`: (object) There was a failure attempting to lookup the
  user autonomous system number. The JSON returned by this event is like:

```JSON
{
  "key": "failure.asn_lookup",
  "value": {
    "failure": "<failure_string>"
  }
}
```

Where all the possible values of `<failure_string>` are described below.

- `"failure.cc_lookup"`: (object) There was a failure attempting to lookup the
  user country code. The JSON returned by this event is like:

```JSON
{
  "key": "failure.cc_lookup",
  "value": {
    "failure": "<failure_string>"
  }
}
```

Where all the possible values of `<failure_string>` are described below.

- `"failure.ip_lookup"`: (object) There was a failure attempting to lookup the
  user IP address. The JSON returned by this event is like:

```JSON
{
  "key": "failure.ip_lookup",
  "value": {
    "failure": "<failure_string>"
  }
}
```

Where all the possible values of `<failure_string>` are described below.

- `"failure.measurement"`: (object) There was a failure running the
  measurement. The complete JSON returned by this event is like:

```JSON
{
  "key": "failure.measurement",
  "value": {
    "failure": "<failure_string>"
  }
}
```

Where all the possible values of `<failure_string>` are described below.

- `"failure.measurement_submission"`: (object) There was a failure in
submitting the measurement result to the configured collector (if any). The
complete JSON returned by this event is like:

```JSON
{
  "key": "failure.measurement_submission",
  "value": {
    "idx": 0,
    "json_str": "<serialized_result>",
    "failure": "<failure_string>"
  }
}
```

Where `idx` is the index of the current measurement, which is relevant for the
tests that run over an input list; `json_str` is the measurement that we failed
to submit, serialized as JSON; `failure` is the error that occurred.

- `"failure.report_create"`: (object) There was a failure in creating the
measurement result to the configured collector (if any). The complete JSON
returned by this event is like:

```JSON
{
  "key": "failure.report_create",
  "value": {
    "failure": "<failure_string>"
  }
}
```

Where `failure` is the error that occurred.

- `"failure.report_close"`: (object) There was a failure in closing the
measurement result to the configured collector (if any). The complete JSON
returned by this event is like:

```JSON
{
  "key": "failure.report_close",
  "value": {
    "failure": "<failure_string>"
  }
}
```

Where `failure` is the error that occurred.

- `"failure.resolver_lookup"`: (object) There was a failure attempting to
  lookup the user DNS resolver. The JSON returned by this event is like:

```JSON
{
  "key": "failure.resolver_lookup",
  "value": {
    "failure": "<failure_string>"
  }
}
```

Where all the possible values of `<failure_string>` are described below.

- `"failure.startup"`: (object) There was a failure starting the test, most
likely because you passed in incorrect options. The complete JSON returned by
this event is like:

```JSON
{
  "key": "failure.startup",
  "value": {
    "failure": "<failure_string>"
  }
}
```

Where `<failure_string>` is the error that occurred.

- `"log"`: (object) A log line was emitted. The complete JSON is like:

```JSON
{
  "key": "log",
  "value": {
    "log_level": "<a_log_level>",
    "message": "<string>"
  }
}
```

Where `log_level` is one of the log levels described above, and `message`
is the log message emitted by Measurement Kit.

- `"measurement"`: (object) The result of a measurement. The complete JSON
is like:

```JSON
{
  "key": "measurement",
  "value": {
    "idx": 0,
    "json_str": "<serialized_result>"
  }
}
```

Where `json_str` is the measurement result serialized as JSON. The schema of
a measurement result depends on the type of task, as described below. And
where `idx` is the index of the current measurement (relevant only for tasks
that iterate over an input list).

- `"status.end"`: (object) This event is emitted just once at the end of the
test. The complete JSON is like:

```JSON
{
  "key": "status.end",
  "value": {
    "downloaded_kb": 0.0,
    "uploaded_kb": 0.0,
    "failure": "<failure_string>"
  }
}
```

Where `downloaded_kb` and `uploaded_kb` are the amount of downloaded and
uploaded kilo-bytes, and `failure` is the overall failure that occurred during
the test (or the empty string, if no error occurred). This event is always
emitted, regardless of whether the task arrives naturally to its end or instead
is interrupted. As such, you can rely on this event as a "once" event suitable
for clearing up resources allocated as part of the task lifecycle.

- `"status.geoip_lookup"`: (object) This event is emitted only once at the
beginning of the task, and provides information about the user's IP address,
country and autonomous system. In detail, the JSON is like:

```JSON
{
  "key": "status.geoip_lookup",
  "value": {
    "probe_ip": "<ip_address>",
    "probe_asn": "<asn>",
    "probe_cc": "<cc>",
    "probe_network_name": "<network_name>"
  }
}
```

Where `<ip_address>` is the user's IP address, `asn` is the autonomous
system number, `cc` is the country code, `network_name` is the commercial
name associated to the autonomous system number.

- `"status.progress"`: (object) This is emitted during the task lifecycle to
inform you about the task progress towards completion. In detail, the JSON is
like:

```JSON
{
  "key": "status.progress",
  "value": {
    "percentage": 0.0,
    "message": "<string>"
  }
}
```

Where `percentage` is the percentage of completion of the task, and `message`
indicates the operation that the task just completed.

- `"status.queued"`: (object) Indicates that the task has been accepted. In
case there are already running tasks, as mentioned above, they will be
prevented from running concurrently. The JSON is like:

```JSON
{
  "key": "status.queued",
  "value": {
  }
}
```

Where `value` is empty.

- `"status.measurement_start"`: (object) Indicates that a measurement inside
a task has started. The JSON is like:

```JSON
{
  "key": "status.measurement_start",
  "value": {
    "idx": 0,
    "input": "<input>"
  }
}
```

Where `idx` is the index of the current input and `input` is the current
input. For tests that take no input, this event MAY be emitted with
`idx` equal to `0` and `input` equal to the empty string.

- `"status.measurement_submission"`: (object) The specific measurement has
been uploaded successfully. The JSON is like:

```JSON
{
  "key": "status.measurement_submission",
  "value": {
    "idx": 0
  }
}
```

Where `idx` is the index of the measurement input.

- `"status.measurement_done"`: (object) Measurement Kit has finished processing
the specified input. The JSON is like:

```JSON
{
  "key": "status.measurement_done",
  "value": {
    "idx": 0
  }
}
```

Where `idx` is the index of the measurement input.

- `"status.report_close"`: (object) Measurement Kit has closed a report for the
current task, and tells you the report-ID. The report-ID is the identifier of
the measurement result(s), which have been submitted. The JSON is like:

```JSON
{
  "key": "status.report_close",
  "value": {
    "report_id": "string"
  }
}
```

Where `report_id` is the report identifier.

- `"status.report_create"`: (object) Measurement Kit has created a report for
the current task, and tells you the report-ID. The report-ID is the identifier
of the measurement result(s), which will be later submitted. The JSON is like:

```JSON
{
  "key": "status.report_create",
  "value": {
    "report_id": "string"
  }
}
```

Where `report_id` is the report identifier.

- `"status.resolver_lookup"`: (object) This event is emitted only once at the
beginning of the task, when the IP address of the resolver is discovered. The
JSON is like:

```JSON
{
  "key": "status.resolver_lookup",
  "value": {
    "resolver_ip": "<ip_address>"
  }
}
```

Where `<ip_address>` is the resolver's IP address.

- `"status.started"`: (object) The task has started, and the JSON is like:

```JSON
{
  "key": "status.started",
  "value": {
  }
}
```

Where `value` is empty.

- `"status.update.performance"`: (object) This is an event emitted by tests that
measure network performance. The JSON is like:

```JSON
{
  "key": "status.update.performance",
  "value": {
    "direction": "<direction>",
    "elapsed": 0.0,
    "num_streams": 0,
    "speed_kbps": 0.0
  }
}
```

Where `direction` is either "download" or "upload", `elapsed` is the elapsed
time (in seconds) since the measurement started, `num_streams` is the number of
streams we are using to measure performance, `speed_kbps` is the speed, in
kbit/s, measured since the previous performance measurement.

- `"status.update.websites"`: (object) This is an event emitted by tests that
measure the reachability of websites. The JSON is like:

```JSON
{
  "key": "status.update.websites",
  "value": {
    "url": "<url>",
    "status": "<status>"
  }
}
```

Where `url` is the URL we're measuring and `status` is either `accessible`
or `blocking`.

- `"task_terminated"`: (object) This event is emitted when you attempt to
extract events from the task queue, but the task is not running anymore (i.e.
it's the equivalent of `EOF` for the task queue). The related JSON is like:

```JSON
{
  "key": "status.terminated",
  "value": {
  }
}
```

Where `value` is empty.

## Task pseudocode

The following illustrates in pseudocode the operations performed by a task
once you call `mk_task_start`. It not 100% accurate; in particular, we have
omitted the code that generates most log messages. This pseudocode is meant to
help you understand how Measurement Kit works internally, and specifically how all
the settings described above interact together when you specify them for
running Measurement Kit tasks. We are using pseudo JavaScript because that
is the easiest language to show manipulation of JSON objects such as the
`settings` object.

As mentioned, a task run in its own thread. It first validate settings, then
it opens the logfile (if needed), and finally it waits in queue until other
possibly running tasks terminate. The `finish` function will be called when the
task is done, and will emit all the events emitted at the end of a task.

```JavaScript
function taskThread(settings) {
  emitEvent("status.queued", {})
  semaphore.Acquire()                 // blocked until my turn

  let finish = function(error) {
    semaphore.Release()               // allow another test to run
    emitEvent("status.end", {
      downloaded_kb: countDownloadedKb(),
      uploaded_kb: countUploadedKb(),
      failure: (error) ? error.AsString() : null
    })
  }

  if (!settingsAreValid(settings)) {
    emitEvent("failure.startup", {
      failure: "value_error",
    })
    finish("value_error")
    return
  }

  if (settings.log_filepath != "") {
    // TODO(bassosimone): we should decide whether we want to deal with the
    // case where we cannot write into the log file. Currently we don't.
    openLogFile(settings.log_filepath)
  }

  let task = makeTask(settings.name)

  emitEvent("status.started", {})


```

After all this setup, a task contacts the OONI bouncer, lookups the IP address,
the country code, the autonomous system number, and the resolver lookup. All
these information end up in the JSON measurement. Also, all these operations can
be explicitly disabled by setting the appropriate settings.

```JavaScript
  let test_helpers = test.defaultTestHelpers()
  if (!settings.options.no_bouncer) {
    if (settings.options.bouncer_base_url == "") {
      settings.options.bouncer_base_url = defaultBouncerBaseURL()
    }
    let error
    [test_helpers, error] = queryOONIBouncer(settings)
    if (error) {
      emitWarning(settings, "cannot query OONI bouncer")
      if (!settings.options.ignore_bouncer_error) {
        finish(error)
        return
      }
    }
  }

  // TODO(bassosimone): we should make sure the progress described here
  // is consistent with the one emitted by the real code.
  emitProgress(0.1, "contacted bouncer")

  let probe_ip = "127.0.0.1"
  if (settings.options.probe_ip != "") {
    probe_ip = settings.options.probe_ip
  } else if (!settings.options.no_ip_lookup) {
    let error
    [probe_ip, error] = lookupIP(settings)
    if (error) {
      emitEvent("failure.ip_lookup", {
        failure: error.AsString()
      })
      emitWarning(settings, "cannot lookup probe IP")
    }
  }

  let probe_asn = "AS0",
      probe_network_name = "Unknown"
  if (settings.options.probe_asn != "") {
    probe_asn = settings.options.probe_asn
  } else if (!settings.options.no_asn_lookup &&
             settings.options.geoip_asn_path != "") {
    let error
    [probe_asn, probe_network_name, error] = lookupASN(settings)
    if (error) {
      emitEvent("failure.asn_lookup", {
        failure: error.AsString()
      })
      emitWarning(settings, "cannot lookup probe ASN")
    }
  }

  let probe_cc = "ZZ"
  if (settings.options.probe_cc != "") {
    probe_cc = settings.options.probe_cc
  } else if (!settings.options.no_cc_lookup &&
             settings.options.geoip_country_path != "") {
    let error
    [probe_cc, error] = lookupCC(settings)
    if (error) {
      emitEvent("failure.cc_lookup", {
        failure: error.AsString()
      })
      emitWarning(settings, "cannot lookup probe CC")
    }
  }

  emitEvent("status.geoip_lookup", {
    probe_ip: probe_ip,
    probe_asn: probe_asn,
    probe_network_name: probe_network_name,
    probe_cc: probe_cc
  })

  emitProgress(0.2, "geoip lookup")

  // TODO(bassosimone): take decision wrt null vs. ""
  let resolver_ip = null
  if (!settings.options.no_resolver_lookup) {
    let error
    [resolver_ip, error] = lookupResolver(settings)
    if (error) {
      emitEvent("failure.resolver_lookup", {
        failure: error.AsString()
      })
      emitWarning(settings, "cannot lookup resolver IP")
    }
  }

  emitEvent("status.resolver_lookup", {
    resolver_ip: resolver_ip
  })
  emitProgress(0.3, "resolver lookup")
```

Then, Measurement Kit opens the report file on disk, which will contain
the measurements, each serialized on a JSON on its own line. It will also
contact the OONI bouncer and obtain a report-ID for the report.

```JavaScript
  if (!settings.options.no_file_report) {
    if (settings.output_filepath == "") {
      settings.output_filepath = makeDefaultOutputFilepath(settings);
    }
    let error = openFileReport(settings.output_filepath)
    if (error) {
      emitWarning(settings, "cannot open file report")
      finish(error)
      return
    }
  }

  let report_id
  if (!settings.options.no_collector) {
    if (settings.options.collector_base_url == "") {
      settings.options.collector_base_url = defaultCollectorBaseURL();
    }
    let error
    [report_id, error] = collectorOpenReport(settings)
    if (error) {
      emitWarning("cannot open report with OONI collector")
      emitEvent("failure.report_create", {
        failure: error.AsString()
      })
      if (!settings.options.ignore_open_report_error) {
        finish(error)
        return
      }
    } else {
      emitEvent("status.report_create", {
        report_id: report_id
      })
    }
  }

  emitProgress(0.4, "open report")
```

Then comes input processing. Measurement Kit assembles a list of inputs to
process. If the test do not take any input, we fake a single input entry
consisting of the empty string, implying that this test needs to perform just
a single iteration. (This is a somewhat internal detail, but it explains
some events with `idx` equal to `0` and `input` equal to an empty string.)

```JavaScript
  for (let i = 0; i < settings.input_filepaths.length; ++i) {
    let [inputs, error] = readInputFile(settings.input_filepaths[i])
    if (error) {
      emitWarning("cannot read input file")
      finish(error)
      return
    }
    settings.inputs = settings.inputs.concat(inputs)
  }
  if (settings.inputs.length <= 0) {
    if (task.needs_input) {
      emitWarning(settings, "no input provided")
      finish(Error("value_error"))
      return
    }
    settings.inputs.push("") // empty input for input-less tests
  }
  if (settings.options.randomize_input) {
    shuffle(settings.input)
  }
```

Then, Measurement Kit iterates over all the input and runs the function
implementing the specified task on each input.

```JavaScript
  let begin = timeNow()
  for (let i = 0; i < settings.inputs; ++i) {
    let currentTime = timeNow()
    if (settings.options.max_runtime >= 0 &&
        currentTime - begin > settings.options.max_runtime) {
      emitWarning("exceeded maximum runtime")
      break
    }
    emitEvent("status.measurement_start", {
      idx: i,
      input: settings.inputs[i]
    })
    let measurement = Measurement()
    measurement.annotations = settings.annotations
    measurement.annotations.engine_name = "libmeasurement_kit"
    measurement.annotations.engine_version = mkVersion()
    measurement.annotations.engine_version_full = mkVersionFull()
    measurement.annotations.platform = platformName()
    measurement.annotations.probe_network_name = settings.options.save_real_probe_asn
                                                  ? probe_network_name : "Unknown"
    measurement.id = UUID4()
    measurement.input = settings.inputs[i]
    measurement.input_hashes = []
    measurement.measurement_start_time = currentTime
    measurement.options = []
    measurement.probe_asn = settings.options.save_real_probe_asn ? probe_asn : "AS0"
    measurement.probe_cc = settings.options.save_real_probe_cc ? probe_cc : "ZZ"
    measurement.probe_ip = settings.options.save_real_probe_ip
                              ? probe_ip : "127.0.0.1"
    measurement.report_id = report_id
    measurement.sotfware_name = settings.options.software_name
    measurement.sotfware_version = settings.options.software_version
    measurement.test_helpers = test_helpers
    measurement.test_name = test.AsOONITestName()
    measurement.test_start_time = begin
    measurement.test_verson = test.Version()
    let [test_keys, error] = task.Run(
          settings.inputs[i], settings, test_helpers)
    measurement.test_runtime = timeNow() - currentTime
    measurement.test_keys = test_keys
    measurement.test_keys.resolver_ip = settings.options.save_resolver_ip
                                          ? resolve_ip : "127.0.0.1"
    if (error) {
      emitEvent("failure.measurement", {
        failure: error.AsString(),
        idx: i,
        input: settings.inputs[i]
      })
    }
    emitEvent("measurement", {
      json_str: measurement.asJSON(),
      idx: i,
      input: settings.inputs[i]
    })
    if (!settings.options.no_file_report) {
      let error = writeReportToFile(measurement)
      if (error) {
        emitWarning("cannot write report to file")
        finish(error)
        return
      }
    }
    if (!settings.options.no_collector) {
      let error = submitMeasurementToOONICollector(measurement)
      if (error) {
        emitEvent("failure.measurement_submission", {
          idx: i,
          input: settings.inputs[i],
          json_str: measurement.asJSON(),
          failure: error.AsString()
        })
      } else {
        emitEvent("status.measurement_submission", {
          idx: i,
          input: settings.inputs[i],
        })
      }
    }
    emitEvent("status.measurement_done", {
      idx: i
    })
  }
```

Finally, Measurement Kit ends the test by closing the local results file
and the remote report managed by the OONI collector.

```JavaScript
  emitProgress(0.9, "ending the test")

  if (!settings.options.no_file_report) {
    error = closeFileReport()
    if (error) {
      emitWarning("cannot close file report")
      finish(error)
      return
    }
  }
  if (!settings.options.no_collector) {
    let error = closeRemoteReport()
    if (error) {
      emitEvent("failure.report_close", {
        failure: error.AsString()
      })
      emitWarning("cannot close remote report with OONI collector")
    } else {
      emitEvent("status.report_close", {
        report_id: report_id
      })
    }
  }

  finish()
}
```
