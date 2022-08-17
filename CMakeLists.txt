cmake_minimum_required(VERSION 3.23)
project(clox)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

find_package(fmt)
find_package(magic_enum)

add_executable(
        clox
        main.cc
        chunk.hh
        opcode.hh
        value.hh
        vm.hh
        vm.cc
        scanner.cc
        scanner.hh
        compile.cc
        compile.hh
        expected.hh)

target_link_libraries(clox
        PRIVATE
        fmt::fmt
        magic_enum::magic_enum
        )
