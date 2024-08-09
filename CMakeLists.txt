cmake_minimum_required(VERSION 3.26)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

project(Kirby C)

include_directories(
    "${CMAKE_SOURCE_DIR}/source"
)

add_executable(Kirby
    "${CMAKE_SOURCE_DIR}/source/main.c"
)

target_sources(Kirby PRIVATE "${CMAKE_SOURCE_DIR}/zasoby/TestoweOkienka.rc")

foreach(LANG  C CXX RC)
    set(CMAKE_${LANG}_STANDARD_INCLUDE_DIRECTORIES ${CUSTOM_INCLUDE_DIRECTORIES})
endforeach()
