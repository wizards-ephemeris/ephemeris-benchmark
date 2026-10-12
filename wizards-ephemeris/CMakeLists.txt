cmake_minimum_required(VERSION 3.16)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

project(wizards-ephemeris LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(eph
  src/Time.cpp
  src/Frames.cpp
  src/Moon.cpp
  src/VSOP87.cpp
  src/Spk.cpp
  src/Minor.cpp
  src/Houses.cpp
  src/Ayanamsa.cpp
  src/Ephemeris.cpp
  src/generated/vsop87_data.cpp
  src/generated/elp_data.cpp
  src/generated/nutation_data.cpp
  src/generated/minor_cheb.cpp
)
target_include_directories(eph
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_compile_options(eph PRIVATE -O2 -Wall -Wextra -Wno-unused-parameter)
set_source_files_properties(src/generated/minor_cheb.cpp PROPERTIES COMPILE_OPTIONS "-O0")
target_link_libraries(eph PUBLIC m)
if(COMMAND apply_p2996_target)
  apply_p2996_target(eph)
endif()
