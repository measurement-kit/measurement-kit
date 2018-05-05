# Measurement Kit API

Measurement Kit only exposes a simple C like API suitable to be used via
[FFI](https://en.wikipedia.org/wiki/Foreign_function_interface).

We implemented the following higher-level, easier-to-use APIs on top of
this basic C-like FFI-friendly API:

- [iOS ObjectiveC API](https://github.com/measurement-kit/ios-libs);

- [Android Java API](https://github.com/measurement-kit/android-libs);

- [Golang API](https://github.com/measurement-kit/go-measurement-kit);

- [C++11 API](https://github.com/measurement-kit/cxx11-api).

We encourage you to avoid using it when a more user-friendly API is available.

## Synopsis

```C++
#include <measurement_kit/ffi.h>

typedef          struct mk_event_   mk_event_t;
typedef          struct mk_task_    mk_task_t;

mk_task_t       *mk_task_start(const char *settings);
mk_event_t      *mk_task_wait_for_next_event(mk_task_t *task);
int              mk_task_is_done(mk_task_t *task);
void             mk_task_interrupt(mk_task_t *task);

const char      *mk_event_serialize(mk_event_t *event);
void             mk_event_destroy(mk_event_t *event);

void             mk_task_destroy(mk_task_t *task);
```

## Introduction

Measurement Kit is a network measurement engine. It runs _tasks_ (e.g. a
specific network measurement). To start a task, you need to configure with
specific _settings_. Among these settings there is the most important one, the
task name (e.g. "Ndt" is the task name of the NDT network performance test).
While running, a task emits _events_ (e.g. a log line, a JSON describing the
result of the measurement, and other intermediate results).

Tasks run sequentially. Starting several tasks concurrently means that they will
run in FIFO order. Each task will run in a separate thread. This thread will
post events on a shared queue that you should drain, by extracting and
processing events emitted by the task. You should loop extracting events from
such queue as long as the task is running.

## API documentation

`mk_task_start` starts a task with the configuration provided as a serialized
JSON. Returns `NULL` if `conf` was `NULL`, or in case of parse error. You
own (and must destroy) the returned task pointer.

`mk_task_wait_for_next_event` blocks waiting for the `task` to emit the next
event. Returns `NULL` if `task` is `NULL` or on internal error. If the task is
terminated, it returns immediately a `task.terminated` event. You own (and
must destroy) the returned event pointer.

`mk_task_is_done` returns zero when the tasks is running, nonzero otherwise. If
the `task` is `NULL`, nonzero is returned.

`mk_task_interrupt` interrupts a running `task`. Interrupting a `NULL` task
has no effect.

`mk_event_serialize` obtains the JSON serialization of `event`. Return `NULL`
if either the `event` is `NULL` or there is an internal error.

`mk_event_destroy` destroys the memory associated with `event`. Attempting to
destroy a `NULL` event has no effect.

`mk_task_destroy` destroys the memory associated with `task`. Attempting to
destroy a `NULL` task has no effect. Attempting to destroy a running `task` will
wait for the task to complete before releasing memory.

## Example

The following example runs the "Ndt" test with "INFO" verbosity.

```C++
  const char *settings = R"({
    "name": "Ndt",
    "log_level": "INFO"
  })";
  mk_task_t *task = mk_task_start(settings);
  if (!task) {
    std::clog << "ERROR: cannot start task" << std::endl;
    return;
  }
  while (!mk_task_is_done(task)) {
    mk_event_t *event = mk_task_wait_for_next_event(task);
    if (!event) {
      std::clog << "ERROR: cannot wait for next event" << std::endl;
      break;
    }
    const char *json_serialized_event = mk_event_serialize(event);
    if (!json_serialized_event) {
      std::clog << "ERROR: cannot serialize event" << std::endl;
      break;
    }
    {
      // TODO: process the JSON-serialized event
    }
    mk_event_destroy(event);
  }
  mk_task_destroy(task);
```

## Task pseudocode

The following illustrates in pseudocode the operations performed by a task
once you call `mk_task_start`. It not 100% accurate, rather it's meant to help
you understand how Measurement Kit works internally.

As mentioned, a task run in its own thread. It first validate settings, then
it opens the logfile (if needed), and finally it waits in queue until other
possibly running tasks terminate. The `finish` function will be called when the
task is done, and will emit all the events emitted at the end of a task.

```JavaScript
function taskThread(settings) {
  if (!settingsAreValid(settings)) {
    emitEvent("status.failure_startup", {
      failure: "value_error",
    })
    emitEvent("status.terminated", {})
    return
  }

  if (settings.log_filepath != "") {
    openLogFile(settings.log_filepath)
  }

  let task = MakeTask(settings.name)

  emitEvent("status.queued", {})
  semaphore.Acquire()                 // blocked until my turn
  emitEvent("status.started", {})

  let finish = function(error) {
    semaphore.Release()               // allow another test to run
    emitEvent("status.end", {
      downloaded_kb: countDownloadedKb(),
      uploaded_kb: countUploadedKb(),
      failure: error.AsString()
    })
    emitEvent("status.terminated", {})
  }

```

After all this setup, a task contacts the OONI bouncer, lookups the IP address,
the country code, the autonomous system number, and the resolver lookup. All
these information end up in the JSON measurement. Also, all these operations can
be explicitly disabled by setting the appropriate settings.

```JavaScript
  if (!settings.options.no_bouncer) {
    let error = queryOONIBouncer()
    if (error && !settings.options.ignore_bouncer_error) {
      finish(error)
      return
    }
  }

  emitProgress(0.025, "contacted bouncer")

  let probe_ip = "127.0.0.1"
  if (settings.options.probe_ip != "") {
    probe_ip = settings.options.probe_ip
  } else if (!settings.options.no_ip_lookup) {
    let error = lookupIP(settings)
    if (error) {
      emitWarning(settings, "cannot lookup probe IP")
    }
  }

  let probe_asn = "AS0"
  if (settings.options.probe_asn != "") {
    probe_asn = settings.options.probe_asn
  } else if (!settings.options.no_asn_lookup &&
             settings.options.geoip_asn_path != "") {
    let error
    [probe_asn, error] = lookupASN(settings)
    if (error) {
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
      emitWarning(settings, "cannot lookup probe CC")
    }
  }

  emitProgress(0.05, "geoip lookup")

  let resolver_ip
  if (!settings.options.no_resolver_lookup) {
    let error
    [resolver_ip, error] = lookupResolver()
    if (error) {
      emitWarning(settings, "cannot lookup resolver IP")
    }
  }

  emitProgress(0.075, "resolver lookup")
```

Then, Measurement Kit opens the report file on disk, which will contain
the measurements, each serialized on a JSON on its own line. It will also
contact the OONI bouncer and obtain a report-ID for the report.

```JavaScript
  if (!settings.options.no_file_report) {
    let error = openFileReport(settings.output_filepath)
    if (error) {
      finish(error)
      return
    }
  }

  if (!settings.options.no_collector) {
    let [report_id, error] = openRemoteReport(settings)
    if (error) {
      if (!settings.options.ignore_open_report_error) {
        finish(error)
        return
      }
      emitWarning("cannot open report")
    } else {
      emitEvent("status.report_created", {
        report_id: report_id
      })
    }
  }

  emitProgress(0.1, "open report")
```

Then comes input processing. Measurement Kit assembles a list of inputs to
process. If the test do not take any input, we fake a single input entry
consisting of the empty string, implying that this test needs to perform just
a single iteration. (This is a somewhat internal detail, but it explains
some events with `idx` equal to `0` and `input equal to an empty string.)

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
  let begin = TimeNow()
  for (let i = 0; i < settings.inputs; ++i) {
    if (settings.options.max_runtime >= 0 &&
        TimeNow() - begin > settings.options.max_runtime) {
      emitWarning("exceeded maximum runtime")
      break
    }
    emitEvent("measurement.started", {
      idx: i,
      input: settings.inputs[i]
    })
    let [measurement, error] = task.Run(settings.inputs[i])
    // TODO(bassosimone): in both of the following events I'd add `idx`
    if (error) {
      emitEvent("failure.measurement", {
        failure: error.AsString()
      })
    } else {
      emitEvent("measurement", {
        json_str: measurement.asJSON()
      })
    }
    if (!settings.options.no_file_report) {
      let error = WriteReportToFile(measurement)
      if (error) {
        emitWarning("cannot write report to file")
        finish(error)
        return
      }
    }
    if (!settings.options.no_collector) {
      let error = SubmitMeasurementToCollector(measurement)
      if (error) {
        emitEvent("failure.measurement_submission", {
          idx: i,
          json_str: measurement.asJSON()
          failure: error.AsString()
        })
      } else {
        emitEvent("status.measurement_uploaded", {
          idx: i
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
  emitProgress(0.95, "ending the test")

  if (!settings.options.no_file_report) {
    error = closeFileReport()
    if (error) {
      emitWarning("cannot close report file")
      finish(error)
      return
    }
  }
  if (!settings.options.no_collector) {
    let error = CloseReport()
    if (error) {
      emitWarning("cannot close remote report")
    }
  }

  finish()
}
```

## Settings

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
  "log_level": "INFO"
  "name": "WebConnectivity",
  "options": {
  },
  "output_filepath": "results.txt",
}
```

The following keys are available:

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

- `"log_level"`: how much information you want to see written in the log
  file and emitted by log-related events.

## Log levels

The available log levels are:

- `"ERR"`: an error message

- `"WARNING"`: a warning message

- `"INFO"`: an informational message

- `"DEBUG"`: a debugging message

- `"DEBUG2"`: a really specific debugging message

When you specify a log level in the settings, only message with a log level
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

Where `"value"` is a JSON object with an event-specific structure, and `"key"`
is a string. Below we describe all the possible event keys, along with the
"value" JSON structure. Unless otherwise specified, an event key can be emitted
an arbitrary number of times during the lifecycle of a task. Unless otherwise
specified, all the keys introduced below where added in MK v0.9.0.

- `"failure.measurement"`: There was a failure running the measurement. The
complete JSON returned by this event is like:

```JSON
{
  "key": "failure.measurement",
  "value": {
    "failure": "<failure_string>"
  }
}
```

Where all the possible values of `<failure_string>` are described below.

- `"failure.measurement_submission"`: There was a failure in submitting the
measurement result to the configured collector (if any). The complete JSON
returned by this event is like:

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

- `"failure.startup"`: There was a failure starting the test, most likely
because you passed in incorrect options. The complete JSON returned by this
event is like:

```JSON
{
  "key": "failure.startup",
  "value": {
    "failure": "<failure_string>"
  }
}
```

Where `<failure_string>` is the error that occurred.

- `"log"`: A log line was emitted. The complete JSON is like:

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

- `"measurement"`: The result of a measurement. The complete JSON is like:

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

- `"status.end"`: This event is emitted just once at the end of the test. The
complete JSON is like:

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
the test (or the empty string, if no error occurred).

- `"status.geoip_lookup"`: This event is emitted only once at the beginning
of the task, and provides information about the user's IP address, country and
autonomous system. In detail, the JSON is like:

```JSON
{
  "key": "status.geoip_lookup",
  "value": {
    "probe_ip": "<ip_address>,
    "probe_asn": "<asn>",
    "probe_cc": "<cc>"
  }
}
```

Where `<ip_address>` is the user's IP address, `asn` is the autonomous
system number, `cc` is the country code.

- `"status.progress"`: This is emitted during the task lifecycle to inform
you about the task progress towards completion. In detail, the JSON is like:

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

- `"status.queued"`: Indicates that the task has been accepted. In case there
are already running tasks, as mentioned above, the task will wait in a queue
until previously running tasks have terminated. The JSON is like:

```JSON
{
  "key": "status.queued",
  "value": {
  }
}
```

Where `value` is empty.

- `"status.measurement_started"`: Indicates that a measurement inside a task
has started. The JSON is like:

```JSON
{
  "key": "status.measurement_started",
  "value": {
    "idx": 0,
    "input": "<input>"
  }
}
```

Where `idx` is the index of the current input and `input` is the current
input. For tests that take no input, this event MAY be emitted with
`idx` equal to `0` and `input` equal to the empty string.

- `"status.measurement_uploaded"`: The specific measurement has been
uploaded successfully. The JSON is like:

```JSON
{
  "key": "status.measurement_uploaded",
  "value": {
    "idx": 0,
  }
}
```

Where `idx` is the index of the measurement input.

- `"status.measurement_done"`: Measurement Kit has finished processing
the specified input. The JSON is like:

```JSON
{
  "key": "status.measurement_uploaded",
  "value": {
    "idx": 0,
  }
}
```

Where `idx` is the index of the measurement input.

- `"status.report_created"`: Measurement Kit has created a report for the
current task, and tells you the report-ID. The report-ID is the identifier of
the measurement result(s), which will be later submitted. The JSON is like:

```JSON
{
  "key": "status.report_created",
  "value": {
    "report_id": "string",
  }
}
```

Where `report_id` is the report identifier.

- `"status.started"`: The task has started, and the JSON is like:

```JSON
{
  "key": "status.started",
  "value": {
  }
}
```

Where `value` is empty.

- `"status.update.performance"`: This is an event emitted by tests that
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

- `"status.update.websites"`: This is an event emitted by tests that measure
the reachability of websites. The JSON is like:

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

- `"task_terminated"`: This event is emitted only once when the task has
terminated running. The related JSON is like:

```JSON
{
  "key": "status.terminated",
  "value": {
  }
}
```

Where `value` is empty.

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
