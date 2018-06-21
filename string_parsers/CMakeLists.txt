if(DOTEST)
  add_executable(string_parsers_tests tests.cxx)
  add_dependencies(string_parsers_tests linedoc)
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
  DESTINATION include/string_parsers)
