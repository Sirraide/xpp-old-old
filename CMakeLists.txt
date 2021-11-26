cmake_minimum_required(VERSION 3.20)
project(xpp CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})

file(GLOB SRC src/*.cc src/*.h)

add_executable(xpp ${SRC})
target_compile_options(xpp PRIVATE -Wall -Wextra -Wundef -Wshadow)
target_compile_definitions(xpp PRIVATE V8_COMPRESS_POINTERS)
target_link_libraries(xpp PRIVATE v8)
target_link_libraries(xpp PRIVATE v8_libplatform)