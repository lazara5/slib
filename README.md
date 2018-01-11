# slib
Application bootstrapping and general utility library. It handles:

  * Application configuration
  * Logging (on top of spdlog)
  * String and number manipulation (with Java-like API)
  * Basic data structures (with mixed Java/C++-like API)
  * Basic threading and concurrency
  * File manipulation

Currently Linux only. Requires a compiler with C++14 support.

Please note this is not currently intended as a general-purpose library. Only what was needed so far is implemented. There is no warranty for merchantability or fitness for a particular purpose.

## Dependencies

  * [fmtlib](https://github.com/fmtlib/fmt)
  * [spdlog](https://github.com/gabime/spdlog)
  * [tclap](http://tclap.sourceforge.net/)
  * [libbsd](https://libbsd.freedesktop.org/wiki/)
  * [std::experimental::optional](https://github.com/akrzemi1/Optional)
  * [rapidjson](http://rapidjson.org/) - optional, for JsonUtils only

