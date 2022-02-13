cmake_minimum_required (VERSION 3.14 FATAL_ERROR)

project(fhiclcpp)
set(fhiclcpp_VERSION 0.4.0)

if(NOT DEFINED CMAKE_INSTALL_PREFIX OR 
    "${CMAKE_INSTALL_PREFIX}x" STREQUAL "x" OR 
    "${CMAKE_INSTALL_PREFIX}x" STREQUAL "/usr/localx")
  set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/${CMAKE_SYSTEM_NAME}")
endif()

if(NOT DEFINED CMAKE_BUILD_TYPE OR 
    "${CMAKE_BUILD_TYPE}x" STREQUAL "x")
  set(CMAKE_BUILD_TYPE Release)
endif()

if(NOT DEFINED DOTEST)
  SET(DOTEST FALSE)
endif()

if(NOT DEFINED SETUPFILENAME)
  SET(SETUPFILENAME "setup.sh")
endif()

if(DOTEST)
  enable_testing()
endif()

if(NOT DEFINED CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD LESS 14)
  set(CMAKE_CXX_STANDARD 14)
endif()

include (${CMAKE_SOURCE_DIR}/cmake/fhiclcppConfig.cmake)

include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(string_parsers)
add_subdirectory(types)
add_subdirectory(fhiclcpp)

configure_file(cmake/setup.sh.in
"${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${SETUPFILENAME}" @ONLY)
install(FILES
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${SETUPFILENAME}" DESTINATION
  ${CMAKE_INSTALL_PREFIX})

install(FILES
  ${CMAKE_SOURCE_DIR}/cmake/fhiclcppConfig.cmake DESTINATION
  ${CMAKE_INSTALL_PREFIX}/cmake)

FILE(WRITE ${CMAKE_INSTALL_PREFIX}/cmake/fhiclcppVersion.cmake 
  "SET(fhiclcpp_VERSION ${fhiclcpp_VERSION})\n" )

if(DOTEST)
  file(GLOB EXAMPLE_FCL fcl/*.fcl)
  install(FILES ${EXAMPLE_FCL} DESTINATION ${CMAKE_INSTALL_PREFIX}/fcl)
endif()
