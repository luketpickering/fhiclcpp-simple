###### linedoc set up
include(ExternalProject)

ExternalProject_Add(linedoc
  PREFIX "${PROJECT_BINARY_DIR}/linedoc"
  GIT_REPOSITORY https://github.com/luketpickering/linedoc.git
  GIT_TAG stable
  UPDATE_DISCONNECTED 1
  CMAKE_ARGS
  -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
  -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
  -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
  -DCMAKE_BUILD_TYPE=${DCMAKE_BUILD_TYPE})
