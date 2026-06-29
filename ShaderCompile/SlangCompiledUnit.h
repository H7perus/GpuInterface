#pragma once


#include "vulkan/vulkan.hpp"

#include "slang-com-ptr.h"
#include "slang.h"


#include "../Types/Device.h"


struct VertexInput
{
    std::string semanticName;
    u32         semanticIndex;
    vk::Format  format;
    u32         location;
};

struct PushConstantParameter
{
    std::string name;
    std::string type;
    u32         offset;
};

// just supporting int right now
struct CompileTimeConstant
{
    std::string name;
    int         def = 0;
    int         min = -1;
    int         max = -1;
};

// Intent: Maintain the composed program and provide helpers for reflection etc.

class SlangCompiledUnit
{
  public:
    SlangCompiledUnit(Slang::ComPtr<slang::IComponentType> program) : composedProgram(program) {};

    SlangCompiledUnit(Slang::ComPtr<slang::IModule> module) : module(module)
    {
        int entryCount = module->getDefinedEntryPointCount();
        for (int i = 0; i < entryCount; i++)
        {
            entryPoints.push_back(Slang::ComPtr<slang::IEntryPoint>());
            module->getDefinedEntryPoint(i, entryPoints[i].writeRef());
        }

    };


    Slang::ComPtr<slang::IComponentType> composedProgram;

    Slang::ComPtr<slang::IModule>                  module;
    std::vector<Slang::ComPtr<slang::IEntryPoint>> entryPoints;


    Slang::ComPtr<slang::IBlob> getTargetCode()
    {
        Slang::ComPtr<slang::IBlob> blob;
        composedProgram->getTargetCode(0, blob.writeRef());
        return blob;
    }

    // TODO:
    //  1. Reflection for vertex input

    std::optional<std::vector<VertexInput>> ReflectOnVertexInput()
    {
        std::vector<VertexInput> result;

        auto layout = module->getLayout(); // entry points will be present now


        auto globalParamVarLayout = layout->getGlobalParamsTypeLayout();

        auto countTest = layout->getParameterByIndex(0);

        auto kindTest = countTest->getType()->getKind();

        SlangUInt entryPointCount = layout->getEntryPointCount();

        if (entryPointCount == 0)
        {
            return std::optional<std::vector<VertexInput>>();
        }

        slang::EntryPointLayout* entryPoint = layout->getEntryPointByIndex(0);

        SlangUInt paramCount = entryPoint->getParameterCount();

        for (SlangUInt i = 0; i < paramCount; i++)
        {
            slang::VariableLayoutReflection* param = entryPoint->getParameterByIndex(i);

            const char*                  name       = param->getName();
            slang::TypeLayoutReflection* typeLayout = param->getTypeLayout();
            slang::TypeReflection::Kind  kind       = typeLayout->getType()->getKind();

            if (kind == slang::TypeReflection::Kind::Struct)
            {
                if (paramCount > 1)
                {
                    return std::optional<std::vector<VertexInput>>();
                }

                int elementCount = typeLayout->getFieldCount();
                for (int eIdx = 0; eIdx < elementCount; eIdx++)
                {
                    slang::VariableLayoutReflection* fieldLayoutRefl = typeLayout->getFieldByIndex(eIdx);

                    result.push_back(GetVertexParameterReflection(fieldLayoutRefl));
                }

                return std::optional<std::vector<VertexInput>>(result);
            }

            result.push_back(GetVertexParameterReflection(param));
        }
        return std::optional<std::vector<VertexInput>>(result);
    }

  private:
    VertexInput GetVertexParameterReflection(slang::VariableLayoutReflection* vertexParamRefl)
    {
        const char* semantic    = vertexParamRefl->getSemanticName();
        u32         semanticIdx = vertexParamRefl->getSemanticIndex();
        u32         location    = vertexParamRefl->getBindingIndex();

        slang::TypeReflection* typeRefl   = vertexParamRefl->getType();
        auto                   scalarType = typeRefl->getScalarType();

        vk::Format format = vk::Format::eR32Sfloat;

        switch (scalarType)
        {
        case slang::TypeReflection::Float32:
            format = vk::Format::eR32Sfloat;
            break;
        case slang::TypeReflection::Int32:
            format = vk::Format::eR32Sint;
            break;
        default:
            // TODO: We need to handle errors like this. We need to tell the programmer that their format is unsupported
        }

        int elementCount = std::max((size_t)1, typeRefl->getElementCount());

        format = vk::Format(int(format) + (elementCount - 1) * 3);

        return {.semanticName = semantic, .semanticIndex = semanticIdx, .format = format, .location = location};
    }

  public:
    // 2. Reflection for push constants.

    std::optional<std::vector<PushConstantParameter>> ReflectOnPushConstants()
    {
        std::vector<PushConstantParameter> result;

        auto layout = module->getLayout(); // entry points will be present now


        auto globalParamVarLayout = layout->getGlobalParamsTypeLayout();


        u32 globalParamCount = layout->getParameterCount();

        bool hasFoundPushConstants = false;

        for (int paramIdx = 0; paramIdx < globalParamCount; paramIdx++)
        {
            auto varReflection = layout->getParameterByIndex(paramIdx);
            auto kindOfParam   = varReflection->getTypeLayout()->getParameterCategory();

            if (kindOfParam == slang::ParameterCategory::PushConstantBuffer)
            {
                if (hasFoundPushConstants)
                {
                    // TODO: This really needs some error handling. Its a bit whack rn.
                    return std::optional<std::vector<PushConstantParameter>>();
                }
                hasFoundPushConstants = true;
            }
            auto elementLayout = varReflection->getTypeLayout()->getElementTypeLayout();
            auto constantCount = elementLayout->getFieldCount();

            for (int constantIdx = 0; constantIdx < constantCount; constantIdx++)
            {
                auto constantVarRefl    = elementLayout->getFieldByIndex(constantIdx);
                auto constantTypeLayout = constantVarRefl->getTypeLayout();

                auto kind = constantTypeLayout->getKind();


                std::string typeString;
                if (kind == slang::TypeReflection::Kind::Vector)
                {
                    typeString = constantTypeLayout->getElementTypeLayout()->getName() +
                                 std::to_string(constantTypeLayout->getElementCount());
                }
                else
                {
                    typeString = constantTypeLayout->getType()->getName();
                }


                u32         offset = constantVarRefl->getOffset();
                std::string name   = constantVarRefl->getName();

                PushConstantParameter param = {.name = name, .type = typeString, .offset = offset};

                result.push_back(param);
            }
        }
        return result;
    }


    // 3. Reflection for set constants (extern const int)
    std::optional<std::vector<CompileTimeConstant>> ReflectOnCompileTimeConstants()
    {
        std::vector<CompileTimeConstant> result;
        int constantCount = module->getModuleReflection()->getChildrenCount();

        for(int i = 0; i < constantCount; i++)
        {
            auto decl = module->getModuleReflection()->getChild(i);

            if(decl->getKind() != slang::DeclReflection::Kind::Variable) continue;

            slang::VariableReflection *var = decl->asVariable();
            if(var == nullptr || !var->findModifier(slang::Modifier::Extern)) continue;

            i64 defaultValue;

            var->getDefaultValueInt(&defaultValue);

            int min = -1, max = -1;

            for(int attributeIdx = 0; attributeIdx < var->getUserAttributeCount(); attributeIdx++)
            {
                auto attribute = var->getUserAttributeByIndex(attributeIdx);

                std::string attributeName = attribute->getName();
                if(!strcmp(attribute->getName(),"ConstantRange"))
                {
                    attribute->getArgumentValueInt(0, &min);
                    attribute->getArgumentValueInt(1, &max);
                }
            }
            

            result.push_back(
                {
                    .name = var->getName(),
                    .def  = (i32)defaultValue,
                    .min = min,
                    .max = max
                }
            );
        }

        return result;
    }

    // uses reflection to provide the correct descriptor set layout

    std::pair<vk::DescriptorSetLayout, std::vector<vk::DescriptorSetLayoutBinding>> createDescriptorSetLayout(
        const vk::Device& device)
    {
        auto layout = composedProgram->getLayout();
        // auto layout = composedProgram->getLayout();
        auto typeLayout = layout->getGlobalParamsTypeLayout();

        const int fieldCount = typeLayout->getFieldCount();

        std::vector<vk::DescriptorSetLayoutBinding> bindings(fieldCount);

        for (int i = 0; i < fieldCount; i++)
        {
            slang::VariableLayoutReflection* field = typeLayout->getFieldByIndex(i);

            slang::TypeLayoutReflection* typeLayout = field->getTypeLayout();
            slang::TypeReflection*       type       = typeLayout->getType();

            bindings[i].binding = field->getBindingIndex();

            bindings[i].descriptorCount    = 1;
            bindings[i].stageFlags         = vk::ShaderStageFlagBits::eAll;
            bindings[i].pImmutableSamplers = nullptr;

            switch (type->getKind())
            {
            case slang::TypeReflection::Kind::ConstantBuffer:
            {
                bindings[i].descriptorType = vk::DescriptorType::eUniformBuffer;
                break;
            }
            case slang::TypeReflection::Kind::Resource:
            {
                SlangResourceShape shape = type->getResourceShape();
                // unused as of now. Vulkan dgaf
                SlangResourceAccess access = type->getResourceAccess();

                if (shape == SLANG_STRUCTURED_BUFFER)
                {
                    bindings[i].descriptorType = vk::DescriptorType::eStorageBuffer;
                }
                else if (shape & SLANG_TEXTURE_COMBINED_FLAG)
                {
                    bindings[i].descriptorType = vk::DescriptorType::eCombinedImageSampler;
                }
                break;
            }
            }
        }

        vk::DescriptorSetLayoutCreateInfo layoutInfo;
        layoutInfo.bindingCount = bindings.size();
        layoutInfo.pBindings    = bindings.data();

        return std::pair(device.createDescriptorSetLayout(layoutInfo), bindings);
    }
};