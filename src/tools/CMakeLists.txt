# A few little tools for everyone to use

add_library(
  tools_lib
  STATIC
    base64.cpp file_tools.cpp text_tools.cpp os/${TARGET_OS}.cpp
)
target_include_directories(tools_lib PUBLIC include)
# target_include_directories(tools_lib PRIVATE impl)

# Link the dependencies with our binary
target_link_libraries(tools_lib PUBLIC ipc_lib ${CROW_LIB})

add_subdirectory(test)
