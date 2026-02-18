#include "ShaderCompiler.h"
#include "ShaderCompilerCAPI.h"

#include <exception>
#include <algorithm>

#include <cctype>
#include <cstring>
#include <cstdlib>


namespace
{
    // Holds current C callback wiring used by bridge callback.
    struct CLogBridgeContext
    {
        IgniteLogCallback callback = nullptr;
        void* userData = nullptr;
    };

    CLogBridgeContext g_logBridge = {};

    // Duplicates std::string into malloc-allocated C string.
    char* DuplicateCString(const std::string& value)
    {
        const size_t len = value.size();
        char* result = static_cast<char*>(std::malloc(len + 1));
        if (!result)
        {
            return nullptr;
        }

        std::memcpy(result, value.c_str(), len + 1);
        return result;
    }

    // Bridges C++ log callback invocation to C callback signature.
    void CLogBridge(IGNITE_LogType type, const char* message, void* userData)
    {
        CLogBridgeContext* bridge = reinterpret_cast<CLogBridgeContext*>(userData);
        if (bridge == nullptr || bridge->callback == nullptr)
        {
            return;
        }

        bridge->callback(type, message, bridge->userData);
    }

    // ASCII lower-case helper for extension checks.
    std::string ToLower(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        return value;
    }

    // Detects GLSL input by file extension.
    bool IsGlslFile(const std::filesystem::path& path)
    {
        return ToLower(path.extension().string()) == ".glsl";
    }

    // Release helpers for nested reflection arrays.
    void FreeResourceArray(IgniteShaderResourceInfo* array, size_t count)
    {
        if (!array)
        {
            return;
        }

        for (size_t i = 0; i < count; ++i)
        {
            std::free(array[i].name);
        }
        std::free(array);
    }

    void FreeStageIOArray(IgniteShaderStageIOInfo* array, size_t count)
    {
        if (!array)
        {
            return;
        }

        for (size_t i = 0; i < count; ++i)
        {
            std::free(array[i].name);
        }
        std::free(array);
    }

    void FreePushConstantArray(IgniteShaderPushConstantInfo* array, size_t count)
    {
        if (!array)
        {
            return;
        }

        for (size_t i = 0; i < count; ++i)
        {
            std::free(array[i].name);
        }
        std::free(array);
    }

    void FreeVertexAttributeArray(IgniteVertexAttribute* array, size_t count)
    {
        if (!array)
        {
            return;
        }

        for (size_t i = 0; i < count; ++i)
        {
            std::free(array[i].name);
        }
        std::free(array);
    }

    // Conversion helpers from C++ reflection vectors to C arrays.
    bool FillResourceArray(const std::vector<ignite::ShaderResourceInfo>& source, IgniteShaderResourceInfo** outArray)
    {
        *outArray = nullptr;
        if (source.empty())
        {
            return true;
        }

        IgniteShaderResourceInfo* array = static_cast<IgniteShaderResourceInfo*>(std::calloc(source.size(), sizeof(IgniteShaderResourceInfo)));
        if (!array)
        {
            return false;
        }

        for (size_t i = 0; i < source.size(); ++i)
        {
            array[i].name = DuplicateCString(source[i].name);
            array[i].id = source[i].id;
            array[i].set = source[i].set;
            array[i].binding = source[i].binding;
            array[i].count = source[i].count;
        }

        *outArray = array;
        return true;
    }

    bool FillStageIOArray(const std::vector<ignite::ShaderStageIOInfo>& source, IgniteShaderStageIOInfo** outArray)
    {
        *outArray = nullptr;
        if (source.empty())
        {
            return true;
        }

        IgniteShaderStageIOInfo* array = static_cast<IgniteShaderStageIOInfo*>(std::calloc(source.size(), sizeof(IgniteShaderStageIOInfo)));
        if (!array)
        {
            return false;
        }

        for (size_t i = 0; i < source.size(); ++i)
        {
            array[i].name = DuplicateCString(source[i].name);
            array[i].id = source[i].id;
            array[i].location = source[i].location;
            array[i].format = source[i].format;
            array[i].vecSize = source[i].vecSize;
            array[i].columns = source[i].columns;
        }

        *outArray = array;
        return true;
    }

    bool FillPushConstantArray(const std::vector<ignite::ShaderPushConstantInfo>& source, IgniteShaderPushConstantInfo** outArray)
    {
        *outArray = nullptr;
        if (source.empty())
        {
            return true;
        }

        IgniteShaderPushConstantInfo* array = static_cast<IgniteShaderPushConstantInfo*>(std::calloc(source.size(), sizeof(IgniteShaderPushConstantInfo)));
        if (!array)
        {
            return false;
        }

        for (size_t i = 0; i < source.size(); ++i)
        {
            array[i].name = DuplicateCString(source[i].name);
            array[i].size = source[i].size;
        }

        *outArray = array;
        return true;
    }

    bool FillVertexArray(const std::vector<ignite::VertexAttribute>& source, IgniteVertexAttribute** outArray)
    {
        *outArray = nullptr;
        if (source.empty())
        {
            return true;
        }

        IgniteVertexAttribute* array = static_cast<IgniteVertexAttribute*>(std::calloc(source.size(), sizeof(IgniteVertexAttribute)));
        if (!array)
        {
            return false;
        }

        for (size_t i = 0; i < source.size(); ++i)
        {
            array[i].name = DuplicateCString(source[i].name);
            array[i].format = source[i].format;
            array[i].bufferIndex = source[i].bufferIndex;
            array[i].offset = source[i].offset;
            array[i].elementStride = source[i].elementStride;
        }

        *outArray = array;
        return true;
    }

    // Populates C reflection object from the C++ reflection model.
    IGNITE_ResultCode FillCReflectionInfo(const ignite::ShaderReflectionInfo& reflection, IgniteShaderReflectionInfo* outReflectionInfo)
    {
        outReflectionInfo->shaderType = reflection.shaderType;
        outReflectionInfo->numUniformBuffers = reflection.numUniformBuffers;
        outReflectionInfo->numSamplers = reflection.numSamplers;
        outReflectionInfo->numStorageTextures = reflection.numStorageTextures;
        outReflectionInfo->numStorageBuffers = reflection.numStorageBuffers;
        outReflectionInfo->numSeparateSamplers = reflection.numSeparateSamplers;
        outReflectionInfo->numSeparateImages = reflection.numSeparateImages;
        outReflectionInfo->numPushConstants = reflection.numPushConstants;
        outReflectionInfo->numStageInputs = reflection.numStageInputs;
        outReflectionInfo->numStageOutputs = reflection.numStageOutputs;
        outReflectionInfo->vertexAttributeCount = reflection.vertexAttributes.size();

        if (!FillResourceArray(reflection.uniformBuffers, &outReflectionInfo->uniformBuffers) ||
            !FillResourceArray(reflection.sampledImages, &outReflectionInfo->sampledImages) ||
            !FillResourceArray(reflection.storageImages, &outReflectionInfo->storageImages) ||
            !FillResourceArray(reflection.storageBuffers, &outReflectionInfo->storageBuffers) ||
            !FillResourceArray(reflection.separateSamplers, &outReflectionInfo->separateSamplers) ||
            !FillResourceArray(reflection.separateImages, &outReflectionInfo->separateImages) ||
            !FillPushConstantArray(reflection.pushConstants, &outReflectionInfo->pushConstants) ||
            !FillStageIOArray(reflection.stageInputs, &outReflectionInfo->stageInputs) ||
            !FillStageIOArray(reflection.stageOutputs, &outReflectionInfo->stageOutputs) ||
            !FillVertexArray(reflection.vertexAttributes, &outReflectionInfo->vertexAttributes))
        {
            return IGNITE_RESULT_INTERNAL_ERROR;
        }

        return IGNITE_RESULT_OK;
    }
}

extern "C"
{
    // C API: version query.
    const char* IgniteCompiler_GetVersion(void)
    {
        return ignite::ShaderCompiler::GetVersion();
    }

    // C API: log callback registration/clear.
    void IgniteCompiler_SetLogCallback(IgniteLogCallback callback, void* userData)
    {
        g_logBridge.callback = callback;
        g_logBridge.userData = userData;

        if (callback == nullptr)
        {
            ignite::ShaderCompiler::ClearLogCallback();
            return;
        }

        ignite::ShaderCompiler::SetLogCallback(CLogBridge, &g_logBridge);
    }

    // C API: compile one shader file according to request options.
    IGNITE_ResultCode IgniteCompiler_Compile(const IgniteCompileRequest* request)
    {
        if (request == nullptr || request->inputPath == nullptr || request->inputPath[0] == '\0')
        {
            return IGNITE_RESULT_INVALID_ARGUMENT;
        }

        try
        {
            ignite::CompilerOptions options = {};
            options.compilerType = IGNITE_SHADER_COMPILER_TYPE_DXC;
            options.platformType = request->platformType;
            options.filepath = request->inputPath;

            if (request->outputDirectory != nullptr && request->outputDirectory[0] != '\0')
            {
                options.outputFilepath = request->outputDirectory;
            }

            options.shaderDesc.entryPoint = (request->entryPoint != nullptr && request->entryPoint[0] != '\0')
                ? request->entryPoint
                : "main";

            options.shaderDesc.shaderModel = (request->shaderModel != nullptr && request->shaderModel[0] != '\0')
                ? request->shaderModel
                : "6_5";

            options.shaderDesc.vulkanVersion = (request->vulkanVersion != nullptr && request->vulkanVersion[0] != '\0')
                ? request->vulkanVersion
                : "1.3";

            if (request->vulkanMemoryLayout != nullptr)
            {
                options.shaderDesc.vulkanMemoryLayout = request->vulkanMemoryLayout;
            }

            options.shaderDesc.shaderType = request->shaderType;
            options.shaderDesc.optLevel = request->optimizationLevel;

            options.tRegShift = request->tRegShift;
            options.sRegShift = request->sRegShift;
            options.bRegShift = request->bRegShift;
            options.uRegShift = request->uRegShift;

            options.warningsAreErrors = request->warningsAreErrors != 0;
            options.allResourcesBound = request->allResourcesBound != 0;
            options.stripReflection = request->stripReflection != 0;
            options.matrixRowMajor = request->matrixRowMajor != 0;
            options.hlsl2021 = request->hlsl2021 != 0;
            options.embedPdb = request->embedPdb != 0;
            options.pdb = request->pdb != 0;
            options.verbose = request->verbose != 0;

            if (IsGlslFile(options.filepath))
            {
                std::vector<uint8_t> result = ignite::ShaderCompiler::CompileGLSL(options);
                if (result.empty())
                {
                    return IGNITE_RESULT_COMPILATION_FAILED;
                }
                return IGNITE_RESULT_OK;
            }

#if defined(_WIN32)
            std::shared_ptr<ignite::DXCInstance> dxc = ignite::ShaderCompiler::CreateDXCCompiler();
            if (!dxc)
            {
                return IGNITE_RESULT_INTERNAL_ERROR;
            }

            std::vector<uint8_t> result = ignite::ShaderCompiler::CompileDXC(dxc, options);
            if (result.empty())
            {
                return IGNITE_RESULT_COMPILATION_FAILED;
            }

            return IGNITE_RESULT_OK;
#else
            return IGNITE_RESULT_UNSUPPORTED_PLATFORM;
#endif
        }
        catch (...)
        {
            return IGNITE_RESULT_INTERNAL_ERROR;
        }
    }

    // C API: SPIR-V reflection entry point.
    IGNITE_ResultCode IgniteCompiler_ReflectSPIRV(const uint32_t* spirvData, size_t sizeInBytes, IGNITE_ShaderType shaderType, IgniteShaderReflectionInfo* outReflectionInfo)
    {
        if (!spirvData || sizeInBytes == 0 || !outReflectionInfo)
        {
            return IGNITE_RESULT_INVALID_ARGUMENT;
        }

        std::memset(outReflectionInfo, 0, sizeof(*outReflectionInfo));

        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(spirvData);
        std::vector<uint8_t> shaderCode(bytes, bytes + sizeInBytes);

        try
        {
            ignite::ShaderReflectionInfo reflection = ignite::ShaderReflection::SPIRVReflect(shaderType, shaderCode);

            IGNITE_ResultCode result = FillCReflectionInfo(reflection, outReflectionInfo);
            if (result != IGNITE_RESULT_OK)
            {
                IgniteCompiler_FreeReflectionInfo(outReflectionInfo);
            }
            return result;
        }
        catch (...)
        {
            IgniteCompiler_FreeReflectionInfo(outReflectionInfo);
            return IGNITE_RESULT_INTERNAL_ERROR;
        }
    }

    // C API: DXIL reflection entry point.
    IGNITE_ResultCode IgniteCompiler_ReflectDXIL(const uint8_t* dxilData, size_t sizeInBytes, IGNITE_ShaderType shaderType, IgniteShaderReflectionInfo* outReflectionInfo)
    {
        if (!dxilData || sizeInBytes == 0 || !outReflectionInfo)
        {
            return IGNITE_RESULT_INVALID_ARGUMENT;
        }

        std::memset(outReflectionInfo, 0, sizeof(*outReflectionInfo));

        std::vector<uint8_t> shaderCode(dxilData, dxilData + sizeInBytes);

        try
        {
            ignite::ShaderReflectionInfo reflection = ignite::ShaderReflection::DXILReflect(shaderType, shaderCode);

            IGNITE_ResultCode result = FillCReflectionInfo(reflection, outReflectionInfo);
            if (result != IGNITE_RESULT_OK)
            {
                IgniteCompiler_FreeReflectionInfo(outReflectionInfo);
            }
            return result;
        }
        catch (...)
        {
            IgniteCompiler_FreeReflectionInfo(outReflectionInfo);
            return IGNITE_RESULT_INTERNAL_ERROR;
        }
    }

    // C API: release allocations produced by reflection functions.
    void IgniteCompiler_FreeReflectionInfo(IgniteShaderReflectionInfo* reflectionInfo)
    {
        if (!reflectionInfo)
        {
            return;
        }

        FreeResourceArray(reflectionInfo->uniformBuffers, reflectionInfo->numUniformBuffers);
        FreeResourceArray(reflectionInfo->sampledImages, reflectionInfo->numSamplers);
        FreeResourceArray(reflectionInfo->storageImages, reflectionInfo->numStorageTextures);
        FreeResourceArray(reflectionInfo->storageBuffers, reflectionInfo->numStorageBuffers);
        FreeResourceArray(reflectionInfo->separateSamplers, reflectionInfo->numSeparateSamplers);
        FreeResourceArray(reflectionInfo->separateImages, reflectionInfo->numSeparateImages);
        FreePushConstantArray(reflectionInfo->pushConstants, reflectionInfo->numPushConstants);
        FreeStageIOArray(reflectionInfo->stageInputs, reflectionInfo->numStageInputs);
        FreeStageIOArray(reflectionInfo->stageOutputs, reflectionInfo->numStageOutputs);
        FreeVertexAttributeArray(reflectionInfo->vertexAttributes, reflectionInfo->vertexAttributeCount);

        std::memset(reflectionInfo, 0, sizeof(*reflectionInfo));
    }
}
