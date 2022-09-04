add_library(
  clox
  OBJECT
  chunk.hh
  code.cc
  code_context.cc
  code_context.hh
  code_fwd.hh
  compile.cc
  compile.hh
  disassemble.cc
  disassemble.hh
  expected.hh
  memory.cc
  memory.hh
  nil.hh
  obj_string.cc
  obj_string.hh
  opcode.hh
  scanner.cc
  scanner.hh
  stack.cc
  stack.hh
  token.hh
  value.cc
  value.hh
  visitor.hh
  vm.cc
  vm.hh
)

if (ipo_supported)
  set_property(TARGET clox PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif ()

target_include_directories(
  clox
  PUBLIC
  ${CMAKE_SOURCE_DIR}
)

target_include_directories(
  clox
  SYSTEM
  PUBLIC
  ${Boost_INCLUDE_DIRS}
)

target_link_libraries(
  clox
  PUBLIC
  fmt::fmt
  magic_enum::magic_enum
  type_safe::type_safe
)
