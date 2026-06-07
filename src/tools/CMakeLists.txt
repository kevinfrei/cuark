# A few little tools for everyone to use

add_library(os_module_p)
target_sources(
  os_module_p
  PUBLIC
  FILE_SET
  CXX_MODULES
  FILES
    os/${TARGET_OS}.cppm
)

add_library(tools_lib)
target_sources(
  tools_lib
  PUBLIC
  FILE_SET
  CXX_MODULES
  FILES
    base64.cppm
    file_tools.cppm
    range_tools.cppm
    text_tools.cppm
    tuple_hash.cppm
)
# Link the dependencies with our binary
target_link_libraries(tools_lib PUBLIC ipc_lib os_module_p ${CROW_LIB})

add_subdirectory(test)
