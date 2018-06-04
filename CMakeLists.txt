cmake_minimum_required (VERSION 2.6 FATAL_ERROR)

project(fhiclcpp_simple)

set (fhiclcpp_simple_VERSION_MAJOR 0)
set (fhiclcpp_simple_VERSION_MINOR 2)
set (fhiclcpp_simple_VERSION_REVISION 0)

set (fhiclcpp_simple_VERSION_STRING "v${fhiclcpp_simple_VERSION_MAJOR}r${fhiclcpp_simple_VERSION_MINOR}p${fhiclcpp_simple_VERSION_REVISION}")

if(NOT DEFINED DOTEST)
  SET(DOTEST FALSE)
endif()

if(NOT DEFINED NO_SETUP)
  SET(NO_SETUP FALSE)
endif()

if(DOTEST)
  enable_testing()
endif()

#Set this to TRUE to enable build debugging messages
set(BUILD_DEBUG_MSGS TRUE)

include(${CMAKE_SOURCE_DIR}/cmake/cmessage.cmake)

#Changes default install path to be a subdirectory of the build dir.
#Can set build dir at configure time with -DCMAKE_INSTALL_PREFIX=/install/path
if(CMAKE_INSTALL_PREFIX STREQUAL "" OR CMAKE_INSTALL_PREFIX STREQUAL
  "/usr/local")
  set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/${CMAKE_SYSTEM_NAME}")
elseif(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/${CMAKE_SYSTEM_NAME}")
endif()

cmessage(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")

if(CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE DEBUG)
elseif(NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE DEBUG)
endif()

################################################################################
#                            Check Dependencies
################################################################################

SET(EXTRA_CXX_FLAGS)
SET(INCLUDE_DIRECTORIES)
SET(EXTRA_LINK_DIRS)
SET(EXTRA_LIBS)

################################## COMPILER ####################################
include(${CMAKE_SOURCE_DIR}/cmake/c++CompilerSetup.cmake)

include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(string_parsers)
add_subdirectory(types)
add_subdirectory(fhiclcpp)

if(NOT NO_SETUP)
  configure_file(cmake/setup.sh.in
    "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/setup.sh" @ONLY)
  install(FILES
    "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/setup.sh" DESTINATION
    ${CMAKE_INSTALL_PREFIX})
endif()

if(DOTEST)
  file(GLOB EXAMPLE_FCL fcl/*.fcl)
  install(PROGRAMS ${EXAMPLE_FCL} DESTINATION ${CMAKE_INSTALL_PREFIX}/fcl)
endif()
