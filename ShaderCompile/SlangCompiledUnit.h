#pragma once


#include "vulkan/vulkan.hpp"

#include "slang-com-ptr.h"
#include "slang.h"


#include "../Types/Device.h"
#include "GpuInterface/ShaderCompile/ShaderReflection.h"
#include "GpuInterface/ShaderCompile/SpecialisedShader.h"
#include <algorithm>

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
        if (!module)
            composedProgram->getTargetCode(0, blob.writeRef());
        else
        {
            Slang::ComPtr<slang::IComponentType> composed;

            std::vector<slang::IComponentType*> componentTypes(entryPoints.size() + 1);

            componentTypes[0] = module.get();

            for (int i = 0; i < entryPoints.size(); i++)
            {
                componentTypes[1 + i] = entryPoints[i].get();
            }

            // not using diagnostics here for now. Might become necessary in the future.
            SlangResult result = module->getSession()->createCompositeComponentType(
                componentTypes.data(), componentTypes.size(), composed.writeRef());

            composed->getTargetCode(0, blob.writeRef());
        }

        return blob;
    }

    std::optional<std::vector<KE::VK::VertexInput>> GetVertexInputReflection()
    {


        // not using diagnostics here for now. Might become necessary in the future.

        if (composedProgram == nullptr)
        {
            std::vector<slang::IComponentType*> componentTypes(entryPoints.size() + 1);
            for (int i = 0; i < entryPoints.size(); i++)
            {
                componentTypes[1 + i] = entryPoints[i].get();
            }
            componentTypes[0]         = module.get();
            SlangResult resultCompose = module->getSession()->createCompositeComponentType(
                componentTypes.data(), componentTypes.size(), composedProgram.writeRef());
        }
        std::vector<KE::VK::VertexInput> result;

        auto layout = composedProgram->getLayout(); // entry points will be present now


        auto globalParamVarLayout = layout->getGlobalParamsTypeLayout();

        auto countTest = layout->getParameterByIndex(0);

        auto kindTest = countTest->getType()->getKind();

        SlangUInt entryPointCount = layout->getEntryPointCount();

        if (entryPointCount == 0)
        {
            return std::optional<std::vector<KE::VK::VertexInput>>();
        }

        // big fucking assumption that it is index 0
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
                    return std::optional<std::vector<KE::VK::VertexInput>>();
                }

                int elementCount = typeLayout->getFieldCount();
                for (int eIdx = 0; eIdx < elementCount; eIdx++)
                {
                    slang::VariableLayoutReflection* fieldLayoutRefl = typeLayout->getFieldByIndex(eIdx);

                    result.push_back(GetVertexParameterReflection(fieldLayoutRefl));
                }

                return std::optional<std::vector<KE::VK::VertexInput>>(result);
            }

            result.push_back(GetVertexParameterReflection(param));
        }
        return std::optional<std::vector<KE::VK::VertexInput>>(result);
    }

  private:
    KE::VK::VertexInput GetVertexParameterReflection(slang::VariableLayoutReflection* vertexParamRefl)
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
    std::optional<std::vector<KE::VK::StructMember>> GetPushConstantReflection()
    {
        std::vector<KE::VK::StructMember> result;

        slang::ProgramLayout* layout;

        if (module == nullptr)
            layout = composedProgram->getLayout();
        else
            layout = module->getLayout();


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
                    return std::optional<std::vector<KE::VK::StructMember>>();
                }
                hasFoundPushConstants = true;
            }
            auto elementLayout = varReflection->getTypeLayout()->getElementTypeLayout();
            auto constantCount = elementLayout->getFieldCount();

            for (int constantIdx = 0; constantIdx < constantCount; constantIdx++)
            {
                auto constantVarRefl    = elementLayout->getFieldByIndex(constantIdx);
                auto constantTypeLayout = constantVarRefl->getTypeLayout();

                auto kind = constantTypeLayout->getType()->getKind();


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

                KE::VK::StructMember param = {.name = name, .type = typeString, .offset = offset};

                result.push_back(param);
            }
        }
        return result;
    }

    // OVERHAUL: This is a rather temporary solution
    std::optional<std::vector<KE::VK::StructMember>> GetMaterialParameterReflection()
    {
        std::vector<KE::VK::StructMember> result;
        bool                              hasMaterialParamStruct = false;

        slang::TypeLayoutReflection* globalVarLayout;
        if (module == nullptr)
            globalVarLayout = composedProgram->getLayout()->getGlobalParamsTypeLayout();
        else
            globalVarLayout = module->getLayout()->getGlobalParamsTypeLayout();

        auto globalParamsFieldCount = globalVarLayout->getFieldCount();

        for (int paramIdx = 0; paramIdx < globalParamsFieldCount; paramIdx++)
        {
            auto varLayout = globalVarLayout->getFieldByIndex(paramIdx);

            if (strcmp(varLayout->getName(), "refl_MaterialParameters"))
                continue;

            auto typeLayout = varLayout->getTypeLayout();

            if (typeLayout->getKind() != slang::TypeReflection::Kind::ConstantBuffer)
                return std::optional<std::vector<KE::VK::StructMember>>();


            auto matParamLayout = typeLayout->getElementTypeLayout();

            int matParamCount = matParamLayout->getFieldCount();

            for (int matParamIdx = 0; matParamIdx < matParamCount; matParamIdx++)
            {
                auto        matParam = matParamLayout->getFieldByIndex(matParamIdx);
                u32         offset   = matParam->getOffset();
                std::string name     = matParam->getName();

                // auto testKind = matParam->getType()->getElementType()->getName();

                std::string typeString;
                if (matParam->getType()->getKind() == slang::TypeReflection::Kind::Vector)
                {
                    typeString = matParam->getTypeLayout()->getElementTypeLayout()->getName() +
                                 std::to_string(matParam->getTypeLayout()->getElementCount());
                }
                else
                {
                    typeString = matParam->getType()->getName();
                }
                result.push_back({.name = name, .type = typeString, .offset = offset});
            }
        }

        return result;
    }

    std::optional<std::vector<KE::VK::CompileTimeConstant>> GetCompileTimeConstantReflection()
    {
        std::vector<KE::VK::CompileTimeConstant> result;
        int                                      constantCount = module->getModuleReflection()->getChildrenCount();

        for (int i = 0; i < constantCount; i++)
        {
            auto decl = module->getModuleReflection()->getChild(i);


            if (decl->getKind() != slang::DeclReflection::Kind::Variable)
                continue;

            slang::VariableReflection* var = decl->asVariable();


            if (var == nullptr || !var->findModifier(slang::Modifier::Extern))
                continue;

            i64 defaultValue;

            var->getDefaultValueInt(&defaultValue);

            int min = -1, max = -1;

            for (int attributeIdx = 0; attributeIdx < var->getUserAttributeCount(); attributeIdx++)
            {
                auto attribute = var->getUserAttributeByIndex(attributeIdx);

                std::string attributeName = attribute->getName();
                if (!strcmp(attribute->getName(), "ConstantRange"))
                {
                    attribute->getArgumentValueInt(0, &min);
                    attribute->getArgumentValueInt(1, &max);
                }
            }


            result.push_back({.name = var->getName(), .def = (i32)defaultValue, .min = min, .max = max});
        }

        return result;
    }

    KE::VK::SpecializedShader Specialise(
        Slang::ComPtr<slang::ISession>                session        = nullptr,
        std::vector<KE::VK::CompileTimeConstantValue> constantValues = std::vector<KE::VK::CompileTimeConstantValue>())
    {
        KE::VK::SpecializedShader specialisedShader;

        auto compileTimeConstantsAvailable = GetCompileTimeConstantReflection();


        std::vector<KE::VK::CompileTimeConstantValue> defaultCompileTimeConstantValues;

        if (compileTimeConstantsAvailable.has_value())
        {
            std::sort(compileTimeConstantsAvailable.value().begin(), compileTimeConstantsAvailable.value().end(),
                      [](const KE::VK::CompileTimeConstant& a, const KE::VK::CompileTimeConstant& b)
                      { return a.name < b.name; });

            for (auto& constant : compileTimeConstantsAvailable.value())
            {
                defaultCompileTimeConstantValues.push_back({.name = constant.name, .value = constant.def});
            }
        }


        SlangCompiledUnit* unit;
        if (constantValues.size() == 0)
        {
            unit = this;
        }
        else
        {
            // sort first. This makes the hash of the constant values and the path consistent. Although I'd expect
            // tooling to keep it sorted. Easier for manual material edits though.
            std::sort(constantValues.begin(), constantValues.end(),
                      [](const KE::VK::CompileTimeConstantValue& a, const KE::VK::CompileTimeConstantValue& b)
                      { return a.name < b.name; });


            i32 defaultsIndex        = 0;
            i32 passedConstantsIndex = 0;

            while (defaultsIndex < defaultCompileTimeConstantValues.size() &&
                   passedConstantsIndex < constantValues.size())
            {
                if (constantValues[passedConstantsIndex].name == defaultCompileTimeConstantValues[defaultsIndex].name)
                {
                    auto val = constantValues[passedConstantsIndex].value;
                    auto min = compileTimeConstantsAvailable.value()[defaultsIndex].min;
                    auto max = compileTimeConstantsAvailable.value()[defaultsIndex].max;
                    i32  clamped;

                    if (clamped = std::clamp(val, min, max); val != clamped)
                    {
                        std::cout << "Shader Specialisation: passed constant value not in range!"
                                  << std::endl; // we would throw a warning here;
                    }
                    defaultCompileTimeConstantValues[defaultsIndex].value = clamped;

                    passedConstantsIndex++;
                }
                defaultsIndex++;


                if (defaultsIndex + 1 == defaultCompileTimeConstantValues.size() &&
                    passedConstantsIndex < constantValues.size())
                {
                    throw std::logic_error("Passed constant(s) have no equivalent in the shader!");
                }
            }


            std::string specialisationModuleString;
            for (auto& constantValue : defaultCompileTimeConstantValues)
            {
                specialisationModuleString += "export static const int ";
                specialisationModuleString += constantValue.name;
                specialisationModuleString += " = ";
                specialisationModuleString += std::to_string(constantValue.value);
                specialisationModuleString += ";\n";
            }

            Slang::ComPtr<slang::IComponentType> specialisedProgram;

            Slang::ComPtr<slang::IBlob> diagnosticsBlob;

            std::array<slang::IComponentType*, 2> componentTypes;


            Slang::ComPtr<slang::IModule> specialisationModule;
            specialisationModule = session->loadModuleFromSourceString(specialisationModuleString.c_str(), nullptr,
                                                                       specialisationModuleString.c_str());

            componentTypes[0] = module;
            componentTypes[1] = specialisationModule;

            session->createCompositeComponentType(componentTypes.data(), componentTypes.size(),
                                                  specialisedProgram.writeRef(), diagnosticsBlob.writeRef());

            unit = new SlangCompiledUnit(specialisedProgram);
        }


        if (auto vertexInputs = unit->GetVertexInputReflection(); vertexInputs.has_value())
        {
            specialisedShader.vertexInputs = vertexInputs.value();
        }
        if (auto pushConstantRefl = unit->GetPushConstantReflection(); pushConstantRefl.has_value())
        {
            specialisedShader.pushConstants = pushConstantRefl.value();
        }
        if (auto materialRefl = unit->GetMaterialParameterReflection(); materialRefl.has_value())
        {
            specialisedShader.materialParameters = unit->GetMaterialParameterReflection().value();
        }

        specialisedShader.compileTimeConstants = defaultCompileTimeConstantValues;

        auto spirvBlob = unit->getTargetCode();

        specialisedShader.spirv.resize(spirvBlob->getBufferSize() / 4);

        std::memcpy(specialisedShader.spirv.data(), spirvBlob->getBufferPointer(), spirvBlob->getBufferSize());

        if (constantValues.size() != 0)
        {
            delete unit;
        }

        return specialisedShader;
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