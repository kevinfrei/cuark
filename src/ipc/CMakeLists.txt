# This is just a couple headers, but I like the componentization of
# having a separate CMake target for it.
add_library(ipc_lib)
target_sources(
  ipc_lib
  PUBLIC
  FILE_SET
  CXX_MODULES
  FILES
    CommonTypes.cppm CrowSupport.cppm
)
target_link_libraries(ipc_lib PUBLIC ${CROW_LIB})

add_subdirectory(test)
