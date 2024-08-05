cmake_minimum_required(VERSION 3.11.0 FATAL_ERROR)
project(wasm-knife VERSION 0.1.0 LANGUAGES CXX C)

option(BUILD_STATIC_LIB "" ON)
add_subdirectory(binaryen)

add_executable(trim-func "")
target_sources(trim-func PRIVATE "cpp/trim-func.cpp")
target_link_libraries(trim-func PRIVATE binaryen)