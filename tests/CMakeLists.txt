find_package(${Qt_PACKAGE} COMPONENTS Test REQUIRED)
include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/Modules/Tests.cmake")

add_subdirectory(unit)
