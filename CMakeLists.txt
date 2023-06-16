cmake_minimum_required (VERSION 3.14 FATAL_ERROR)

project(fhiclcppsimple VERSION 23.06 LANGUAGES CXX)

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

include (${PROJECT_SOURCE_DIR}/cmake/Modules/fhiclcppsimpleDependencies.cmake)

add_library(fhiclcppsimple_includes INTERFACE)
target_include_directories(fhiclcppsimple_includes INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(fhiclcppsimple_includes INTERFACE linedoc::includes)
set_target_properties(fhiclcppsimple_includes PROPERTIES EXPORT_NAME fhiclcppsimple::includes)

install(TARGETS fhiclcppsimple_includes EXPORT fhiclcppsimpleTargets)

add_library(fhiclcppsimple::includes ALIAS fhiclcppsimple_includes)

add_subdirectory(fhiclcppsimple)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/fhiclcppsimpleConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)
configure_package_config_file(
  "${CMAKE_CURRENT_LIST_DIR}/cmake/Templates/fhiclcppsimpleConfig.cmake.in"
  "${PROJECT_BINARY_DIR}/fhiclcppsimpleConfig.cmake"
  INSTALL_DESTINATION cmake
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

install(EXPORT fhiclcppsimpleTargets
        DESTINATION lib/cmake/fhiclcppsimple )

install(FILES "${PROJECT_BINARY_DIR}/fhiclcppsimpleConfigVersion.cmake"
              "${PROJECT_BINARY_DIR}/fhiclcppsimpleConfig.cmake"
        DESTINATION lib/cmake/fhiclcppsimple)

configure_file(${CMAKE_CURRENT_LIST_DIR}/cmake/Templates/setup.fhiclcppsimple.sh.in ${PROJECT_BINARY_DIR}/setup.fhiclcppsimple.sh @ONLY)
install(PROGRAMS ${PROJECT_BINARY_DIR}/setup.fhiclcppsimple.sh DESTINATION bin)

if(DOTEST)
  file(GLOB EXAMPLE_FCL fcl/*.fcl)
  install(FILES ${EXAMPLE_FCL} DESTINATION ${CMAKE_INSTALL_PREFIX}/fcl)
endif()
