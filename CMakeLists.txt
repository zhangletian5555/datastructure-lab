cmake_minimum_required(VERSION 3.16)
project(datastructure-lab LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(${CMAKE_SOURCE_DIR}/container)

enable_testing()

# ---- 数据目录 ----
set(DATA_SRC_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/data)
set(DATA_DST_DIR  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/data)

file(GLOB DATA_FILES CONFIGURE_DEPENDS ${DATA_SRC_DIR}/*.csv)
add_custom_target(copy_data ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DATA_DST_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DATA_FILES} ${DATA_DST_DIR}
    COMMENT "Copying data files to ${DATA_DST_DIR}")

# ---- 测试目标生成器 ----
function(add_dsh_test name src)
    add_executable(${name} ${src})
    target_compile_options(${name} PRIVATE -Wall -Wextra)
    add_dependencies(${name} copy_data)
    add_test(NAME ${name} COMMAND ${name}
             WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    # 超时兜底
    set_tests_properties(${name} PROPERTIES TIMEOUT 10)
endfunction()

# ---- 主程序 ----
add_executable(main src/main.cpp)
target_compile_options(main PRIVATE -Wall -Wextra)
add_dependencies(main copy_data)

# ---- 测试 ----
add_dsh_test(Vector_test tests/ADT_test/Vector_test.cpp)
add_dsh_test(List_test   tests/ADT_test/List_test.cpp)