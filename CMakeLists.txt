# Copyright (c) 2026 Evangelion Manuhutu

cmake_minimum_required(VERSION 3.20)

project(IgniteCompiler)
set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE SRC_FILES 
    Source/*.h
    Source/*.cpp
)

add_library(IgniteCompiler SHARED ${SRC_FILES})

target_include_directories(IgniteCompiler
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Source>
        $<INSTALL_INTERFACE:include>
)

target_compile_definitions(IgniteCompiler PRIVATE IGNITECOMPILER_BUILD_SHARED)

# defines
if (WIN32)
    message("   >> Platform: Windows")
    
    # enable Hot Reload for MSVC compilers if supported.
    if (POLICY CMP0141)
        cmake_policy(SET CMP0141 NEW)
        set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT $<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>)
    endif()

elseif (UNIX AND NOT APPLE)
    # check if we're running in WSL
    execute_process(COMMAND uname -r OUTPUT_VARIABLE WSL_KERNEL_NAME)
    string(FIND "${WSL_KERNEL_NAME}" "Microsoft" WSL_FOUND)
    
    if (WSL_FOUND EQUAL -1)
        message("   >> Platform: WSL")
    else()
        message("   >> Platform: LINUX")
    endif()
endif()

install(TARGETS IgniteCompiler
    EXPORT IgniteCompilerTargets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Source/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

install(EXPORT IgniteCompilerTargets
    FILE IgniteCompilerTargets.cmake
    NAMESPACE Ignite::
    DESTINATION lib/cmake/IgniteCompiler
)

option(IGNITECOMPILER_BUILD_EXAMPLES "Build IgniteCompiler examples" ON)
if (IGNITECOMPILER_BUILD_EXAMPLES)
    include(${CMAKE_CURRENT_SOURCE_DIR}/Example/c_example.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/Example/cpp_example.cmake)
endif()

# linking
if (WIN32)
    find_library(DXCOMPILER_LIB
        NAMES dxcompiler
        PATHS
            "$ENV{VULKAN_SDK}/Lib"
            "$ENV{VULKAN_SDK}/Lib/x64"
        NO_DEFAULT_PATH
    )

    if (NOT DXCOMPILER_LIB)
        message(FATAL_ERROR "dxcompiler.lib not found. Please install DXC/Vulkan SDK and set VULKAN_SDK.")
    endif()

    target_include_directories(IgniteCompiler PRIVATE "$ENV{VULKAN_SDK}/Include")
    target_link_libraries(IgniteCompiler PRIVATE
        ${DXCOMPILER_LIB}

        "d3d12.lib"
        "dxgi.lib"
        "d3dcompiler"
        "dxguid"

        $<$<CONFIG:Debug>:$ENV{VULKAN_SDK}/Lib/shaderc_sharedd.lib>
        $<$<CONFIG:Debug>:$ENV{VULKAN_SDK}/Lib/spirv-cross-cd.lib>
        $<$<CONFIG:Debug>:$ENV{VULKAN_SDK}/Lib/spirv-cross-cored.lib>
        $<$<CONFIG:Debug>:$ENV{VULKAN_SDK}/Lib/spirv-cross-glsld.lib>
        $<$<CONFIG:Debug>:$ENV{VULKAN_SDK}/Lib/spirv-cross-hlsld.lib>
        $<$<CONFIG:Debug>:$ENV{VULKAN_SDK}/Lib/spirv-cross-msld.lib>
        $<$<CONFIG:Debug>:$ENV{VULKAN_SDK}/Lib/spirv-cross-cppd.lib>
        $<$<CONFIG:Debug>:$ENV{VULKAN_SDK}/Lib/spirv-cross-reflectd.lib>
        
        $<$<NOT:$<CONFIG:Debug>>:$ENV{VULKAN_SDK}/Lib/shaderc_shared.lib>
        $<$<NOT:$<CONFIG:Debug>>:$ENV{VULKAN_SDK}/Lib/spirv-cross-c.lib>
        $<$<NOT:$<CONFIG:Debug>>:$ENV{VULKAN_SDK}/Lib/spirv-cross-core.lib>
        $<$<NOT:$<CONFIG:Debug>>:$ENV{VULKAN_SDK}/Lib/spirv-cross-glsl.lib>
        $<$<NOT:$<CONFIG:Debug>>:$ENV{VULKAN_SDK}/Lib/spirv-cross-hlsl.lib>
        $<$<NOT:$<CONFIG:Debug>>:$ENV{VULKAN_SDK}/Lib/spirv-cross-msl.lib>
        $<$<NOT:$<CONFIG:Debug>>:$ENV{VULKAN_SDK}/Lib/spirv-cross-cpp.lib>
        $<$<NOT:$<CONFIG:Debug>>:$ENV{VULKAN_SDK}/Lib/spirv-cross-reflect.lib>
    )
elseif (UNIX AND NOT APPLE)
    target_include_directories(IgniteCompiler PRIVATE "/usr/include")
    target_link_directories(IgniteCompiler PRIVATE /usr/lib)

    target_link_libraries(IgniteCompiler PRIVATE
        vulkan
        shaderc_shared
        spirv-cross-c
        spirv-cross-core
        spirv-cross-glsl
        spirv-cross-hlsl
        spirv-cross-msl
        spirv-cross-cpp
        spirv-cross-reflect
        pthread dl m rt
    )
endif()
