# Measurement Kit

> (deprecated) Network measurement engine

As of 2020-03-15, Measurement Kit is deprecated. This date has been chosen
arbitrarily such that we could write:

    Friends, OONItarians, developers, lend me your ears;
    I come to bury Measurement Kit, not to praise it.
    The bugs that software have live after them;
    The good is oft interred with their remote branches;

The rewrite of Measurement Kit in Go has been going on for quite some time now
as [ooni/probe-engine](https://github.com/ooni/probe-engine). As part of this
rewrite, we considered all the use cases addressed by Measurement Kit, as documented
by [issue #1913](https://github.com/measurement-kit/measurement-kit/issues/1913).

We will most likely never release v0.11.0 of Measurement Kit. We will keep
maintaining the 0.10.x version until 2021-03-15. In the following, we'll discuss
what options you have for replacing Measurement Kit in your use case. The
current README reflects the situation "here and now". We are still working to
provide a smooth upgrade path. Please, let us know if the upgrade path we have
designed is troubling you by opening an issue in this repository.

The content of the old README.md is still available as [OREADME.md](OREADME.md).

## Changes in the settings JSON

The ooni/probe-engine implementation exposes similar APIs to Measurement Kit
and specifically honours the [data format of Measurement Kit v0.10.11](
https://github.com/measurement-kit/measurement-kit/tree/v0.10.11/include/measurement_kit).
There should be no differences in the emitted events. There are however some
differences in the settings as discussed below.

You should now add the following the settings JSON:

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
phase (i.e. it will not throw, but rather you will see a very
short test with explanatory errors in the logs).

Also, the Go code does recognize all the settings recognized by
Measurement Kit, but some unfrequently used settings are not implemented
yet. If are by chance using settings that it does not implement, it
will also fail during the startup phase and tell you with log
messages. If a not implemented setting is causing you issues, let us
know by opening a bug in this repository.

## Android

In your `app/build.gradle` file, replace

```Groovy
  implementation "org.openobservatory.measurement_kit:android-libs:$version"
```

with

```Groovy
  implementation "org.ooni:oonimkall:$version"
```

The `io.ooni.mk.MKAsyncTask` class has been replaced by `oonimkall.Task`. The
following diff shows how you should be upgrading your code:

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
will explicitly throw `Exception` on failure, where the old code
would instead throw `RuntimeException`. The definition of settings has
only slightly changed from Measurement Kit, as discussed above.

We currently do not provide drop-in replacements for other functionality
implemented by measurement-kit/android-libs, e.g., accessing the GeoIP
lookup functionality, submitting reports. We will most likely do that since
we need that in OONI for Android. We should be able to provide drop-in
replacements, except perhaps some minor details.

## iOS

In your `Podfile` replace

```ruby
    pod 'mkall', :git => 'https://github.com/measurement-kit/mkall-ios.git',
                 :tag => '$version'
```

with

```ruby
    pod 'oonimkall', :git => 'https://github.com/ooni/probe-engine',
                     :tag => '$version'
```

The `MKAsyncTask` class has been replaced by `OonimkallTask`. The
following diff should how you should be upgrading your code:

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
that starts a measurement task now fails explicitly (e.g., if the settings
are not valid JSON). Second, the new code takes in input and emits in
output serialized JSONs rather than `NSDictionary *`. The definition of
settings has only slighly changed from MK, as described above.

We currently do not provide drop-in replacements for other functionality
implemented by measurement-kit/mkall-ios, e.g., accessing the GeoIP
lookup functionality, submitting reports. We will most likely do that since
we need that in OONI for iOS.

## Command Line

The [miniooni](https://github.com/ooni/probe-engine#building-miniooni) binary
is a drop-in interface for the `measurement_kit` binary you could build from
this repository. We automatically build `miniooni` for Windows/amd64, Linux/amd64,
and macOS/amd64 at every commit. The Linux build is static and does not depend
on any external shared library. You can find the builds by looking into the
[GitHub actions of probe-engine](https://github.com/ooni/probe-engine/actions)
and selecting for `cli-windows`, `cli-linux`, or `cli-darwin`. We are open to
automatically publish such binaries also at every release of probe-engine at
GitHub. Please, let us know if that is useful in your use case.

## Shared Library

The [libooniffi](https://github.com/ooni/probe-engine/tree/master/libooniffi)
package of ooni/probe-engine is a drop-in replacement for the [Measurement
Kit FFI API](include/measurement_kit). The following diff shows the changes you
need to apply in order to run the tests using probe-engine:

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

We are not going to reimplement other APIs provided by Measurement Kit
and most of them were private anyway. We are not going to automatically
publish libooniffi builds. You can generate your own builds with:

```bash
# macOS from macOS or Linux from Linux
go build -v -tags nomk -ldflags='-s -w' -buildmode c-shared -o libooniffi.so ./libooniffi
# Windows from Linux or macOS with mingw-w64 installed
export CGO_ENABLED=1 GOOS=windows GOARCH=amd64 CC=x86_64-w64-mingw32-gcc
go build -v -tags nomk -ldflags='-s -w' -buildmode c-shared -o libooniffi.dll ./libooniffi
```

Remember also to grab the corresponding header `./libooniffi/ooniffi.h`. Do
not use the autogenerated `./libooniffi.h` since it's incomplete.

Let us know if you want us to automatically publish `libooniffi` binaries
at every commit and/or at every release.
