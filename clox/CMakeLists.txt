add_library(
  clox
  compile.cc
  compile.hh
  expected.hh
  opcode.hh
  scanner.cc
  scanner.hh
  value.hh
  vm.cc
  vm.hh
  visitor.hh
  nil.hh
)

target_include_directories(
  clox
  PUBLIC
  ${CMAKE_SOURCE_DIR}
)

target_link_libraries(
  clox
  PUBLIC
  fmt::fmt
  magic_enum::magic_enum
)
