cmake_minimum_required(VERSION 3.12)
project(vnc-girlskissing)

option(BUILD_STATIC "Build static binary" OFF)
if(BUILD_STATIC)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    set(BUILD_SHARED_LIBS OFF)
    set(CMAKE_EXE_LINKER_FLAGS "-static")
endif()

set(HEADERS src/vnc.h)
set(SOURCES src/vnc.c src/main.c)
add_compile_definitions(PROJECT_NAME="amyavi/vnc-girlskissing")
add_executable(${CMAKE_PROJECT_NAME} ${HEADERS} ${SOURCES})

target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall -Wextra -Werror -Wno-unused-parameter)
set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
set(CMAKE_C_FLAGS_RELEASE "-Ofast -s")
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -s")

find_package(LibVNCServer REQUIRED)
add_library(lodepng lib/lodepng/lodepng.c lib/lodepng/lodepng.h)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE lib)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE LibVNCServer::vncserver lodepng)
