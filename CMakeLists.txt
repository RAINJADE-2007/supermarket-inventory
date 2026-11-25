cmake_minimum_required(VERSION 3.15)
project(supermarket_inventory LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
foreach(config Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER "${config}" upper_config)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${upper_config} "${CMAKE_BINARY_DIR}/bin")
endforeach()

add_library(inventory_core
    src/product.cpp
    src/stock.cpp
    src/query.cpp
    src/data_io.cpp
)
target_include_directories(inventory_core PUBLIC include)

add_executable(supermarket src/main.cpp)
target_link_libraries(supermarket PRIVATE inventory_core)

add_executable(inventory_web web/native/inventory_web.cpp)
target_link_libraries(inventory_web PRIVATE inventory_core)

foreach(target inventory_core supermarket inventory_web)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /utf-8)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endforeach()
