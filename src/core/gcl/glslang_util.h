#pragma once
#ifndef GEODESY_CORE_GCL_GLSLANG_UTIL_H
#define GEODESY_CORE_GCL_GLSLANG_UTIL_H

#include <assert.h>

#include <geodesy/core/gcl/config.h>

//#include <glslang-src/glslang/Include/arrays.h>
#include <glslang-src/glslang/Include/BaseTypes.h>
#include <glslang-src/glslang/Include/Common.h>
#include <glslang-src/glslang/Include/ConstantUnion.h>
#include <glslang-src/glslang/Include/intermediate.h>
#include <glslang-src/glslang/Include/PoolAlloc.h>
#include <glslang-src/glslang/Include/ResourceLimits.h>
#include <glslang-src/glslang/Include/SpirvIntrinsics.h>
#include <glslang-src/glslang/Include/Types.h>

#include <glslang-src/glslang/MachineIndependent/localintermediate.h>

#include <glslang-src/glslang/Public/ShaderLang.h>

// Converts shader source into SPIRV.
//#include <glslang-src/SPIRV/SPIRV/SpvTools.h>
//#include <glslang-src/SPIRV/SPIRV/Logger.h>
#include <glslang-src/SPIRV/GlslangToSpv.h>
//#include <glslang-src/SPIRV/SPIRV/spirv.hpp>
//#include <glslang-src/SPIRV/SPIRV/spvIR.h>
//#include <glslang-src/SPIRV/SPIRV/SPVRemapper.h>

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
