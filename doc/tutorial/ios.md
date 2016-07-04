# iOS tutorial

This tutorial explains how to integrate Measurement Kit into your
iOS Objective-C application. You need to understand how to generate
(or obtain) Measurement Kit frameworks. And how to integrate your
code with Measurement Kit code.

Throughout this tutorial we assume that you are running MacOSX
10.11 El Capitain (i.e. the version of OSX with which we tested
the code presented in this tutorial) and Xcode 9.2. Other versions
of both may work, especially if close to the ones we tested.

## How to generate (or obtain) the frameworks

We have published
[specification for Measurement Kit](
https://github.com/CocoaPods/Specs/tree/master/Specs/measurement_kit
) on [CocoaPods](https://cocoapods.org/).
We have not yet published a specification to directly get the
binaries, however. Hence, for now you will need the command line tools
installed on your build system to build the Pod.

To install Xcode command line tools on your system, run:

    xcode-select --install

Next, you need to install CocoaPods. To this end, run this command:

    sudo gem install cocoapods

As a third step, create a new Xcode project, following these steps:

- open Xcode and select "Create a new XCode project"
- select "Create a single view application"
- fill in the product name
- make sure that the selected language is Objective-C
- select the place where to save the application

Then, you shall use the terminal and `cd` to the directory containing
the `.xcodeproj` file. There, create a file called `Podfile` and write
inside it the following content:

```ruby
target '<YOUR-TARGET-NAME-HERE>' do
    pod 'measurement_kit'
end
```

(Instead, if you want to install measurement_kit directly from its
git repository, write the following inside the `Podfile`:

```ruby
target '<YOUR-TARGET-NAME-HERE>' do
    pod 'measurement_kit',
        :git => 'https://github.com/measurement-kit/measurement-kit.git',
        :branch => 'master'
end
```
)

Save and quit, then type:

    pod install --verbose

This will run for a long time. It will download Measurement Kit from
GitHub and cross-compile it for the iOS emulator and devices.

When this command terminates, new files will appear in your application
directory, including a `.xcworkspace` file that, from now on, you shall use
to open your application. Basically it is an Xcode project container,
which references both the application `.xcodeproj` and the compiled Pods.

As for the compiled Pods, they are inside the `Pods` directory. In
particular, if you're curious, under the following path

    Pods/measurement_kit/mobile/ios/Frameworks/

you will find all the frameworks needed to integrate Measurement Kit
with your application. (In general, you can use the `.xcworkspace` file
to manage you project, but you may also want instead to use directly
the frameworks, even though this is not the expected usage.)

## How to integrate Measurement Kit code

Open the `.xcworkspace` file using Xcode. For example clicking over
it, or with `open` if you are using the command line.

You will find that your workspace contains two projects. One named `Pods`
will contain the compiled frameworks. The other, named after your
project, unsurprisingly contains exactly your project.

Here, for simplicity, we are not going to change the view. Rather we
are going to run a Measurement Kit test just after the application has
started (i.e. in the `didFinishLaunchingWithOptions` method of the
`AppDelegate.m` file).

As a first step, we need to rename the file `AppDelegate.mm`. In fact,
Measurement Kit is written in C++. So, we need to tell Xcode that the app
delegate should be compiled using Objective-C++ (`.mm`) rather than
using Objective-C (`.m`). To rename the file, click on its name for a
long time until it becomes editable, then rename it.

We are going to implement OONI's tcp-connect test. So, you need to include
Measurement Kit OONI's functionality. To this end, modify the top of
your `AppDelegate.mm` file such that it looks like this:

```Objective-C
#import "AppDelegate.h"
#include <measurement_kit/ooni.hpp>

@interface AppDelegate ()

@end

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application
didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.
    return YES;
}

// more code below...
```

We have basically just added `measurement_kit/ooni.hpp` header to the
auto-generated file. Such header contains all MeasurementKit definitions
useful to run OONI tests. Then, try to build the project to ensure that
everything works.

To implement the test, we need to add more code to the currently-empty
implementation of `didFinishLaunchingWithOptions`. To do that, we need to
understand the ingredients required for running the test.

The tcp-connect tests is a test that attempts to connect to a list of
domain names of IP addresses. And measures, for each domain name or IP
address, whether the connection suceeded or not.

The first step would be to gather the device DNS resolver. For the sake of
simplicity, here we're going to use `8.8.8.8` as a resolver.

As a second step, you need to add to the application a file telling the
tcp-connect test which hosts to test. To this end, create a new file named
`inputs.txt` in the "Supporting Files" folder of your project in Xcode.

To create a new file, right click on "Supporting Files" and then select
"New File" from the drop-down menu. Then select "Empty", and name the
file "inputs.txt". Write some domain names inside this file, for example:

```
ooni.torproject.org
nexa.polito.it
measurement-kit.github.io
```

Now you need to obtain the path of this file in the final application. To
this end, append the following code snippet near the end of the above loop,
just below the TODO comment indicated above:

```Objective-C
        // Get path of input file:
        NSBundle *bundle = [NSBundle mainBundle];
        NSString *path = [bundle pathForResource:@"inputs" ofType:@"txt"];
        const char *input_path = [path UTF8String];
        NSLog(@"path of input file: %@", path);
```

We are now ready to invoke the tcp-connect test. Add the following code
just below the code you just added:

```Objective-C
    mk::ooni::TcpConnect()
        .increase_verbosity()
        .set_options("port", 80)
        .set_options("dns/nameserver", "8.8.8.8")
        .set_input_filepath(input_path)
        .run();
```

This will run the tcp-connect test in synchronous mode. That is, the current
thread will be blocked until the test completes.

Running a synchronous test, however, is not so good for a mobile application
because it blocks the UX for the whole duration of the test.

To instruct Measurement Kit to run the test asynchronously (i.e. to run
the test in the background and notify us when done) we can modify the code
above as follows:

```Objective-C
    mk::ooni::TcpConnect()
        .increase_verbosity()
        .set_options("port", 80)
        .set_options("dns/nameserver", "8.8.8.8")
        .set_input_filepath(input_path)
        .run([]() {
            // TODO This code runs in a background thread and is
            // called when the tcp-connect test is complete
        });
```

Basically, the C++11 lambda passed to `run()` is called from a background
thread when the test is complete. Now, to do something useful in that lamba,
let's simulate sending a message to the UX using `dispatch_async`.

Let's create the message before running the test. Let's edit the C++11
lambda capture list to retain a reference to `message`. And finally let's
use `dispatch_async` to dispatch the message when we are done.

```Objective-C
    NSString *message = @"message-0xdeadidea";
    mk::ooni::TcpConnect()
        .increase_verbosity()
        .set_options("port", 80)
        .set_options("dns/nameserver", "8.8.8.8")
        .set_input_filepath(input_path)
        .run([message]() {
            // Caution: code called from a background thread
            dispatch_async(dispatch_get_main_queue(), ^{
                NSLog(@"test complete: %@", message);
            });
        });

    NSLog(@"test in progress: %@", message);
```

The "test in progress" message is there so that it's clear from the logs
that the test is run asynchronously (i.e. that the `run()` returns immediately
and the C++11 lambda is only called when the test is complete).

To complete our example, let's also capture the test logs and insert them
into an array, which will be printed once the test is complete. This simulates
the case where you store separate logs for different tests.

```Objective-C
    NSString *message = @"message-0xdeadidea";
    NSMutableArray *logs = [[NSMutableArray alloc] init];
    NSLock *mtx = [[NSLock alloc] init];
    mk::ooni::TcpConnect()
        .increase_verbosity()
        .set_options("port", 80)
        .set_options("dns/nameserver", "8.8.8.8")
        .set_input_filepath(input_path)
        .on_log([logs, mtx](const char *s) {
            // Caution: code called from a background thread
            // Caution: `s` points to a static buffer, so I must copy it
            [mtx lock];
            [logs addObject:[NSString stringWithUTF8String:s]];
            [mtx unlock];
        })
        .run([message, logs, mtx]() {
            // Caution: code called from a background thread
            dispatch_async(dispatch_get_main_queue(), ^{
                NSLog(@"test complete: %@", message);
                [mtx lock];
                for (NSString *line in logs) {
                    NSLog(@"> %@", line);
                }
                [mtx unlock];
            });
        });

    NSLog(@"test in progress: %@", message);
```

I've also added a lock to ensure that the logs object is always accessed
safely. In this example, that is way overkill. But I wanted to show you the
most general case (i.e. the one in which the view is allowed to access the
`logs` object while tests are running). Also, the reason why I copy `s`
is that actually `s` is just a pointer in a static buffer, hence it must
be copied. Storing only the pointer is not very useful, since the buffer is
overwritten every time a new log line is generated.

The final example code is the following:

```Objective-C
#import "AppDelegate.h"
#include <measurement_kit/ooni.hpp>

@interface AppDelegate ()

@end

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application
didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {

    // Get path of input file:
    NSBundle *bundle = [NSBundle mainBundle];
    NSString *path = [bundle pathForResource:@"inputs" ofType:@"txt"];
    const char *input_path = [path UTF8String];
    NSLog(@"path of input file: %s", input_path);

    NSString *message = @"message-0xdeadidea";
    NSMutableArray *logs = [[NSMutableArray alloc] init];
    NSLock *mtx = [[NSLock alloc] init];
    mk::ooni::TcpConnect()
        .increase_verbosity()
        .set_options("port", 80)
        .set_options("dns/nameserver", "8.8.8.8")
        .set_input_filepath(input_path)
        .on_log([logs, mtx](const char *s) {
            // Caution: code called from a background thread
            // Caution: `s` points to a static buffer, so I must copy it
            [mtx lock];
            [logs addObject:[NSString stringWithUTF8String:s]];
            [mtx unlock];
        })
        .run([message, logs, mtx]() {
            // Caution: code called from a background thread
            dispatch_async(dispatch_get_main_queue(), ^{
                NSLog(@"test complete: %@", message);
                [mtx lock];
                for (NSString *line in logs) {
                    NSLog(@"> %@", line);
                }
                [mtx unlock];
            });
        });

    NSLog(@"test in progress: %@", message);
    return YES;
}

// more code here...
```

As a final remark, to test this sample application on a real device I
also needed to disable bitcode, since the frameworks compiled using Cocoa
Pods do not use bitcode.
