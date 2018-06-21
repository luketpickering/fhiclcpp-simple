add_executable(fhicl-dump fhicl-dump.cxx)
add_dependencies(fhicl-dump linedoc)
install(TARGETS fhicl-dump DESTINATION bin)

if(DOTEST)
  add_executable(fhiclcpp_tests tests.cxx)
  add_dependencies(fhiclcpp_tests linedoc)
  install(TARGETS fhiclcpp_tests DESTINATION test)

  add_test(NAME fhiclcpp_tests COMMAND fhiclcpp_tests)
endif()

install(FILES
  fhicl_doc.hxx
  exception.hxx
  make_ParameterSet.h
  ParameterSet.h
  recursive_build_fhicl.hxx
DESTINATION include/fhiclcpp)
