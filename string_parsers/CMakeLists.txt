add_executable(string_parsers_tests tests.cxx)
install(TARGETS string_parsers_tests DESTINATION test)

add_test(NAME string_parsers_tests COMMAND string_parsers_tests)
