set(
  CMAKE_CXX_CPPCHECK
  cppcheck
)

set(
  CMAKE_CXX_CLANG_TIDY
  clang-tidy
  --config-file=${CMAKE_CURRENT_SOURCE_DIR}/.clang-tidy
  --header-filter=.*
)
