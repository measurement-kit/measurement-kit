# Measurement Kit

> (deprecated) Network measurement engine

As of 2020-03-15, Measurement Kit is deprecated. This date has been chosen
arbitrarily such that we could write:

    Friends, OONItarians, developers, lend me your ears;
    I come to bury Measurement Kit, not to praise it.
    The bugs that software have live after them;
    The good is oft interred with their remote branches;

And, of course, some good, old [piece of art](
https://it.wikipedia.org/wiki/Cesaricidio#/media/File:Vincenzo_Camuccini_-_La_morte_di_Cesare.jpg
) is also in order:

![cesaricidio](doc/cesaricidio.jpg
  "E se Cesare vuol fare anche il padre si abitui ai pugnali e a dormir male")

The rewrite of Measurement Kit in Go has been going on for quite some time now
as [ooni/probe-engine](https://github.com/ooni/probe-engine). As part of this
rewrite, we considered all the use cases addressed by Measurement Kit, as documented
by [issue #1913](https://github.com/measurement-kit/measurement-kit/issues/1913).

We will most likely never release v0.11.0 of Measurement Kit. We will keep
maintaining the 0.10.x version until 2021-03-14 (which is [another
interesting date](https://en.wikipedia.org/wiki/Pi_Day)). In the following, we'll discuss
what options you have for replacing Measurement Kit in your use case. The
current README reflects the situation "here and now". We are still working to
provide a smooth upgrade path. Please, let us know if the upgrade path we have
designed is troubling you by voting/contributing to the issues indicated below.

(The content of the old README.md is still available as [OREADME.md](OREADME.md).)

## Changes in the settings JSON

The ooni/probe-engine implementation exposes similar APIs to Measurement Kit
and specifically honours the [data format of Measurement Kit v0.10.11](
https://github.com/measurement-kit/measurement-kit/tree/v0.10.11/include/measurement_kit).
There should be no differences in the emitted events. There are however some
differences in the settings as discussed below.

You should now add the following three keys to the settings JSON:

```JSON
{
  "assets_dir": "",
  "state_dir": "",
  "temp_dir": ""
}
```

where `assets_dir` is the directory where to store assets, e.g.
GeoIP databases; `state_dir` is the directory where to store the
authentication information used with OONI orchestra; `temp_dir`
is the directory where to store temporary files. If these three
keys are not present, the test will fail during the startup
phase (i.e. it will run for a very short time and you will see
a bunch of `failure.startup` events emitted).

Also, the Go code does recognize all the settings recognized by
Measurement Kit, but we have only implemented the settings required
by OONI. All the other settings, when used, cause a failure during
the experiment startup phase. If a not implemented setting is causing
you issues, let us know by [voting in the corresponding bug tracking
issue](https://github.com/ooni/probe-engine/issues/494).

## Android

In your `app/build.gradle` file, replace

```Groovy
  implementation "org.openobservatory.measurement_kit:android-libs:$version"
```

with

```Groovy
  implementation "org.ooni:oonimkall:$version"
```

(The new package is called oonimkall because it's a OONI probe-engine based
implementation of the `mkall` API for iOS. In turn, `mkall` means that we
are bundling together all the MK APIs (i.e. the API for running experiments
and all the ancillary APIs). For historical reasons `android-libs` was
named before we defined the concept of `mkall` and was never renamed.)

The following differences apply between `android-libs` and `oonimkall`:

1. the import path is `oonimkall` and you can use it directly as a scope
for the classes, rather than doing `import oonimkall.Foo`;

2. the `MKAsyncTask` class is replaced by `oonimkall.Task` and the
`MKAsyncTask.start` factory is replaced by `oonimkall.Oonimkall.startTask`;

3. the `MKGeoIPLookupResults` and `MKGeoIPLookupTask` classes are
replaced by `oonimkall.GeoLookupResults` and `oonimkall.GeoLookupTask`;

4. the `MKOrchestraResults` and `MKOrchestraTask` classes cannot be
replaced, because [it seems we are moving away from the orchestra model
and, in going forward, we will only use orchestra internally inside of
probe-engine to authenticate probes when they fetch input](
https://github.com/ooni/probe-engine/issues/14#issuecomment-599547695);

5. the `MKReporterResults` and `MKReporterTask` classes are replaced
by `oonimkall.CollectorResults` and `oonimkall.CollectorTask`;

6. the `MKResourcesManager` class cannot be replaced, because the new
code manages resources differently, by downloading them when needed
into the `assets_dir` directory mentioned above;

7. the `MKVersion` class cannot be replaced because version pinning in
Go makes it much simpler to know which version of what software we compile;

8. `oonimkall` throws `Exception` in much more cases than the code in
`android-libs` that instead was using `RuntimeException` (using the latter
was actually an _anti-pattern_ and we are fixing it with the new code).

The following diff shows how to update code that runs an experiment, which
is probably the most common use case of `android-libs`:

```diff
--- MK.java	2020-04-10 11:54:53.973521643 +0200
+++ PE.java	2020-04-10 11:55:50.915205613 +0200
@@ -1,10 +1,8 @@
 package com.example.something;
 
-import io.ooni.mk.MKAsyncTask;
-
 public class Example {
-    public static void run(settings String) {
-        MKAsyncTask task = MKAsyncTask.start(settings);
+    public static void run(settings String) throws Exception {
+        oonimkall.Task task = oonimkall.Oonimkall.startTask(settings);
         for (!task.isDone()) {
             String event = task.waitForNextEvent();
             System.out.println(event);
```

The most striking difference is that the function to start a task
will explicitly throw `Exception` on failure. The old code
would instead throw `RuntimeException`, as mentioned above. The
required settings have slightly changed, as discussed above.

## iOS

In your `Podfile` replace

```ruby
    pod 'mkall', :git => 'https://github.com/measurement-kit/mkall-ios.git',
                 :tag => '$version'
```

with

```ruby
    pod 'oonimkall', :git => 'https://github.com/ooni/probe-engine',
                     :branch => 'mobile-staging'
```

The changes are similar to the ones described above for Android except
that the `oonimkall.` prefix is `Oonimkall` for iOS. The following diff
shows how you should be upgrading your `MKAsyncTask` code:

```diff
--- MK.m	2020-04-10 12:06:14.252573662 +0200
+++ PE.m	2020-04-10 12:08:18.520924676 +0200
@@ -1,11 +1,15 @@
 #import <Foundation/Foundation.h>
 
-#import <mkall/MKAsyncTask.h>
+#import <oonimkall/Oonimkall.h>
 
-void run(NSDictionary *settings) {
-    MKAsyncTask *task = [MKAsyncTask start:settings];
-    while (![task done]) {
-        NSDictionary *ev = [task waitForNextEvent];
+NSError *run(NSString *settings) {
+    NSError *error = nil;
+    OonimkallTask *task = OonimkallStartTask(settings, &error);
+    if (error != nil) {
+        return error;
+    }
+    while (![task isDone]) {
+        NSString *ev = [task waitForNextEvent];
         if (ev == nil) {
             continue;
         }
```

The most striking differences are the following. First, the function
that starts a task now fails explicitly (e.g., if the settings
are not valid JSON). Second, the new code takes in input and emits in
output serialized JSONs rather than `NSDictionary *`. You are welcome to
adapt [code from MKAsyncTask](
https://github.com/measurement-kit/mkall-ios/blob/v0.8.0/mkall/MKAsyncTask.mm)
to reimplement the previous behaviour. Also, remember that
some extra mandatory settings are required, as described above.

## Command Line

The [miniooni](https://github.com/ooni/probe-engine#building-miniooni) binary
mostly has the same CLI of the `measurement_kit` binary you could build from
this repository. The following list describes the main differences between
the two command line interfaces:

- `miniooni` by default _appends_ measurements to `report.jsonl` while
`measurement_kit` uses a file name including the experiment name and the
datetime when the experiment was started;

- `miniooni` uses the `-i, --input <input>` flag to uniformly provide input for
every experiment, while in `measurement_kit` different experiments use
different command line flags _after_ the experiment name (e.g., the `-u <URL>`
flag is used by MK's Web Connectivity);

- `miniooni` allows you to specify a proxy (e.g. Tor, Psiphon) with
`-P, --proxy <URL>` that will be used for interacting with OONI services,
but no such option exists in `measurement_kit`;

- `miniooni` does not yet implement `-s, --list` that lists all the
available experiments;

- `miniooni` does not implement `-l, --logfile <path>` but you can use
output redirection and `tee` to save logs anyway;

- `miniooni` does not implement `--ca-bundle-path <path>`, `--version`,
`--geoip-country-path <path>`, `--geoip-asn-path <path>`, because
these resources are now downloaded and managed automatically;

- `miniooni` does not implement `--no-resolver-lookup`;

- `miniooni` writes state at `$HOME/.miniooni`.

We automatically build `miniooni` for windows/amd64, linux/amd64,
and darwin/amd64 at every commit. The Linux build is static and does not depend
on any external shared library. You can find the builds by looking into the
[GitHub actions of probe-engine](https://github.com/ooni/probe-engine/actions)
and selecting for `cli-windows`, `cli-linux`, or `cli-darwin`. If you want us
to attach such binaries to every release, please [upvote
the related issue](https://github.com/ooni/probe-engine/issues/495).

## Shared Library

The [libooniffi](https://github.com/ooni/probe-engine/tree/master/libooniffi)
package of ooni/probe-engine is a drop-in replacement for the [Measurement
Kit FFI API](include/measurement_kit). The new API is defined by the
`ooniffi.h` header. It is ABI compatible with MK's API. The
only required change is to replace the `mk_` prefix with `ooniffi_`. This
diff shows the changes you typically need:

```diff
--- MK.c	2020-04-10 12:32:13.582783743 +0200
+++ PE.c	2020-04-10 12:32:36.633038633 +0200
@@ -1,19 +1,19 @@
 #include <stdio.h>
 
-#include <measurement_kit/ffi.h>
+#include <ooniffi.h>
 
 void run(const char *settings) {
-    mk_task_t *task = mk_task_start(settings);
+    ooniffi_task_t *task = ooniffi_task_start(settings);
     if (task == NULL) {
         return;
     }
-    while (!mk_task_is_done(task)) {
-        mk_event_t *event = mk_task_wait_for_next_event(task);
+    while (!ooniffi_task_is_done(task)) {
+        ooniffi_event_t *event = ooniffi_task_wait_for_next_event(task);
         if (event == NULL) {
             continue;
         }
-        printf("%s\n", mk_event_serialization(event));
-        mk_event_destroy(event);
+        printf("%s\n", ooniffi_event_serialization(event));
+        ooniffi_event_destroy(event);
     }
-    mk_task_destroy(task);
+    ooniffi_task_destroy(task);
 }
```

Of course, you also need to take into account the changes to
the settings documented above.

You can generate your own builds with:

```bash
# macOS from macOS or Linux from Linux
go build -v -tags nomk -ldflags='-s -w' -buildmode c-shared -o libooniffi.so ./libooniffi
rm libooniffi.h  # not needed
cp libooniffi/ooniffi.h .  # use this header

# Windows from Linux or macOS with mingw-w64 installed
export CGO_ENABLED=1 GOOS=windows GOARCH=amd64 CC=x86_64-w64-mingw32-gcc
go build -v -tags nomk -ldflags='-s -w' -buildmode c-shared -o libooniffi.dll ./libooniffi
rm libooniffi.h  # not needed
cp libooniffi/ooniffi.h .  # use this header
```

Let us know if you want us to automatically publish `libooniffi` dynamic
libraries [by upvoting the related issue](
https://github.com/ooni/probe-engine/issues/496).
