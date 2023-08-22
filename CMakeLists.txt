cmake_minimum_required(VERSION 3.26)
project(MonoTest)

set(CMAKE_CXX_STANDARD 20)

if(WIN32)
    # Use the MONO_ROOT environment variable to locate Mono
    if(DEFINED ENV{MONO_ROOT})
        set(MONO_ROOT $ENV{MONO_ROOT})
    else()
        message(FATAL_ERROR "Environment variable MONO_ROOT is not set. Please set it to the path of your Mono installation.")
    endif()

    set(MONO_INCLUDE_DIRS ${MONO_ROOT}/include/mono-2.0)
    set(MONO_LIBRARY_DIRS ${MONO_ROOT}/lib)
    set(MONO_LIBRARIES ${MONO_ROOT}/lib/mono-2.0.lib)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(MONO REQUIRED mono-2)
endif()

#message("include dirs: ${MONO_INCLUDE_DIRS}")
#message("lib dirs: ${MONO_LIBRARY_DIRS}")
#message("libs: ${MONO_LIBRARIES}")

include_directories(${MONO_INCLUDE_DIRS})
link_directories(${MONO_LIBRARY_DIRS})

add_executable(MonoTest src/main.cpp)

target_link_libraries(MonoTest ${MONO_LIBRARIES})

set(OUTPUT_DLL assembly.dll)
set(CSHARP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/csharp)

file(GLOB_RECURSE CSHARP_SOURCES "${CSHARP_DIR}/*.cs")

# Create a custom target to compile C# sources into a DLL
add_custom_target(CompileCSharp
        COMMAND mcs -target:library -out:${OUTPUT_DLL} ${CSHARP_SOURCES}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Compiling C# sources into ${OUTPUT_DLL}"
)

add_dependencies(MonoTest CompileCSharp)