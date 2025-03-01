#include <geodesy/core/gcl/shader.h>

#include "glslang_util.h"

#include <iostream>

namespace geodesy::core::gcl {

	static struct {
		shader::stage Stage;
		std::vector<std::string> Name;
	} ShaderExtensionDataBase[] = {
		{ shader::stage::VERTEX						, { "vsh", "vert" } },
		{ shader::stage::TESSELLATION_CONTROL		, { "tcsh" } },
		{ shader::stage::TESSELLATION_EVALUATION	, { "tesh" } },
		{ shader::stage::GEOMETRY					, { "gsh" } },
		{ shader::stage::PIXEL						, { "psh", "fsh", "frag" } },
		{ shader::stage::COMPUTE					, { "csh" } },
		{ shader::stage::RAYGEN						, { "rgsh" } },
		{ shader::stage::ANY_HIT					, { "ahsh" } },
		{ shader::stage::CLOSEST_HIT				, { "chsh" } },
		{ shader::stage::MISS						, { "msh" } },
		{ shader::stage::INTERSECTION				, { "ish" } },
		{ shader::stage::CALLABLE					, { "cash" } }
	};

	// Convert string extension to shader stage.
	static shader::stage extension_to_stage(std::string aExtension) {
		for (size_t i = 0; i < sizeof(ShaderExtensionDataBase) / sizeof(ShaderExtensionDataBase[0]); i++) {
			for (size_t j = 0; j < ShaderExtensionDataBase[i].Name.size(); j++) {
				if (aExtension == ShaderExtensionDataBase[i].Name[j]) {
					return ShaderExtensionDataBase[i].Stage;
				}
			}
		}
		return shader::stage::UNKNOWN;
	}

	shader::shader() {
		this->Stage			= shader::stage::UNKNOWN;
		this->Handle		= nullptr;
	}

	shader::shader(std::string aFilePath) : file(aFilePath) {
		this->Stage			= shader::stage::UNKNOWN;
		this->Handle		= nullptr;

		// Load Source File
		this->read(aFilePath);

		// Determine Stage and compile to SPIRV.
		this->Stage = extension_to_stage(this->Extension);

		// Compile and generate SPIRV.
		this->compile_source(this->Stage, (const char*)this->HostData);
	}

	shader::shader(stage aShaderStage, std::string aSourceCode) : shader() {
		// Determine Stage and compile to SPIRV.
		this->Stage = aShaderStage;

		// Compile and generate SPIRV.
		this->compile_source(this->Stage, aSourceCode);
	}

	VkShaderStageFlagBits shader::get_stage() {
		return (VkShaderStageFlagBits)this->Stage;
	}

	VkPipelineShaderStageCreateInfo shader::pipeline_shader_stage_create_info() {
		VkPipelineShaderStageCreateInfo Temp;
		Temp.sType					= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		Temp.pNext					= NULL;
		Temp.flags					= 0;
		Temp.stage					= this->get_stage();
		Temp.module					= VK_NULL_HANDLE;
		Temp.pName					= "main";  // Entry Point function name
		Temp.pSpecializationInfo	= NULL;
		return Temp;
	}

	bool shader::initialize() {
		return glslang::InitializeProcess();
	}

	void shader::terminate() {
		glslang::FinalizeProcess();
	}

	bool shader::compile_source(stage aShaderStage, std::string aSourceCode) {
		EShMessages Options = (EShMessages)(
			EShMessages::EShMsgAST | 
			EShMessages::EShMsgSpvRules | 
			EShMessages::EShMsgVulkanRules | 
			EShMessages::EShMsgDebugInfo | 
			EShMessages::EShMsgBuiltinSymbolTable
		);

		EShLanguage ShaderStage 									= vulkan_to_glslang((VkShaderStageFlagBits)aShaderStage);
		glslang::EShSource Source									= glslang::EShSource::EShSourceGlsl;
		glslang::EShClient Client									= glslang::EShClient::EShClientVulkan;
		int ClientInputSemanticsVersion								= 100;
		const int DefaultVersion									= 100;
		glslang::EShTargetClientVersion ClientVersion				= glslang::EShTargetClientVersion::EShTargetVulkan_1_0;
		glslang::EShTargetLanguage TargetLanguage					= glslang::EShTargetLanguage::EShTargetSpv;
		glslang::EShTargetLanguageVersion TargetLanguageVersion		= glslang::EShTargetLanguageVersion::EShTargetSpv_1_0;

		std::vector<const char*> SourceCode = { aSourceCode.c_str() };

		// Setup
		this->Handle = std::make_shared<glslang::TShader>(ShaderStage);
		this->Handle->setStrings(SourceCode.data(), SourceCode.size());
		this->Handle->setEnvInput(Source, ShaderStage, Client, ClientInputSemanticsVersion);
		this->Handle->setEnvClient(Client, ClientVersion);
		this->Handle->setEnvTarget(TargetLanguage, TargetLanguageVersion);
		this->Handle->setEntryPoint("main");

		// this->Handle.preprocess(&glslang::DefaultTBuiltInResource, DefaultVersion, ENoProfile, false, false, Options, NULL);
		bool CompilationSuccess = this->Handle->parse(&glslang::DefaultTBuiltInResource, DefaultVersion, false, Options);
		if (!CompilationSuccess) {
			std::cout << this->Name << "." << this->Extension << std::endl;
			std::cout << this->Handle->getInfoLog() << std::endl;
		}

		return CompilationSuccess;
	}

}
