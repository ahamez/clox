add_library(clox)
add_library(clox::clox ALIAS clox)

target_sources(
  clox
  PRIVATE
  code.cc
  compile.cc
  disassemble.cc
  memory.cc
  obj_string.cc
  scanner.cc
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
  detail/compile.hh
  detail/index.hh
  detail/interpret.hh
  detail/stack.hh
  detail/visitor.hh
  disassemble.hh
  memory.hh
  nil.hh
  obj_string.hh
  opcode.hh
  scanner.hh
  value.hh
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
  Microsoft.GSL::GSL
  type_safe::type_safe
)

install(
  TARGETS clox
  EXPORT cloxTargets
  FILE_SET HEADERS
)
