add_executable(
  test_clox
  test_value.cc
  test_clox.cc
)

target_link_libraries(
  test_clox
  PRIVATE
  Catch2::Catch2
  clox
)

catch_discover_tests(test_clox)
