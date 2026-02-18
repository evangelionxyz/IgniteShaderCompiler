#ifndef _SHADER_ENUMS_H
#define _SHADER_ENUMS_H

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Core shared enums and helper functions used by both C and C++ APIs.
 * These values define shader stage/platform/compiler selection, result codes,
 * and compact format mappings for reflected vertex/stage IO information.
 */

typedef enum IGNITE_LogType
{
    IGNITE_LOG_TYPE_INFO = 0,
    IGNITE_LOG_TYPE_WARNING = 1,
    IGNITE_LOG_TYPE_ERROR = 2
} IGNITE_LogType;

typedef enum IGNITE_ShaderType
{
    IGNITE_SHADER_TYPE_VERTEX = 0,
    IGNITE_SHADER_TYPE_PIXEL = 1,
    IGNITE_SHADER_TYPE_GEOMETRY = 2,
    IGNITE_SHADER_TYPE_COMPUTE = 3,
    IGNITE_SHADER_TYPE_TESSELLATION = 4
} IGNITE_ShaderType;

typedef enum IGNITE_ShaderPlatformType
{
    IGNITE_SHADER_PLATFORM_TYPE_DXBC = 0,
    IGNITE_SHADER_PLATFORM_TYPE_DXIL = 1,
    IGNITE_SHADER_PLATFORM_TYPE_SPIRV = 2
} IGNITE_ShaderPlatformType;

typedef enum IGNITE_ShaderCompilerType
{
    IGNITE_SHADER_COMPILER_TYPE_DXC = 0,
    IGNITE_SHADER_COMPILER_TYPE_FXC = 1,
    IGNITE_SHADER_COMPILER_TYPE_SLANG = 2
} IGNITE_ShaderCompilerType;

typedef enum IGNITE_OptimizationLevel
{
    IGNITE_OPT_LEVEL_0 = 0,
    IGNITE_OPT_LEVEL_1 = 1,
    IGNITE_OPT_LEVEL_2 = 2,
    IGNITE_OPT_LEVEL_3 = 3
} IGNITE_OptimizationLevel;

typedef enum IGNITE_ResultCode
{
    IGNITE_RESULT_OK = 0,
    IGNITE_RESULT_INVALID_ARGUMENT = 1,
    IGNITE_RESULT_UNSUPPORTED_PLATFORM = 2,
    IGNITE_RESULT_COMPILATION_FAILED = 3,
    IGNITE_RESULT_INTERNAL_ERROR = 4
} IGNITE_ResultCode;

typedef enum IGNITE_VertexElementFormat
{
    IGNITE_VERTEX_ELEMENT_FORMAT_INVALID,

    /* 32-bit Signed Integers */
    IGNITE_VERTEX_ELEMENT_FORMAT_INT,
    IGNITE_VERTEX_ELEMENT_FORMAT_INT2,
    IGNITE_VERTEX_ELEMENT_FORMAT_INT3,
    IGNITE_VERTEX_ELEMENT_FORMAT_INT4,

    /* 32-bit Unsigned Integers */
    IGNITE_VERTEX_ELEMENT_FORMAT_UINT,
    IGNITE_VERTEX_ELEMENT_FORMAT_UINT2,
    IGNITE_VERTEX_ELEMENT_FORMAT_UINT3,
    IGNITE_VERTEX_ELEMENT_FORMAT_UINT4,

    /* 32-bit Floats */
    IGNITE_VERTEX_ELEMENT_FORMAT_FLOAT,
    IGNITE_VERTEX_ELEMENT_FORMAT_FLOAT2,
    IGNITE_VERTEX_ELEMENT_FORMAT_FLOAT3,
    IGNITE_VERTEX_ELEMENT_FORMAT_FLOAT4,

    /* 8-bit Signed Integers */
    IGNITE_VERTEX_ELEMENT_FORMAT_BYTE2,
    IGNITE_VERTEX_ELEMENT_FORMAT_BYTE4,

    /* 8-bit Unsigned Integers */
    IGNITE_VERTEX_ELEMENT_FORMAT_UBYTE2,
    IGNITE_VERTEX_ELEMENT_FORMAT_UBYTE4,

    /* 8-bit Signed Normalized */
    IGNITE_VERTEX_ELEMENT_FORMAT_BYTE2_NORM,
    IGNITE_VERTEX_ELEMENT_FORMAT_BYTE4_NORM,

    /* 8-bit Unsigned Normalized */
    IGNITE_VERTEX_ELEMENT_FORMAT_UBYTE2_NORM,
    IGNITE_VERTEX_ELEMENT_FORMAT_UBYTE4_NORM,

    /* 16-bit Signed Integers */
    IGNITE_VERTEX_ELEMENT_FORMAT_SHORT2,
    IGNITE_VERTEX_ELEMENT_FORMAT_SHORT4,

    /* 16-bit Unsigned Integers */
    IGNITE_VERTEX_ELEMENT_FORMAT_USHORT2,
    IGNITE_VERTEX_ELEMENT_FORMAT_USHORT4,

    /* 16-bit Signed Normalized */
    IGNITE_VERTEX_ELEMENT_FORMAT_SHORT2_NORM,
    IGNITE_VERTEX_ELEMENT_FORMAT_SHORT4_NORM,

    /* 16-bit Unsigned Normalized */
    IGNITE_VERTEX_ELEMENT_FORMAT_USHORT2_NORM,
    IGNITE_VERTEX_ELEMENT_FORMAT_USHORT4_NORM,

    /* 16-bit Floats */
    IGNITE_VERTEX_ELEMENT_FORMAT_HALF2,
    IGNITE_VERTEX_ELEMENT_FORMAT_HALF4
} IGNITE_VertexElementFormat;

static const char *IGNITE_ShaderPlatformToString(IGNITE_ShaderPlatformType type)
{
    switch (type)
    {
        case IGNITE_SHADER_PLATFORM_TYPE_DXIL: return "DXIL";
        case IGNITE_SHADER_PLATFORM_TYPE_DXBC: return "DXBC";
        case IGNITE_SHADER_PLATFORM_TYPE_SPIRV: return "SPIRV";
        default: return "Unknown";
    }
}

/* Returns the default output file extension for a target shader platform. */
static const char *IGNITE_ShaderPlatformExtension(IGNITE_ShaderPlatformType type)
{
    switch (type)
    {
    case IGNITE_SHADER_PLATFORM_TYPE_DXIL: return ".dxil";
    case IGNITE_SHADER_PLATFORM_TYPE_DXBC: return ".dxbc";
    case IGNITE_SHADER_PLATFORM_TYPE_SPIRV: return ".spirv";
    default: return "Unknown";
    }
}

/* Returns executable name used by the selected shader compiler backend. */
 static const char *IGNITE_ShaderCompilerExecutablePath(IGNITE_ShaderCompilerType type)
{
    switch (type)
    {
#ifdef _WIN32
    case IGNITE_SHADER_COMPILER_TYPE_DXC: return "dxc.exe";
    case IGNITE_SHADER_COMPILER_TYPE_FXC: return "fxc.exe";
    case IGNITE_SHADER_COMPILER_TYPE_SLANG: return "slangc.exe";
#else
    case IGNITE_SHADER_COMPILER_TYPE_DXC: return "dxc";
    case IGNITE_SHADER_COMPILER_TYPE_FXC: return "fxc";
    case IGNITE_SHADER_COMPILER_TYPE_SLANG: return "slangc";
#endif
    default: return "Unknown";
    }
}

/* Returns shader profile prefix used by HLSL-style targets (vs/ps/gs/cs/ts). */
static const char *IGNITE_ShaderTypeToProfile(IGNITE_ShaderType type)
{
    switch (type)
    {
    case IGNITE_SHADER_TYPE_VERTEX: return "vs";
    case IGNITE_SHADER_TYPE_PIXEL: return "ps";
    case IGNITE_SHADER_TYPE_GEOMETRY: return "gs";
    case IGNITE_SHADER_TYPE_COMPUTE: return "cs";
    case IGNITE_SHADER_TYPE_TESSELLATION: return "ts";
    default: return "invalid";
    }
}

/* Returns a human-readable stage name for logs and diagnostics output. */
static const char* IGNITE_GetShaderTypeString(IGNITE_ShaderType type)
{
    switch (type)
    {
        case IGNITE_SHADER_TYPE_VERTEX: return "Vertex";
        case IGNITE_SHADER_TYPE_PIXEL: return "Pixel";
        case IGNITE_SHADER_TYPE_GEOMETRY: return "Geometry";
        case IGNITE_SHADER_TYPE_COMPUTE: return "Compute";
        default: return "Invalid";
    }
}

#ifdef __cplusplus
}
#endif

#endif