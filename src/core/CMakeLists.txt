# The core library of the app, implementing the main logic.
# This is basically everything except the main entry point and the tests.

add_library(core_lib)
target_sources(
  core_lib
  PUBLIC
  FILE_SET
  CXX_MODULES
  FILES
    config.cppm images.cppm quitting.cppm setup.cppm web.cppm
)
target_sources(
  core_lib
  PUBLIC
    files.cpp handlers.cpp transpile.cpp tunes.cpp websocket.cpp
)
target_include_directories(core_lib PUBLIC include)
# This would be for core-specific header files:
# target_include_directories(core_lib PRIVATE impl)

if(APPLE)
  set_source_files_properties(
    files.cpp
    PROPERTIES
    COMPILE_FLAGS
    "-Wno-deprecated-declarations"
  )
endif()

# Link the dependencies with our binary
target_link_libraries(
  core_lib
  PUBLIC
    ipc_lib
    tools_lib
    ${BOOST_LIB}
    ${CROW_LIB}
    ${PFD_LIB}
    ${PLATFORMFOLDERS_LIB}
    ${REPROC_LIB}
    ${WEBVIEW_LIB}
)

add_subdirectory(test)
