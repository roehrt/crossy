cmake_minimum_required(VERSION 3.20)
project(crossy)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(external/EvalMaxSAT)

add_executable(crossy main.cpp)

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -ggdb3 -fsanitize=address,undefined -fno-omit-frame-pointer \
-fno-sanitize-recover=all -fmax-errors=2 -D_GLIBCXX_DEBUG -D_LIBCPP_DEBUG -D_GLIBCXX_ASSERTIONS")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g -ggdb3")

target_compile_definitions(crossy PRIVATE CROSSY_VERSION="1.0.2" LARGE_WEIGHTS)

if (CMAKE_BUILD_TYPE STREQUAL "PACE_STATIC")
    target_compile_options(crossy PRIVATE -no-pie)
endif ()


file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.hpp)
file(GLOB_RECURSE CPP_SOURCES CONFIGURE_DEPENDS src/*.cpp)

target_sources(crossy PRIVATE ${SOURCES} ${CPP_SOURCES})
target_link_libraries(crossy PRIVATE EvalMaxSAT)
