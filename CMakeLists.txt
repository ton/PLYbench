cmake_minimum_required(VERSION 3.5)

project(PLYbench
  VERSION 0.0.1
  LANGUAGES CXX
  HOMEPAGE_URL "https://github.com/ton/plybench"
  DESCRIPTION "PLY benchmark collection"
)

# Custom cmake modules
set(PROJECT_CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH ${PROJECT_CMAKE_MODULE_PATH})

# Require C++17 support.
set(CMAKE_CXX_STANDARD 17)

# By default, build in release mode, using -O2.
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

# Put all executables in build/.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Set the appropriate compiler flags.
add_compile_options("$<$<CONFIG:RELEASE>:-Wall;-Wextra;-Wpedantic;-Werror;-Wno-unused-parameter;-fno-strict-aliasing;-march=native>")

# External (optional) dependencies.
include(FeatureSummary)

find_package(benchmark REQUIRED)
find_package(Catch2 3 REQUIRED)
find_package(PLYwoot REQUIRED)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES)

# Add an object library for object code we need to reuse for both the benchmark
# and unit test applications.
add_library(PLYbench OBJECT
  submodules/miniply/miniply.cpp
  submodules/vcglib/wrap/ply/plylib.cpp
  src/parsers.cpp
)

# Disable warnings on external source code.
set_source_files_properties(submodules/miniply/miniply.cpp PROPERTIES COMPILE_FLAGS -w)
set_source_files_properties(submodules/vcglib/wrap/ply/plylib.cpp PROPERTIES COMPILE_FLAGS -w)

target_include_directories(PLYbench SYSTEM
  PRIVATE
  submodules
)

target_link_libraries(PLYbench
  PLYwoot::plywoot
)

# Define the benchmark target.
add_executable(plybench
  src/plybench.cpp
)

# Test executable; depends on Catch2.
add_executable(tests
  src/tests.cpp
)
target_link_libraries(tests
  PRIVATE
  PLYbench
  Catch2::Catch2WithMain
)

target_link_libraries(plybench
  PRIVATE
  PLYbench
  benchmark::benchmark
)
