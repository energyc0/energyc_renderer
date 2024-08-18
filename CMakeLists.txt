cmake_minimum_required(VERSION 3.28.3)

project("energyc_renderer"
    LANGUAGES C CXX)

add_executable("energyc_renderer"
    "sources/main.cpp"
)

add_compile_definitions(-DRENDERER_DIRECTORY="${CMAKE_CURRENT_LIST_DIR}")
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_compile_definitions(-DDEBUG)
    set(CMAKE_C_FLAGS "/fsanitize=address")
    set(CMAKE_CXX_FLAGS "/fsanitize=address")
endif()

set(CMAKE_CXX_STANDARD 20)
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

add_subdirectory("externals")
add_subdirectory("sources")

target_link_libraries("energyc_renderer" PRIVATE core::renderer)