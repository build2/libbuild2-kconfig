# kconfig-hello

The Kconfig version of the "Hello World" example program. Its aim is to
demonstrate the key features of Kconfig-based configuration management and
show how to integrate it into `build2` projects. For more information on
Kconfig and its support in `build2` see [The `build2` Kconfig Module][doc].

Note that this project defaults to a configuration with default values from
the `Kconfig` file. To create a custom configuration you will need to request
one of the interactive configuration methods explicitly. For example:

```
$ b configure config.kconfig=ask
```

[doc]: https://build2.org/libbuild2-kconfig/doc/build2-kconfig-manual.xhtml
