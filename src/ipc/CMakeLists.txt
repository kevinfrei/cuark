# This is just a couple headers, but I like the componentization of
# having a separate CMake target for it.
add_library(ipc_lib INTERFACE CommonTypes.hpp)
target_include_directories(ipc_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ipc_lib INTERFACE ${CROW_LIB})

add_subdirectory(test)
