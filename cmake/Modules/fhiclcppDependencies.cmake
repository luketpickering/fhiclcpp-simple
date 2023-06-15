get_filename_component(nusystematicsDependencies_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

include(${nusystematicsDependencies_CMAKE_DIR}/CPM.cmake)

CPMFindPackage(
    NAME linedoc
    GIT_TAG stable
    GITHUB_REPOSITORY luketpickering/linedoc
)