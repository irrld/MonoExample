cmake_minimum_required(VERSION 3.26)
project(MonoTest)

set(CMAKE_CXX_STANDARD 20)
set(COMPILE_DLL_RUNTIME ON)

set(CMAKE_BUILD_TYPE Debug)

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
    message("mono include dirs: ${MONO_}")
endif()

#message("include dirs: ${MONO_INCLUDE_DIRS}")
#message("lib dirs: ${MONO_LIBRARY_DIRS}")
#message("libs: ${MONO_LIBRARIES}")

include_directories(${MONO_INCLUDE_DIRS})
link_directories(${MONO_LIBRARY_DIRS})

include_directories(include)
add_executable(MonoTest src/main.cc src/mono_util.cc)

target_link_libraries(MonoTest ${MONO_LIBRARIES})

set(CSHARP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/csharp)
file(GLOB_RECURSE CSHARP_SOURCES "${CSHARP_DIR}/*.cs")

if(COMPILE_DLL_RUNTIME)
    add_definitions(-DCOMPILE_DLL_RUNTIME)
    # copy mcs to the build directory
    if(WIN32)
        # todo
    else()
        add_custom_command(TARGET MonoTest PRE_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory /opt/homebrew/opt/mono/bin ${CMAKE_BINARY_DIR}/mono/bin
                COMMENT "Copying mono bin to the build directory"
        )
        add_custom_command(TARGET MonoTest PRE_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory /opt/homebrew/opt/mono/etc ${CMAKE_BINARY_DIR}/mono/etc
                COMMENT "Copying mono etc to the build directory"
        )
        add_custom_command(TARGET MonoTest PRE_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory /opt/homebrew/opt/mono/lib ${CMAKE_BINARY_DIR}/mono/lib
                COMMENT "Copying mono lib to the build directory"
        )
    endif()

    add_custom_target(CopyCSharpSources
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/csharp ${CMAKE_BINARY_DIR}/csharp
            COMMENT "Copying C# sources to the build directory"
    )
    add_dependencies(MonoTest CopyCSharpSources)
else()
    set(OUTPUT_DLL_FOLDER ${CMAKE_BINARY_DIR}/obj)
    set(OUTPUT_DLL ${OUTPUT_DLL_FOLDER}/assembly.dll)

    # Create a custom target to compile C# sources into a DLL
    add_custom_command(TARGET MonoTest POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_DLL_FOLDER})

    add_custom_target(CompileCSharp
            COMMAND mcs -target:library -out:${OUTPUT_DLL} ${CSHARP_SOURCES}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Compiling C# sources into ${OUTPUT_DLL}"
    )

    add_dependencies(MonoTest CompileCSharp)
endif()