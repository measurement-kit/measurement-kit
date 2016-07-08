# MeasurementKit 0.2.3 [2016-07-08]

- make sure podspec uses up-to-date version number
- bump version number to v0.2.3

# MeasurementKit 0.2.2 [2016-07-08]

- document v0.2.1 release (I forgot to do that)
- properly bump version number (again, I forgot to do that)
- add script to automate release tasks
- and document v0.2.2 release

# MeasurementKit 0.2.1 [2016-07-08]

- use jessie64 rather than trusty64 in Vagrant file to make
  sure we work on debian stable
- make sure autogen.sh works ok on jessie64
- fix a bug that prevented headers to be installed when $DESTDIR
  wasn't specified

# MeasurementKit 0.2.0 [2016-06-30]

- autogen.sh: with -n do not use the network
- fix net/connect timeout
- update documentation for v0.2.0
- bump version to v0.2.0

# MeasurementKit 0.2.0-beta.2 [2016-06-19]

- http/request_serializer: allow custom path
- refactoring and small fixes
- common/net_test: removed misleading identifier() function
- common/utils: export time related utils functions
- ooni: merge DSL and Impl tests, use functions to implement tests (#677)
- ooni: avoid collisions in test file names
- common/net_test: file_path => filepath

# MeasurementKit 0.2.0-beta.1 [2016-06-08]

- `ip_lookup()`: allow custom settings, reactor, and logger (required
   by Android app)

# MeasurementKit 0.2.0-beta [2016-06-07]

- add initial NDT client implementation
- add more continuous integration providers using docker
- add support for setting default CA bundle path
- updated http-parser to v2.7.0
- updated Catch to v1.5.0
- add precompiled dependencies for Android and iOS
- upgrade Android build scripts to NDK r11c
- switch to use libevent's upstream repository instead of our fork
- peg GeoIP to v2.7.9 rather than using the master branch
- update libressl to v2.3.6
- add more documentation for common
- start updating existing docs
- migrate the tree to use templates to mock API calls
- rationalize HTTP and mlabns APIs
- rationalize OONI tests DSL API (this would probably break apps)
- add client for OONI report protocol
- make Delegate more convenient
- make Error way more robust and convenient, e.g. allow to chain errors
- use define to increase robustness of error definitions
- allow to pass flags to logger to convey extended semantic, e.g. state update
- add more helpful MOCK macros
- unify NetTest and NetTestDsl object models (OONI not yet ready for that)
- rename Poller as Reactor (more precise name); Poller is now a React implement.
- Reactor: add `loop_with_initial_event_and_connectivity`
- rename Async as Runner (more precise name)
- make Settings more useful
- add nlohmann/json as external header
- rewrite report::Entry using nlohmann/json via private inheritance
- FIX: make sure `readn()` never consumes too much stack
- export `report` headers as public API now that it uses `report::Entry`
- make sure we don't ignore possible errors when writing report
- remove unused, incomplete Tor controller
- downgrade minimum iOS platform from 9.0 to 7.1
- use `std::promise<>` and `std::future<>` for increased robustness
- Poller: make sure we ignore `SIGPIPE`
- significantly increase code coverage (now greater than 93%)
- make sure we handle `EPIPE`
- FIX: make sure OONI code produces spec compliant reports
- refactor OoniTestImpl replacing InputGenerator with much simpler code
- add to OONI support for geolookup and for automatically submitting reports

# MeasurementKit 0.2.0-alpha [2016-05-12]

We are not ready to declare a stable release yet, because tons of things still need love. However, we've reached a point where we could tag an alpha release. Too many changes, hence listing them in the order in which I see them when reading the huge diff since the previous stable release:

- autogenerate toplevel headers
- rewrite async
- add more functional programming abstractions
- allow to bind context to errors
- rename the Maybe object as ErrorOr
- remove evbuffer wrapper
- introduce lexical_cast
- use templates rather than Libs object to make code testable
- all global objects are now `Var<>` rather than raw pointers
- more granular logger levels
- make logger thread safe
- the Poller is now known as Reactor (but Poller is still an implementation of Reactor)
- removed code to manage global dns resolver, now each test sets its own resolver
- now Settings maps strings to most scalar types
- now it is possible to cast from `Var<BaseClass>` to `Var<DerivedClass>`
- rewritten dns code from scratch to increase memory safety
- rewritten http code from scratch to increase memory safety
- prefix settings with module name, to avoid conflicts
- implement client for mlabns
- remove TransportInterface, make Transport abstract and pass around `Var<Transport>`
- rewrite from scratch the connect flow to increase memory safety
- implement SSL transport
- verify SSL certificates
- allow to connect many socket at once
- migrated OONI code to use JSON rather than YAML (thus replacing yaml-cpp dependency with nlohmann/json)
- as a consequence of previous change, remove boost from dependencies
- implement GeoIP using the old rather than the new maxmind library (not all free database available for new library)
- removed most dependency from tree and rely now on the `./build/dependency` script
- add ./autogen.sh script to automatically generate parts of the tree that it would be otherwise tedious to maintain
- rewritten from scratch and significantly improved travis.yml
- added more example files
- moved the mobile subtree below build along with the new `./build/dependency` script
- added basic listener
- more cleanups and bug fixes

To speed up cross compiling, this release would also be the first release to contain bits of compiled code (e.g. libressl) that could be downloaded during the cross compilation process.

# MeasurementKit 0.1.2 [2016-02-04]
- cleanups and bug fixes

# MeasurementKit 0.1.1 [2016-01-13]
- allow to specify report file path for OONI tests
- add missing documentation 
- add Vagrantfile
- mv ooni::NetTest ooni::NetTest to avoid confusion with common::NetTest
- misc fixes

# MeasurementKit 0.1.0 [2015-12-16]
- allow to clear DNS servers on Android and iOS
- doc: add examples and tutorials
- a few more small fixes here and there

# MeasurementKit 0.1.0-beta.4 [2015-12-12]
- http: avoid throwing runtime-error
- poller: add functions to manipulate default's evdns-base
- updated changelog file

# MeasurementKit 0.1.0-beta.3 [2015-12-09]
- readd documentation of private classes
- add support for mkdocs
- bugfix: make the Connection class non-copyable and non-movable
- fixes to simplify using cross-compiled android libraries
- make OONI tests set_verbose() accept a boolean value (true by default)
- improve iOS build system

# MeasurementKit 0.1.0-beta.2 [2015-12-07]
- install missing traceroute header
- shrink android build in size and create tarballs
- rename and simplify namespaces
    - rename measurement_kit to mk
    - remove namespace common
- update dependencies to latest version
- simplify transport usage
    - rename Transport to TransportInterface
    - reintroduce Transport as a proxy for Var<TransportInterface>
    - teach connect() to return Maybe<Transport>
- other minor changes

# MeasurementKit 0.1.0-beta.1 [2015-12-02]
- implemented code coverage using clang
- misc build system improvements
- make sure that dependencies headers do not appear in measurement-kit public headers
- fix build process to avoid installing dependencies headers for android
- ooni.hpp: more abstract api for running ooni tests (this changed the api of the ooni sublibrary)
- net.hpp: rename Buffer::foreach() to Buffer::for_each()
- net.hpp: modify Buffer::readline() to return Maybestd::string
- net.hpp: modify signature of Transport::on_data()
- net.hpp: modify signature of net::connect()
- common.hpp: add Async::global()
- common.hpp: implement the Maybe template
- moved many headers from include to src to make sure they are not part of the public api
- fix http-parser to throw common::Error subclasses on error
- net/stream.cpp: route common::Error exceptions received when reading/writing
- improve regress tests
- add more documentation

# MeasurementKit 0.1.0-alpha.1 [2015-11-21]
* Project name changed to MeasurementKit
* Most code has been rewritten using C++11
* Cleanup the API in:
    * common
    * dns
    * http
    * net
    * ooni
    * report
    * traceroute
* Implemented OONI tests:
    * dns injection
    * http invalid request line
    * tcp connect
* Many more changes with respect to LibNeubot (compared to that
  project, MeasurementKit is basically another project)

# LibNeubot 3.0.0 [2014-05-04]
* Rewrite NeubotPollable now that I know that, in C++, a structure can
  inherit from a class. As a result, the FooState structure is now gone
  and the API now looks like v1.0 again. That is, again the poller is
  passed to the Pollable constructor, not to init().
* Related to the previous change: clarified that the Pollable main
  purpose is to allow experiments in which Neubot uses LibNeubot.
* Related to the previous change: crank major (we're now at API 3.0).
* Take advantage of the API 3.0 cranking and install the headers
  at a different location (<neubot.h> becomes <libneubot/neubot.h>).
* Add (but do not use yet) joyent/http-parser as submodule.
* Update the OpenBSD port and fix the code accordingly (mainly cases
  in which mixed libevent 1.4 and 2.0 headers were being used).
* Related to the previous change: update README to better clarify
  which are the dependencies.
* Deprecate gen_cxx and neubot.hh, now that it's clear that the code
  will be written in C++ to ease the interaction with SWIG.
* Move testing code in `test/`.
* Significant improvements in gen_python, which now enforces the
  type safety (i.e., you can only pass to Pollable_foo() an instance
  of the Python Pollable class), and in general produces much
  less bloated and more pythonic Python code.
* Related to the previous change: half of the unit test is now gone
  because it was not typesafe (we passed integers to functions in
  hope that they were pointer to the right C/C++ objects).
* More minor changes, fixes and tweaks.

# LibNeubot 2.0.0 [2014-04-11]
* Add Android.mk to compile a static library for Android
* Remove the unused LibNeubot.java/gen_java thing
* Reimplement NeubotPollable using C++ and write C wrappers to
  provide a similar API (however, the arguments of the constructor
  and of attach() were changed, hence the API bump)
* Move Java code that was autogenerated with SWIG into the
  neubot/neubot_android repository
* Make sure gen_cxx generates code that works on Android: correct
  the order of the generated classes, replace the nonworking cast with
 the invocation of the ->pointer() method
* Unify gen_py/gen_python and libneubot.py/libneubot1.py: now
  gen_python generates a libneubot.py file that contains the old
  stuff plus the stuff that was in libneubot1.py, moreover, the
  gen_py script and libneubot1.py were removed
* Regen libneubot_python
* Make sure shared C functions (e.g., log.h and utils.h) are
  correctly exported to C++
* While bumping the API, unbreak the resolve() API
* While touching the Pollable, reimplement timeouts using
  the support provided by libevent
* Realize that the destruction of objects must flow from the
  derived classes to the base classes (as C++ does) not the
  other way round (as PyNeubot does)
* Add some defines needed to compile on Android
* Export the logs to the logcat to ease the debugging

# LibNeubot 1.0.1 [2014-02-18]
* Tweak gen_cxx to generate code that SWIG can handle
* Use SWIG to generate Java and Python wrappers

# LibNeubot 1.0.0 [2013-12-15]
* Return status_t in NeubotPoller_defer_{read,write}
* Pass opaque, not pollable, to NeubotPollable callbacks
* libneubot.json: Add and use hooks, slots and __destructor__
* Add easier-to-use libneubot1.py wrappers
* Add neubot.hh, wrappers for C++

# LibNeubot 0.1.0 [2013-12-12]
* Initial release
