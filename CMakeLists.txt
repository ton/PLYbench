cmake_minimum_required(VERSION 3.5)

project(PLYbench
  VERSION 0.0.1
  LANGUAGES C CXX
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

# External (optional) dependencies.
include(FeatureSummary)

find_package(benchmark REQUIRED)
find_package(Catch2 3 REQUIRED)
find_package(PLYwoot REQUIRED)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES)

# Add an object library for the RPly C library.
add_library(RPly OBJECT
  submodules/rply/rply.c
)

add_library(msh_ply OBJECT
  src/msh_ply.c
)

# Add an object library for object code we need to reuse for both the benchmark
# and unit test applications.
add_library(PLYbench OBJECT
  submodules/miniply/miniply.cpp
  submodules/tinyply/source/tinyply.cpp
  submodules/vcglib/wrap/ply/plylib.cpp
  src/parsers.cpp
  src/util.cpp
  src/writers.cpp
)

target_include_directories(msh_ply SYSTEM
  PRIVATE
  submodules
)

target_include_directories(PLYbench SYSTEM
  PRIVATE
  submodules
)

target_link_libraries(PLYbench
  PRIVATE
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
  msh_ply
  RPly
  PLYbench
  Catch2::Catch2WithMain
)

target_link_libraries(plybench
  PRIVATE
  msh_ply
  RPly
  PLYbench
  benchmark::benchmark
)

target_compile_options(tests
  PRIVATE
  "$<$<CONFIG:RELEASE>:-Wall;-Wextra;-Wpedantic;-Werror;-Wno-unused-parameter;-fno-strict-aliasing;-march=native>"
)

target_compile_options(plybench
  PRIVATE
  "$<$<CONFIG:RELEASE>:-Wall;-Wextra;-Wpedantic;-Werror;-Wno-unused-parameter;-fno-strict-aliasing;-march=native>"
)
