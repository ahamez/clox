add_library(clox)

target_sources(
  clox
  PRIVATE
  code.cc
  compile.cc
  disassemble.cc
  memory.cc
  obj_string.cc
  scanner.cc
  stack.cc
  value.cc
  vm.cc
)

target_sources(
  clox
  PUBLIC FILE_SET HEADERS
  BASE_DIRS
  ${CMAKE_SOURCE_DIR}
  FILES
  chunk.hh
  code.hh
  index.hh
  disassemble.hh
  expected.hh
  memory.hh
  nil.hh
  obj_string.hh
  opcode.hh
  scanner.hh
  stack.hh
  value.hh
  visitor.hh
  vm.hh
)

if (ipo_supported)
  set_property(TARGET clox PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif ()

target_link_libraries(
  clox
  PUBLIC
  Boost::boost
  fmt::fmt
  magic_enum::magic_enum
  type_safe::type_safe
)

install(TARGETS clox FILE_SET HEADERS)
