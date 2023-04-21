get_filename_component(nusystematicsDependencies_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

include(${nusystematicsDependencies_CMAKE_DIR}/CPM.cmake)
CPMFindPackage(
    NAME CMakeModules
    GIT_TAG stable
    GITHUB_REPOSITORY NuHepMC/CMakeModules
    DOWNLOAD_ONLY
)
include(${CMakeModules_SOURCE_DIR}/NuHepMCModules.cmake)

include(CMessage)

CPMFindPackage(
    NAME linedoc
    GIT_TAG stable
    GITHUB_REPOSITORY luketpickering/linedoc
)