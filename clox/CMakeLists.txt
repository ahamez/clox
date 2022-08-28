add_library(
  clox
  OBJECT
  chunk.cc
  compile.cc
  compile.hh
  expected.hh
  memory.cc
  memory.hh
  nil.hh
  obj_string.hh
  opcode.hh
  scanner.cc
  scanner.hh
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
  ${Boost_INCLUDE_DIRS}
)

target_link_libraries(
  clox
  PUBLIC
  fmt::fmt
  magic_enum::magic_enum
)
