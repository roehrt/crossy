cmake_minimum_required(VERSION 3.20)
project(crossy)

set(CMAKE_CXX_STANDARD 20)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif ()

# cache SCIP_CPU and SCIP_CPU_ADD
SET(SCIP_CPU 0 CACHE STRING "SCIP CPU limit")
SET(SCIP_CPU_ADD 0 CACHE STRING "SCIP CPU limit additional")

string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_LOWER)
if (SCIP_CPU AND SCIP_CPU GREATER 0)
    set(SIGNATURE crossy-${CMAKE_BUILD_TYPE_LOWER}-S${SCIP_CPU}-${SCIP_CPU_ADD})
else ()
    set(SIGNATURE crossy-${CMAKE_BUILD_TYPE_LOWER}-S0)
endif ()
message(STATUS "signature: ${SIGNATURE}")

add_executable(${SIGNATURE} main.cpp)

target_compile_options(${SIGNATURE} PRIVATE
        -Wall -Wextra -Wfloat-equal
        $<$<CONFIG:RELEASE>:-O3>
        $<$<CONFIG:RELEASE>:-march=native>
        $<$<CONFIG:PACE_STATIC>:-O3>
        $<$<CONFIG:PACE_STATIC>:-march=haswell>
        $<$<CONFIG:DEBUG>:-fsanitize=address,undefined>
        $<$<CONFIG:DEBUG>:-fno-omit-frame-pointer>
        $<$<CONFIG:DEBUG>:-fno-sanitize-recover=all>
        $<$<CONFIG:DEBUG>:-D_GLIBCXX_DEBUG>
        $<$<CONFIG:DEBUG>:-D_LIBCPP_DEBUG>
)

target_link_options(${SIGNATURE} PRIVATE
        $<$<CONFIG:DEBUG>:-fsanitize=address,undefined>
        $<$<CONFIG:DEBUG>:-fno-omit-frame-pointer>
        $<$<CONFIG:PACE_STATIC>:-static>
        $<$<CONFIG:PACE_STATIC>:-static-libgcc>
        $<$<CONFIG:PACE_STATIC>:-static-libstdc++>
)


target_compile_definitions(${SIGNATURE} PRIVATE
        CROSSY_VERSION="1.0.2"
        LARGE_WEIGHTS
        SCIP_CPU=${SCIP_CPU}
        SCIP_CPU_ADD=${SCIP_CPU_ADD}
)

if (CMAKE_BUILD_TYPE STREQUAL "PACE_STATIC")
    target_compile_options(${SIGNATURE} PRIVATE -no-pie)
endif ()


file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.hpp)
file(GLOB_RECURSE CPP_SOURCES CONFIGURE_DEPENDS src/*.cpp)

target_sources(${SIGNATURE} PRIVATE ${SOURCES} ${CPP_SOURCES})

target_link_libraries(${SIGNATURE} PRIVATE
        ${CMAKE_SOURCE_DIR}/external/uwrmaxsat/build/release/lib/libuwrmaxsat.a
        ${CMAKE_SOURCE_DIR}/external/cadical/build/libcadical.a
        ${CMAKE_SOURCE_DIR}/external/scipoptsuite-9.0.1/build/lib/libscip.a
        ${CMAKE_SOURCE_DIR}/external/scipoptsuite-9.0.1/build/lib/libsoplex.a
        gmp
)
