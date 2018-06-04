add_executable(fhicl-dump fhicl-dump.cxx)
install(TARGETS fhicl-dump DESTINATION bin)

if(DOTEST)
  add_executable(fhiclcpp_tests tests.cxx)
  install(TARGETS fhiclcpp_tests DESTINATION test)

  add_test(NAME fhiclcpp_tests COMMAND string_parsers_tests)
endif()

install(FILES
  fhicl_doc.hxx
  exception.hxx
  make_ParameterSet.h
  ParameterSet.h
  recursive_build_fhicl.hxx
DESTINATION include/fhiclcpp)
