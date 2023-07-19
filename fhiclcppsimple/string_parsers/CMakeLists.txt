if(DOTEST)
  add_executable(string_parsers_tests tests.cxx)
  target_link_libraries(string_parsers_tests fhiclcppsimple_includes linedoc::includes)
  install(TARGETS string_parsers_tests DESTINATION test)

  add_test(NAME string_parsers_tests COMMAND string_parsers_tests)
endif()

install(FILES
  exception.hxx
  from_string.hxx
  md5.hxx
  to_string.hxx
  traits.hxx
  utility.hxx
  DESTINATION include/fhiclcppsimple/string_parsers)
