# Glue buildfile that "pulls" the tests and examples.
#
import tests = libbuild2-kconfig-tests/ libbuild2-kconfig-examples/
./: $tests
