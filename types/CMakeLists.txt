add_executable(types_tests tests.cxx)
install(TARGETS types_tests DESTINATION test)

add_test(NAME types_tests COMMAND types_tests)
