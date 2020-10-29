# libbuild2-kconfig

Linux kernel configuration system (Kconfig) build system module for `build2`.
For details on Kconfig and its support in `build2` see [The `build2` Kconfig
Module][doc]. For a complete example that demonstrates key features of
Kconfig-based configuration management see [`kconfig-hello`][example].

This module is part of the standard pre-installed `build2` modules and no
extra integration steps are required other than the `using` directive in
`bootstrap.build`. For development build instructions see
[`libbuild2-hello/README`][build] (note that this module requires
bootstrapping).

[doc]:     https://build2.org/libbuild2-kconfig/doc/build2-kconfig-manual.xhtml
[example]: https://github.com/build2/libbuild2-kconfig/tree/master/kconfig-hello
[build]:   https://github.com/build2/libbuild2-hello
