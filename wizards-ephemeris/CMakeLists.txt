cmake_minimum_required(VERSION 3.16)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

project(wizards-ephemeris LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(EPH_MINOR_SERIES year CACHE STRING "Minor-body series: year or full")
set_property(CACHE EPH_MINOR_SERIES PROPERTY STRINGS year full)
if(EPH_MINOR_SERIES STREQUAL "year")
  set(EPH_MINOR_CHEB src/generated/minor_cheb.cpp)
elseif(EPH_MINOR_SERIES STREQUAL "full")
  set(EPH_MINOR_CHEB src/generated/minor_cheb_full.cpp)
else()
  message(FATAL_ERROR "EPH_MINOR_SERIES must be year or full, got '${EPH_MINOR_SERIES}'")
endif()

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
  ${EPH_MINOR_CHEB}
)
target_include_directories(eph
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_compile_options(eph PRIVATE -O2 -Wall -Wextra -Wno-unused-parameter)
if(EPH_MINOR_SERIES STREQUAL "full")
  set_source_files_properties(src/generated/minor_cheb_full.cpp PROPERTIES COMPILE_OPTIONS "-O0")
endif()
target_link_libraries(eph PUBLIC m)
if(COMMAND apply_p2996_target)
  apply_p2996_target(eph)
endif()

# Native golden values for the JavaScript tests. Top-level builds only,
# so the monorepo chart target does not grow an extra executable.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR AND NOT EMSCRIPTEN)
  add_executable(eph_fixture js/src/eph_fixture.cpp)
  target_link_libraries(eph_fixture PRIVATE eph)

  # Tests of the documented examples. Not part of the monorepo chart build.
  enable_testing()
  add_executable(eph_example_tests
    examples/cpp/test_examples.cpp
    examples/cpp/julian_day.cpp
    examples/cpp/utc_date.cpp
    examples/cpp/delta_t.cpp
    examples/cpp/tropical_sun.cpp
    examples/cpp/equatorial_sun.cpp
    examples/cpp/separation.cpp
    examples/cpp/lahiri_chart.cpp
    examples/cpp/mean_nutation.cpp
    examples/cpp/citra_galactic.cpp
    examples/cpp/inclined_lilith.cpp
    examples/cpp/ecliptic_lilith.cpp
    examples/cpp/placidus.cpp
    examples/cpp/equal_houses.cpp
    examples/cpp/whole_sign.cpp
    examples/cpp/snap_then_shift.cpp
    examples/cpp/armc_fixed.cpp
    examples/cpp/equator_fixed.cpp
    examples/cpp/sripati.cpp
    examples/cpp/polich_page.cpp
    examples/cpp/polar_fallback.cpp
  )
  target_include_directories(eph_example_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp
  )
  target_link_libraries(eph_example_tests PRIVATE eph)
  add_test(NAME eph_example_tests COMMAND eph_example_tests)
endif()

# Stock Emscripten only. Leave this OFF in the monorepo p2996 wasm build.
option(EPH_JS "Build the JavaScript wasm binding" OFF)
if(EPH_JS)
  if(NOT EMSCRIPTEN)
    message(FATAL_ERROR "EPH_JS requires the Emscripten toolchain (emcmake)")
  endif()
  # The static library must match the link, or emscripten looks for __cxa_throw.
  # -Oz is a link mode: emcc runs wasm-opt -Oz and emits a smaller runtime.
  # The JS entry never opens a kernel file, so this link drops the filesystem.
  target_compile_options(eph PRIVATE -Oz -fno-exceptions -fno-rtti)
  add_executable(eph_js js/src/eph_api.cpp)
  target_link_libraries(eph_js PRIVATE eph)
  target_compile_options(eph_js PRIVATE -Oz -fno-exceptions -fno-rtti -Wall -Wextra)
  target_link_options(eph_js PRIVATE
    "-Oz"
    "-fno-exceptions"
    "-fno-rtti"
    "-sDISABLE_EXCEPTION_CATCHING=1"
    "-sSUPPORT_LONGJMP=0"
    "-sMALLOC=emmalloc"
    "-sFILESYSTEM=0"
    "-sASSERTIONS=0"
    "-sMODULARIZE=1"
    "-sEXPORT_ES6=1"
    "-sENVIRONMENT=web,node"
    "-sALLOW_MEMORY_GROWTH=1"
    "-sEXPORTED_RUNTIME_METHODS=HEAPF64,wasmMemory"
    "SHELL:-sEXPORTED_FUNCTIONS=[_malloc,_free,_eph_create,_eph_destroy,_eph_position,_eph_equatorial,_eph_houses,_eph_julian_day,_eph_rev_jul,_eph_delta_t,_eph_ayanamsa,_eph_sep_deg]"
  )
  set_target_properties(eph_js PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/js/dist
    OUTPUT_NAME eph
  )
endif()
