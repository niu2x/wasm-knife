cmake_minimum_required(VERSION 3.11.0 FATAL_ERROR)
project(wasm-knife VERSION 0.1.0 LANGUAGES CXX C)


option(BUILD_TOOLS "" OFF)
option(BUILD_TESTS "" OFF)
option(BUILD_STATIC_LIB "" ON)
add_subdirectory(binaryen)


add_executable(trim-func "")
target_sources(trim-func PRIVATE "cpp/trim-func.cpp")
target_link_libraries(trim-func PRIVATE binaryen)
	
if(EMSCRIPTEN)
	target_include_directories(trim-func PRIVATE ./binaryen/src/)
	target_link_options(trim-func PRIVATE "-sNODERAWFS=1")
	target_link_options(trim-func PRIVATE "-sALLOW_MEMORY_GROWTH=1")
	target_compile_definitions(trim-func PRIVATE WASM_KNIFE_EMSCRIPTEN=1)
endif()

