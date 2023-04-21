cmake_minimum_required (VERSION 3.14 FATAL_ERROR)

project(fhiclcpp VERSION 0.5 LANGUAGES CXX)

if(NOT DEFINED CMAKE_INSTALL_PREFIX OR 
    "${CMAKE_INSTALL_PREFIX}x" STREQUAL "x" OR 
    "${CMAKE_INSTALL_PREFIX}x" STREQUAL "/usr/localx")
  set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/${CMAKE_SYSTEM_NAME}")
endif()

if(NOT DEFINED CMAKE_BUILD_TYPE OR 
    "${CMAKE_BUILD_TYPE}x" STREQUAL "x")
  set(CMAKE_BUILD_TYPE Release)
endif()

option(DOTEST "Whether to compile tests" OFF)

set(DOTEST_CONFIG FALSE)
if(DOTEST)
  enable_testing()
  set(DOTEST_CONFIG TRUE)
endif()

if(NOT DEFINED CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD LESS 14)
  set(CMAKE_CXX_STANDARD 14)
endif()

include (${CMAKE_SOURCE_DIR}/cmake/Modules/fhiclcppDependencies.cmake)

add_library(fhiclcpp_includes INTERFACE)
target_include_directories(fhiclcpp_includes INTERFACE 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>)
set_target_properties(fhiclcpp_includes PROPERTIES EXPORT_NAME fhiclcpp::includes)

install(TARGETS fhiclcpp_includes EXPORT fhiclcppTargets)

add_library(fhiclcpp::includes ALIAS fhiclcpp_includes)

add_subdirectory(string_parsers)
add_subdirectory(types)
add_subdirectory(fhiclcpp)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/fhiclcppConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)
configure_package_config_file(
  "${CMAKE_CURRENT_LIST_DIR}/cmake/Templates/fhiclcppConfig.cmake.in"
  "${PROJECT_BINARY_DIR}/fhiclcppConfig.cmake"
  INSTALL_DESTINATION cmake
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

install(EXPORT fhiclcppTargets
        DESTINATION lib/cmake/fhiclcpp )

install(FILES "${PROJECT_BINARY_DIR}/fhiclcppConfigVersion.cmake"
              "${PROJECT_BINARY_DIR}/fhiclcppConfig.cmake"
        DESTINATION lib/cmake/fhiclcpp)

configure_file(${CMAKE_CURRENT_LIST_DIR}/cmake/Templates/setup.fhiclcpp.sh.in ${PROJECT_BINARY_DIR}/setup.fhiclcpp.sh @ONLY)
install(PROGRAMS ${PROJECT_BINARY_DIR}/setup.fhiclcpp.sh DESTINATION bin)

if(DOTEST)
  file(GLOB EXAMPLE_FCL fcl/*.fcl)
  install(FILES ${EXAMPLE_FCL} DESTINATION ${CMAKE_INSTALL_PREFIX}/fcl)
endif()
