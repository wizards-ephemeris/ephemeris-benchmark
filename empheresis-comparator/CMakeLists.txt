cmake_minimum_required(VERSION 3.16)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

project(empheresis-comparator LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(EMSCRIPTEN)
  set(EMPH_DEFAULT OFF)
else()
  set(EMPH_DEFAULT ON)
endif()
option(EMPH_BUILD_COMPARATOR "Build the Swiss Ephemeris comparison and benchmarks" ${EMPH_DEFAULT})
if(NOT EMPH_BUILD_COMPARATOR)
  return()
endif()

if(NOT TARGET eph)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../wizards-ephemeris ${CMAKE_CURRENT_BINARY_DIR}/wizards-ephemeris)
endif()

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
include(FetchContent)
if(NOT TARGET doctest)
  FetchContent_Declare(doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG v2.4.11
    GIT_SHALLOW TRUE
  )
  FetchContent_MakeAvailable(doctest)
endif()

if(NOT TARGET swisseph)
  FetchContent_Declare(swisseph
    GIT_REPOSITORY https://github.com/aloistr/swisseph.git
    GIT_TAG v2.10.3final
    GIT_SHALLOW TRUE
  )
  FetchContent_GetProperties(swisseph)
  if(NOT swisseph_POPULATED)
    FetchContent_Populate(swisseph)
  endif()
  add_library(swisseph STATIC
    ${swisseph_SOURCE_DIR}/swecl.c
    ${swisseph_SOURCE_DIR}/swedate.c
    ${swisseph_SOURCE_DIR}/swehouse.c
    ${swisseph_SOURCE_DIR}/swejpl.c
    ${swisseph_SOURCE_DIR}/swemmoon.c
    ${swisseph_SOURCE_DIR}/swemplan.c
    ${swisseph_SOURCE_DIR}/sweph.c
    ${swisseph_SOURCE_DIR}/swephlib.c
    ${swisseph_SOURCE_DIR}/swehel.c
  )
  set_target_properties(swisseph PROPERTIES C_STANDARD 99 POSITION_INDEPENDENT_CODE ON)
  target_include_directories(swisseph PUBLIC ${swisseph_SOURCE_DIR})
  target_compile_options(swisseph PRIVATE -w -O2)
  target_link_libraries(swisseph PUBLIC m)
endif()

add_library(emph_adapters src/Catalog.cpp)
target_include_directories(emph_adapters
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(emph_adapters PUBLIC eph swisseph)
target_compile_definitions(emph_adapters PRIVATE EPH_SWISS_EPHE="${swisseph_SOURCE_DIR}/ephe")
target_compile_options(emph_adapters PRIVATE -O2 -Wall -Wextra -Wno-unused-parameter)
if(COMMAND apply_p2996_target)
  apply_p2996_target(emph_adapters)
endif()

add_executable(empheresis_tests
  tests/main.cpp
  tests/test_math.cpp
  tests/test_time.cpp
  tests/test_frames.cpp
  tests/test_vsop.cpp
  tests/test_moon.cpp
  tests/test_minor.cpp
  tests/test_nodes.cpp
  tests/test_houses.cpp
  tests/test_sidereal.cpp
  tests/test_ephemeris.cpp
)
target_include_directories(empheresis_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(empheresis_tests PRIVATE emph_adapters doctest::doctest)
target_compile_definitions(empheresis_tests PRIVATE EPH_SWISS_EPHE="${swisseph_SOURCE_DIR}/ephe")
target_compile_options(empheresis_tests PRIVATE -Wall -Wextra -Wno-unused-parameter)
if(COMMAND apply_p2996_target)
  apply_p2996_target(empheresis_tests)
endif()
if(COMMAND apply_p2996_rpath_origin)
  apply_p2996_rpath_origin(empheresis_tests)
endif()

add_executable(bench_position bench/bench_position.cpp)
add_executable(bench_houses bench/bench_houses.cpp)
foreach(bench bench_position bench_houses)
  target_include_directories(${bench} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
  target_link_libraries(${bench} PRIVATE emph_adapters)
  target_compile_options(${bench} PRIVATE -O2 -Wall -Wextra -Wno-unused-parameter)
  if(COMMAND apply_p2996_target)
    apply_p2996_target(${bench})
  endif()
  if(COMMAND apply_p2996_rpath_origin)
    apply_p2996_rpath_origin(${bench})
  endif()
endforeach()

enable_testing()
add_test(NAME empheresis_tests COMMAND empheresis_tests)
