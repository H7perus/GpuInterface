#pragma once
#include "slang.h"
#include "slang-com-ptr.h"
#include "slang-com-helper.h"

#include "ShaderCompile.h"
#include "SlangCompiledUnit.h"

#include "GpuInterface.h"

#include <iostream>
#include <iomanip>
#include <vector>
#include <array>

namespace KE::VK
{
	class SlangCompileContext
	{
		//TODO: global session should be split, so that a compute compiler and a graphics compiler can share the same global session. Anyway, problem for future me!
		Slang::ComPtr<slang::IGlobalSession> globalSession;

		Slang::ComPtr<slang::ISession> session;
		slang::SessionDesc sessionDesc;

		//not sure if we need multiple
		Slang::ComPtr<slang::IBlob> diagnosticsBlob;

	public:
		SlangCompileContext()
		{
			createGlobalSession(globalSession.writeRef());

			//Assumption: SPIR-V only!
			slang::TargetDesc targetDesc = {};
			targetDesc.format = SLANG_SPIRV;
			targetDesc.profile = globalSession->findProfile("spirv_1_6");

			std::cout << std::hex;
			u8* ptr = (u8*)(&targetDesc);
			for (int i = 0; i < 24; i++)
			{
				std::cout << std::setw(2) << std::setfill('0')
					<< std::hex << (int)ptr[i] << " ";

			}
			std::cout << std::endl;
			SlangUUID uuid = SlangUUID(slang::SLANG_UUID_IMetadata);

			ptr = (u8*)(&uuid);
			for (int i = 0; i < 16; i++)
			{
				std::cout << std::setw(2) << std::setfill('0')
					<< std::hex << (int)ptr[i] << " ";

			}

			std::cout << std::dec << std::endl;

			

			sessionDesc.targetCount = 1;
			sessionDesc.targets = &targetDesc;


			std::cout << "target profile: " << targetDesc.profile << std::endl;

			sessionDesc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;

			std::array<slang::CompilerOptionEntry, 3> options =
			{ {
				{
					slang::CompilerOptionName::EmitSpirvDirectly,
					{slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr}
				},
				{
					slang::CompilerOptionName::Capability,
					{slang::CompilerOptionValueKind::String, 1, 0, "spvDescriptorHeapEXT", nullptr}

				},
				{ //Hardcoded for now. This might not play nicely with AMD GPUs, which is a bit annoying. If it becomes a serious issue we might have to make it a spirv specialisation constant.
					slang::CompilerOptionName::SPIRVResourceHeapStride,
					{slang::CompilerOptionValueKind::Int, 32, 0, nullptr, nullptr}

				}
			} };

			sessionDesc.compilerOptionEntries = options.data();
			sessionDesc.compilerOptionEntryCount = options.size();


			globalSession->createSession(sessionDesc, session.writeRef());
		}
		
		void LoadModule(char* modulecode)
		{

		}

		void LoadModulePath(char* path)
		{

		}

		void CompileShader(char* shadercode)
		{
			Slang::ComPtr<slang::IModule> shaderModule;


		}

		SlangCompiledUnit CompileShaderPath(const char* path)
		{
			Slang::ComPtr<slang::IBlob> spirvCode;

			Slang::ComPtr<slang::IModule> shaderModule;
			shaderModule = session->loadModule(path, diagnosticsBlob.writeRef()); //loadModuleFromSourceString(0, 0, shaderSource, diagnosticsBlob.writeRef());
			diagnoseIfNeeded(diagnosticsBlob);
			Slang::ComPtr<slang::IComponentType> composedProgram;

			Slang::ComPtr<slang::IBlob> diagnostics;
			Slang::ComPtr<slang::IComponentType> program;


			int entryCount = shaderModule->getDefinedEntryPointCount();
			

			std::vector<slang::IComponentType*> componentTypes(entryCount + 1);

			componentTypes[0] = shaderModule.get();

			for (int i = 0; i < entryCount; i++)
			{
				Slang::ComPtr<slang::IEntryPoint> entryPoint;
				shaderModule->getDefinedEntryPoint(i, entryPoint.writeRef());

				componentTypes[1 + i] = entryPoint.get();
			}

			SlangResult result = session->createCompositeComponentType(
				componentTypes.data(),
				componentTypes.size(),
				composedProgram.writeRef(),
				diagnosticsBlob.writeRef()
			);
			//REFLECT ON BINDINGS
			std::cout << composedProgram->getLayout()->getEntryPointByIndex(0) << std::endl;

			auto layout = composedProgram->getLayout();
			//auto layout = composedProgram->getLayout();
			auto typeLayout = layout->getGlobalParamsTypeLayout();

			int fieldCount = typeLayout->getFieldCount();

			Slang::ComPtr<slang::IMetadata> metadata;
			composedProgram->getEntryPointMetadata(
				0,          // entry point index
				0,          // target index
				metadata.writeRef()
			);

			bool isUsed = false;
			SlangParameterCategory category = SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT;
			unsigned spaceIndex = 0;
			unsigned registerIndex = 0;

			diagnoseIfNeeded(diagnosticsBlob);
			SlangResult result1 = composedProgram->getTargetCode(0, spirvCode.writeRef());

			std::cout << "Compiled " << spirvCode->getBufferSize() << " bytes of SPIR-V" << std::endl;

			return SlangCompiledUnit(composedProgram);
		}
	};
}