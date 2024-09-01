cmake_minimum_required(VERSION 3.12)
project(EvalMaxSAT)

cmake_policy(SET CMP0079 NEW)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE cadical_source lib/cadical/src/*)
add_library(cadical ${cadical_source})
target_compile_definitions(cadical PUBLIC VERSION="ipasirup-refinements")

file(GLOB_RECURSE malib_source lib/MaLib/src/*)
add_library(MaLib ${malib_source})

find_package(ZLIB)
include_directories(${ZLIB_INCLUDE_DIR})

add_library(EvalMaxSAT STATIC lib/EvalMaxSAT/src/EvalMaxSAT.h)
set_target_properties(EvalMaxSAT PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(EvalMaxSAT PUBLIC lib/EvalMaxSAT/src lib/MaLib/src lib/cadical/src)
target_link_libraries(EvalMaxSAT PUBLIC MaLib cadical z)