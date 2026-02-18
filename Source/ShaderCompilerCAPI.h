#ifndef _SHADER_COMPILER_CAPI_H
#define _SHADER_COMPILER_CAPI_H

#pragma once

#if defined(_WIN32)
    #if defined(IGNITECOMPILER_BUILD_SHARED)
        #define IGNITECOMPILER_CAPI __declspec(dllexport)
    #else
        #define IGNITECOMPILER_CAPI __declspec(dllimport)
    #endif
#elif defined(__GNUC__) || defined(__clang__)
    #define IGNITECOMPILER_CAPI __attribute__((visibility("default")))
#else
    #define IGNITECOMPILER_CAPI
#endif

#include "ShaderBase.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C API surface for the Ignite shader compiler.
 * - Compile shader files to target bytecode formats.
 * - Reflect SPIR-V and DXIL binaries into plain C structs.
 * - Release reflection allocations via IgniteCompiler_FreeReflectionInfo.
 */

/* Input parameters for one compile invocation. */
typedef struct IgniteCompileRequest
{
    const char* inputPath;
    const char* outputDirectory;
    const char* entryPoint;
    const char* shaderModel;
    const char* vulkanVersion;
    const char* vulkanMemoryLayout;
    IGNITE_ShaderType shaderType;
    IGNITE_ShaderPlatformType platformType;
    IGNITE_OptimizationLevel optimizationLevel;
    int warningsAreErrors;
    int allResourcesBound;
    int stripReflection;
    int matrixRowMajor;
    int hlsl2021;
    int embedPdb;
    int pdb;
    int verbose;
    uint32_t tRegShift;
    uint32_t sRegShift;
    union
    {
        uint32_t bRegShift;
        uint32_t rRegShift;
    };
    uint32_t uRegShift;
} IgniteCompileRequest;

/* Reflected vertex attribute metadata. */
typedef struct IgniteVertexAttribute
{
    char* name;
    IGNITE_VertexElementFormat format;
    uint32_t bufferIndex;
    uint32_t offset;
    uint32_t elementStride;
} IgniteVertexAttribute;

/* Generic reflected resource (UBO/image/buffer/sampler). */
typedef struct IgniteShaderResourceInfo
{
    char* name;
    uint32_t id;
    uint32_t set;
    uint32_t binding;
    uint32_t count;
} IgniteShaderResourceInfo;

/* Reflected stage input/output entry metadata. */
typedef struct IgniteShaderStageIOInfo
{
    char* name;
    uint32_t id;
    uint32_t location;
    IGNITE_VertexElementFormat format;
    uint32_t vecSize;
    uint32_t columns;
} IgniteShaderStageIOInfo;

/* Reflected push constant metadata. */
typedef struct IgniteShaderPushConstantInfo
{
    char* name;
    uint32_t size;
} IgniteShaderPushConstantInfo;

/* Aggregate reflection result for one shader binary. */
typedef struct IgniteShaderReflectionInfo
{
    IGNITE_ShaderType shaderType;

    size_t numUniformBuffers;
    size_t numSamplers;
    size_t numStorageTextures;
    size_t numStorageBuffers;
    size_t numSeparateSamplers;
    size_t numSeparateImages;
    size_t numPushConstants;
    size_t numStageInputs;
    size_t numStageOutputs;

    IgniteShaderResourceInfo* uniformBuffers;
    IgniteShaderResourceInfo* sampledImages;
    IgniteShaderResourceInfo* storageImages;
    IgniteShaderResourceInfo* storageBuffers;
    IgniteShaderResourceInfo* separateSamplers;
    IgniteShaderResourceInfo* separateImages;
    IgniteShaderPushConstantInfo* pushConstants;
    IgniteShaderStageIOInfo* stageInputs;
    IgniteShaderStageIOInfo* stageOutputs;
    IgniteVertexAttribute* vertexAttributes;
    size_t vertexAttributeCount;
} IgniteShaderReflectionInfo;

/* Callback signature for compiler/reflection log forwarding. */
typedef void(*IgniteLogCallback)(IGNITE_LogType type, const char* message, void* userData);

/* Returns project version string. */
IGNITECOMPILER_CAPI const char* IgniteCompiler_GetVersion(void);

/* Installs or clears callback-based logging integration. */
IGNITECOMPILER_CAPI void IgniteCompiler_SetLogCallback(IgniteLogCallback callback, void* userData);

/* Compiles an input shader file to request->platformType output. */
IGNITECOMPILER_CAPI IGNITE_ResultCode IgniteCompiler_Compile(const IgniteCompileRequest* request);

/* Reflects SPIR-V words and fills outReflectionInfo. */
IGNITECOMPILER_CAPI IGNITE_ResultCode IgniteCompiler_ReflectSPIRV(const uint32_t* spirvData, size_t sizeInBytes, IGNITE_ShaderType shaderType, IgniteShaderReflectionInfo* outReflectionInfo);

/* Reflects DXIL bytes and fills outReflectionInfo. */
IGNITECOMPILER_CAPI IGNITE_ResultCode IgniteCompiler_ReflectDXIL(const uint8_t* dxilData, size_t sizeInBytes, IGNITE_ShaderType shaderType, IgniteShaderReflectionInfo* outReflectionInfo);

/* Releases heap allocations stored in IgniteShaderReflectionInfo. */
IGNITECOMPILER_CAPI void IgniteCompiler_FreeReflectionInfo(IgniteShaderReflectionInfo* reflectionInfo);

#ifdef __cplusplus
}
#endif

#endif
