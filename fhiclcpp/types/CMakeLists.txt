if(DOTEST)
  add_executable(types_tests tests.cxx)
  target_link_libraries(types_tests fhiclcpp_includes linedoc::includes)
  install(TARGETS types_tests DESTINATION test)

  add_test(NAME types_tests COMMAND types_tests)
endif()

install(FILES
  Atom.hxx
  Base.hxx
  CompositeTypesSharedImpl.hxx
  exception.hxx
  ParameterSet.hxx
  Sequence.hxx
  traits.hxx
  utility.hxx
  DESTINATION include/fhiclcpp/types)
