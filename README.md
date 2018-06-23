# slib
Application bootstrapping and general utility library. It handles:

  * Application configuration
  * Logging (on top of spdlog)
  * String and number manipulation (with mixed Java/C++-like API)
  * Basic data structures (with mixed Java/C++-like API)
  * Runtime type system (class names, instanceof and casts only, limited to string, numeric and collection types)
  * Basic threading and concurrency
  * File manipulation
  * Expression evaluator

Currently Linux only. Requires a compiler with C++11 support.

Please note this is not currently intended as a general-purpose library. Only what was needed so far is implemented. There is no warranty for merchantability or fitness for a particular purpose.

## Dependencies

  * [fmtlib](https://github.com/fmtlib/fmt)
  * [spdlog](https://github.com/gabime/spdlog)
  * [tclap](http://tclap.sourceforge.net/)
  * [libbsd](https://libbsd.freedesktop.org/wiki/)
  * [rapidjson](http://rapidjson.org/) - optional, for JsonUtils only
  * [CppUTest](http://cpputest.github.io) - for tests only

