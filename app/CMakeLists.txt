configure_file(
  "version.hh.in"
  "${PROJECT_BINARY_DIR}/app/version.hh"
)

add_executable(clox_interpreter clox.cc)
target_include_directories(clox_interpreter PUBLIC ${CMAKE_BINARY_DIR}/app)
target_link_libraries(clox_interpreter PRIVATE clox)
set_target_properties(clox_interpreter PROPERTIES OUTPUT_NAME "clox")

#if (ipo_supported)
#  set_property(TARGET clox_interpreter PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
#endif ()
