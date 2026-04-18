add_executable(
  core_testing
  config_test.cpp
  files_test.cpp
  handlers_test.cpp
  images_test.cpp
  tools_test.cpp
)
target_link_libraries(
  core_testing
  PUBLIC
    ${GTEST_LIB} core_lib ${BOOST_LIB} ${CROW_LIB}
)

gtest_discover_tests(core_testing)
