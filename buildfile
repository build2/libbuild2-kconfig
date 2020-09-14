# Glue buildfile that "pulls" the tests and examples.
#
import tests = libbuild2-kconfig-tests/ kconfig-hello/
./: $tests
