#pragma once
#include "slang.h"
#include "slang-com-ptr.h"
#include "slang-com-helper.h"


#include <vector>
#include <array>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <optional>
#include <set>
#include <fstream>
#include <filesystem>

Slang::ComPtr<slang::IGlobalSession> globalSession;
Slang::ComPtr<slang::ISession> session;
slang::SessionDesc sessionDesc;

Slang::ComPtr<slang::IBlob> spirvCode;

static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    std::cout << "Working directory: " << std::filesystem::current_path() << std::endl;

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;

}

void diagnoseIfNeeded(slang::IBlob* diagnosticsBlob)
{
    if (diagnosticsBlob != nullptr)
    {
        std::cout << std::hex << (const char*)diagnosticsBlob->getBufferPointer() << std::endl;
        diagnosticsBlob->Release();
    }
}

void ShaderCompileSetup()
{
	createGlobalSession(globalSession.writeRef());
	slang::TargetDesc targetDesc = {};
	targetDesc.format = SLANG_SPIRV;
	targetDesc.profile = globalSession->findProfile("spirv_1_6");

	sessionDesc.targetCount = 1;
	sessionDesc.targets = &targetDesc;
    
    sessionDesc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;

    std::array<slang::CompilerOptionEntry, 1> options =
    {
        {
            slang::CompilerOptionName::EmitSpirvDirectly,
            {slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr}
        }
    };

    sessionDesc.compilerOptionEntries = options.data();
    sessionDesc.compilerOptionEntryCount = options.size();

	
	globalSession->createSession(sessionDesc, session.writeRef());
}


void CompileShader(std::string path)
{
    auto rawSource = readFile(path);
    rawSource.push_back('\0');
    const char* shaderSource = rawSource.data();

    Slang::ComPtr<slang::IModule> testmod;
    Slang::ComPtr<slang::IModule> slangModule;

    {
        Slang::ComPtr<slang::IBlob> diagnosticsBlob;
        //need this if we have named modules, but we don't :^)
        //const char* moduleName = "combined";
        //const char* modulePath = "combined.slang";
        //testmod = session->loadModule("testmod"); //loadModuleFromSourceString(0, 0, shaderSource, diagnosticsBlob.writeRef());

        slangModule = session->loadModule(path.c_str(), diagnosticsBlob.writeRef()); //loadModuleFromSourceString(0, 0, shaderSource, diagnosticsBlob.writeRef());

        //std::cout << shaderSource << std::endl;
        diagnoseIfNeeded(diagnosticsBlob);
        if (!slangModule)
        {
            std::cerr << "Failed on module!" << std::endl;
            
            return;
        }
    }


    std::array<slang::IComponentType*, 1> componentTypes = { slangModule.get()};
    Slang::ComPtr<slang::IComponentType> composedProgram;



    {
        Slang::ComPtr<slang::IBlob> diagnosticsBlob;

        SlangResult result = session->createCompositeComponentType(
            componentTypes.data(),
            (int)componentTypes.size(),
            composedProgram.writeRef(),
            diagnosticsBlob.writeRef()
        );
        diagnoseIfNeeded(diagnosticsBlob);
        if (SLANG_FAILED(result))
        {
            std::cerr << "Failed to compose component type." << std::endl;
            return;
        }
    }
    {
        auto layout = composedProgram->getLayout();

        auto globalParams = layout->getGlobalParamsTypeLayout();
        
        int fieldCount = globalParams->getFieldCount();
        
        for (unsigned i = 0; i < fieldCount; i++)
        {
            slang::VariableLayoutReflection* field = globalParams->getFieldByIndex(i);

            const char* name = field->getVariable()->getName();
            slang::TypeLayoutReflection* typeLayout = field->getTypeLayout();
            slang::TypeReflection* type = typeLayout->getType();

            // Get binding information
            unsigned binding = field->getBindingIndex();
            unsigned space = field->getBindingSpace();

            // Check what kind of resource this is
            slang::TypeReflection::Kind kind = type->getKind();

            switch (kind) {
            case slang::TypeReflection::Kind::ConstantBuffer:
                printf("ConstantBuffer: %s, binding=%d, space=%d\n", name, binding, space);
                break;

            case slang::TypeReflection::Kind::Resource:
            {
                // Check resource shape/type
                SlangResourceShape shape = type->getResourceShape();
                SlangResourceAccess access = type->getResourceAccess();

                slang::TypeReflection* resourceResultType = type->getResourceResultType();

                if (shape & SLANG_TEXTURE_COMBINED_FLAG && (shape & SLANG_TEXTURE_2D)) {
                    printf("Texture2D: %s, binding=%d, space=%d, type=%s\n", name, binding, space, resourceResultType->getName());
                }
                else if (shape == SLANG_STRUCTURED_BUFFER) {
                    ISlangBlob* fullName;

                    type->getFullName(&fullName);
                    printf("StructuredBuffer: %s, binding=%d, space=%d, type=%s\n", name, binding, space, resourceResultType->getName());
                }
                else
                {
                    printf("resource is neither :(\n");
                }
                break;
            }
            case slang::TypeReflection::Kind::SamplerState:
                printf("SamplerState: %s, binding=%d, space=%d\n", name, binding, space);
                break;

            case slang::TypeReflection::Kind::Struct:
                printf("Uniform/Push Constant: %s\n", name);
                break;
            }
        }
			Slang::ComPtr<slang::IBlob> diagnosticsBlob;
			SlangResult result = composedProgram->getTargetCode(0, spirvCode.writeRef());
        diagnoseIfNeeded(diagnosticsBlob);
        if (SLANG_FAILED(result))
        {
            std::cerr << "Failed to compile to SPIR-V." << std::endl;
            return;
        }
    }
    std::cout << "Compiled " << spirvCode->getBufferSize() << " bytes of SPIR-V" << std::endl;
}