add_subdirectory(string_parsers)
add_subdirectory(types)

add_executable(fhicl-dump fhicl-dump.cxx)
target_link_libraries(fhicl-dump fhiclcpp_includes linedoc::includes)
install(TARGETS fhicl-dump DESTINATION bin)

if(DOTEST)
  add_executable(fhiclcpp_tests tests.cxx)
  target_link_libraries(fhiclcpp_tests fhiclcpp_includes linedoc::includes)
  install(TARGETS fhiclcpp_tests DESTINATION test)

  add_test(NAME fhiclcpp_tests COMMAND fhiclcpp_tests)
endif()

install(FILES
  fhicl_doc.hxx
  exception.hxx
  ParameterSet.h
  fwd.h
  recursive_build_fhicl.hxx
DESTINATION include/fhiclcpp)
