#pragma once
#ifndef GEODESY_CORE_GCL_GLSLANG_UTIL_H
#define GEODESY_CORE_GCL_GLSLANG_UTIL_H

#include <assert.h>

#include <geodesy/core/gcl/config.h>

//#include <glslang/Include/arrays.h>
#include <glslang/Include/BaseTypes.h>
#include <glslang/Include/Common.h>
#include <glslang/Include/ConstantUnion.h>
#include <glslang/Include/intermediate.h>
#include <glslang/Include/PoolAlloc.h>
#include <glslang/Include/ResourceLimits.h>
#include <glslang/Include/SpirvIntrinsics.h>
#include <glslang/Include/Types.h>

#include <glslang/MachineIndependent/localintermediate.h>

#include <glslang/Public/ShaderLang.h>

// Converts shader source into SPIRV.
//#include <glslang/SPIRV/SpvTools.h>
//#include <glslang/SPIRV/Logger.h>
#include <glslang/SPIRV/GlslangToSpv.h>
//#include <glslang/SPIRV/spirv.hpp>
//#include <glslang/SPIRV/spvIR.h>
//#include <glslang/SPIRV/SPVRemapper.h>

// Included for compiling
#include "ResourceLimits.h"

#include <geodesy/core/util/variable.h>

namespace geodesy::core::gcl {

	EShLanguage vulkan_to_glslang(VkShaderStageFlagBits aStage);
	VkShaderStageFlagBits vulkan_to_glslang(EShLanguage aStage);

	VkShaderStageFlags glslang_shader_stage_to_vulkan(EShLanguageMask aShaderStage);
	util::variable convert_to_variable(const glslang::TType* aType, const char* aName);

}

#endif // !GEODESY_CORE_GCL_GLSLANG_UTIL_H
