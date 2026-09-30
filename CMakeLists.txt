cmake_minimum_required(VERSION 3.16)
project(datastructure-lab LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(${CMAKE_SOURCE_DIR}/container)

enable_testing()

# 主程序
add_executable(main src/main.cpp)
target_compile_options(main PRIVATE -Wall -Wextra)

# 测试
add_executable(Vector_test tests/ADT_test/Vector_test.cpp)
target_compile_options(Vector_test PRIVATE -Wall -Wextra)
add_executable(List_test tests/ADT_test/List_test.cpp)
target_compile_options(List_test PRIVATE -Wall -Wextra)

# 数据文件自动复制到 build/bin
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tests/ADT_test/Vector_data.txt
               ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Vector_data.txt COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tests/ADT_test/List_data.txt
               ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/List_data.txt COPYONLY)

# 在 build/bin 下运行
add_test(NAME Vector_test COMMAND Vector_test
         WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME List_test COMMAND List_test
         WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})