add_executable(fhiclcpp_tests tests.cxx)
install(TARGETS fhiclcpp_tests DESTINATION test)

add_executable(fhicl-dump fhicl-dump.cxx)
install(TARGETS fhicl-dump DESTINATION bin)


add_test(NAME fhiclcpp_tests COMMAND string_parsers_tests)
