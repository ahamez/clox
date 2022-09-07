set(
  CMAKE_CXX_INCLUDE_WHAT_YOU_USE
  include-what-you-use
  -Xiwyu --no_fwd_decls
  -Xiwyu --cxx17ns
  -Xiwyu --transitive_includes_only
)
