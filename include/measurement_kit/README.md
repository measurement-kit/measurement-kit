# Measurement Kit API

Starting from Measurement Kit v0.9.0, we expose an API composed of the
following few, public C/C++ headers:

- [engine.h](engine.h): internal C++14 engine, exposed if you define
  `MK_ENGINE_INTERNALS`, does not provide API/ABI guarantees;

- [ffi.h](ffi.h): public [FFI](
  https://en.wikipedia.org/wiki/Foreign_function_interface) friendly
  C-ABI-compatible, hopefully stable C++ API;

- [swig.hpp](swig.hpp): inline C++14 ffi.h wrapper for generating
  code using [SWIG](https://github.com/swig/swig).

Which header to use depends on your needs. If you need a stable API/ABI, we
recommend using `ffi.h`. More on the merits of each header below.

Other backward compatibility headers are present. Since this API is *still
experimental*, we recommend using them (and specifically `nettests.hpp`) for
running tests for now, unless you want to experiment with the new API. The
plan is to keep backwards compatibility by reimplementing some of the old
headers (and specifically `nettests.hpp`) in terms of the new API.

These headers deal with the same underlying concept: Measurement Kit is a
network measurement engine running "tasks" (e.g. network measurements) that
produce "events" (e.g. a log line) during their lifecycle. Furthermore, to
start a task, you need to properly "configure" it. A running test runs in
a background thread, and publishes events on a shared queue that you are
supposed to drain to process events. See the
[example/engine](../../example/engine), [example/ffi](../example/ffi),
and [example/swig](../../example/swig) directories for examples.

## Task settings

The task settings is a JSON like:

```JSON
{
  "annotations": {
    "key": "value",
    "foo": "bar"
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
  "name": "WebConnectivity",
  "options": {
  },
  "output_filepath": "results.txt",
  "verbosity": "INFO"
}
```

As you can see, the following keys are available:

- `"annotations"`: an optional JSON object containing key, value string
  mappings that are copied verbatim in the measurement result file;

- `"disabled_events"`: optional strings array containing the names of
  the events that you are not interested into. All the available event
  names are described below. By default all events are enabled;

- `"inputs"`: optional list of strings to be passed to the task as input. If
  the task does not take any input, this is ignored. If the task requires input
  and you provide neither `"inputs"` nor `"no_input_filepaths"`, the task
  will do nothing;

- `"input_filepaths"`: optional list of strings containing input strings, one
  per line, to be passed to the task. See `"inputs"`;

- `"log_filepath"`: optional string containing the name of the file where to
  write logs. By default logs are written on `stderr`;

- `"name"`: name of the task to run. This setting is mandatory. The available
  task names are described below;

- `"options"`: options modifying the task behavior, as an object mapping
  string keys to string, integer or double values. As the same implies, this
  field is optional;

- `"output_filepath"`: optional file where you want Measurement Kit to
  write measurement results, as a sequence of lines, each line being
  the result of a measurement serialized as JSON;

- `"verbosity"`: how much information you want to see written in the log
  file and emitted by log-related events.

## Verbosity and log levels

The available log levels are:

- `"ERR"`: an error message

- `"WARNING"`: a warning message

- `"INFO"`: an informational message

- `"DEBUG"`: a debugging message

- `"DEBUG2"`: a really specific debugging message

When you specify a verbosity in the settings, only message with a log level
equal or greater than the specified one are emitted. For example, if you
specify `"INFO"`, you will only see `"ERR"`, `"WARNING"`, and `"INFO"` logs.

## Events

An event is a JSON object like:

```JSON
  {
    "key": "<key>",
    "value": {}
  }
```

Where `"value"` is a JSON object with an event specific structure, and `"key"`
is one of the following strings:

- `"failure.measurement"`: TBD
- `"failure.measurement_submission"`: TBD
- `"failure.startup"`: TBD
- `"log"`: TBD
- `"measurement"`: TBD
- `"status.end"`: TBD
- `"status.geoip_lookup"`: TBD
- `"status.progress"`: TBD
- `"status.queued"`: TBD
- `"status.measurement_started"`: TBD
- `"status.measurement_uploaded"`: TBD
- `"status.measurement_done"`: TBD
- `"status.report_created"`: TBD
- `"status.started"`: TBD
- `"status.update.performance"`: TBD
- `"status.update.websites"`: TBD
- `"task_terminated"`: TBD

## Task names

The following task names are defined (case matters):

- `"Dash"`: Neubot's DASH test.
- `"CaptivePortal"`: OONI's captive portal test.
- `"DnsInjection"`: OONI's DNS injection test.
- `"FacebookMessenger"`: OONI's Facebook Messenger test.
- `"HttpHeaderFieldManipulation"`: OONI's HTTP header field manipulation test.
- `"HttpInvalidRequestLine"`: OONI's HTTP invalid request line test.
- `"MeekFrontedRequests"`: OONI's meek fronted requests test.
- `"MultiNdt"`: the multi NDT network performance test.
- `"Ndt"`: the NDT network performance test.
- `"TcpConnect"`: OONI's TCP connect test.
- `"Telegram"`: OONI's Telegram test.
- `"WebConnectivity"`: OONI's Web Connectivity test.
- `"Whatsapp"`: OONI's WhatsApp test.

## Available options

TBD

## engine.h

We do not expect this API to change across versions, yet we do not guarantee
that it will not change. In particular, since this API pulls the
`nlohmann/json` library bundled with Measurement Kit, we expect the ABI
to change as we update the bundled `nlohmann/json` library. For this
reason, this API is only exported if you define `MK_ENGINE_INTERNALS`
when pulling the `engine.h` header. If you don't define that, the header
only exports some C compatible macros to enumerate all events, tasks, etc.

## ffi.h

This API is public and stable. The ABI should not change. After we reach
v1.0.0, any change in this API will cause the major version number of
Measurement Kit to increase, to indicate a breaking change. This is the
API you want to use to maximize compatibility across releases.

## swig.hpp

This is an inline API we use to automatically generate SWIG wrappers. We
do not expect this API to change radically over time, but we may apply
some changes to it when we release a new MK version.
API is only accessible if you define `MK_EXPOSE_SWIG_API`.
