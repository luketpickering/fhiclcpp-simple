add_executable(fhicl-dump fhicl-dump.cxx)
target_link_libraries(fhicl-dump linedoc::Headers)
install(TARGETS fhicl-dump DESTINATION bin)

if(DOTEST)
  add_executable(fhiclcpp_tests tests.cxx)
  target_link_libraries(fhiclcpp_tests linedoc::Headers)
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
