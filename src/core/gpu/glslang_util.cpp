#include "glslang_util.h"

namespace geodesy::core::gpu {

	using namespace util;

	EShLanguage vulkan_to_glslang(VkShaderStageFlagBits aStage) {
		switch (aStage) {
		case VK_SHADER_STAGE_VERTEX_BIT:
			return EShLangVertex;
		case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
			return EShLangTessControl;
		case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
			return EShLangTessEvaluation;
		case VK_SHADER_STAGE_GEOMETRY_BIT:
			return EShLangGeometry;
		case VK_SHADER_STAGE_FRAGMENT_BIT:
			return EShLangFragment;
		case VK_SHADER_STAGE_COMPUTE_BIT:
			return EShLangCompute;
		case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
			return EShLangRayGen;
		case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
			return EShLangAnyHit;
		case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
			return EShLangClosestHit;
		case VK_SHADER_STAGE_MISS_BIT_KHR:
			return EShLangMiss;
		case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:
			return EShLangIntersect;
		case VK_SHADER_STAGE_CALLABLE_BIT_KHR:
			return EShLangCallable;
		default:
			return EShLangCount;
		}
	}

	VkShaderStageFlagBits vulkan_to_glslang(EShLanguage aStage) {
		switch (aStage) {
		case EShLangVertex:
			return VK_SHADER_STAGE_VERTEX_BIT;
		case EShLangTessControl:
			return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		case EShLangTessEvaluation:
			return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		case EShLangGeometry:
			return VK_SHADER_STAGE_GEOMETRY_BIT;
		case EShLangFragment:
			return VK_SHADER_STAGE_FRAGMENT_BIT;
		case EShLangCompute:
			return VK_SHADER_STAGE_COMPUTE_BIT;
		case EShLangRayGen:
			return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		case EShLangAnyHit:
			return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
		case EShLangClosestHit:
			return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		case EShLangMiss:
			return VK_SHADER_STAGE_MISS_BIT_KHR;
		case EShLangIntersect:
			return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
		case EShLangCallable:
			return VK_SHADER_STAGE_CALLABLE_BIT_KHR;
		default:
			return VK_SHADER_STAGE_ALL;
		}
	}

	VkShaderStageFlags glslang_shader_stage_to_vulkan(EShLanguageMask aShaderStage) {
		VkShaderStageFlags VulkanShaderStage = 0;
		if (aShaderStage & EShLangVertexMask) {
			VulkanShaderStage |= VK_SHADER_STAGE_VERTEX_BIT;
		}
		if (aShaderStage & EShLangTessControlMask) {
			VulkanShaderStage |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		}
		if (aShaderStage & EShLangTessEvaluationMask) {
			VulkanShaderStage |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		}
		if (aShaderStage & EShLangGeometryMask) {
			VulkanShaderStage |= VK_SHADER_STAGE_GEOMETRY_BIT;
		}
		if (aShaderStage & EShLangFragmentMask) {
			VulkanShaderStage |= VK_SHADER_STAGE_FRAGMENT_BIT;
		}
		if (aShaderStage & EShLangComputeMask) {
			VulkanShaderStage |= VK_SHADER_STAGE_COMPUTE_BIT;
		}
		return VulkanShaderStage;
	}

	variable convert_to_variable(const glslang::TType* aType, const char* aName) {
		variable Variable;
		glslang::TBasicType BasicType = aType->getBasicType();
		if ((BasicType == glslang::TBasicType::EbtBlock) || (BasicType == glslang::TBasicType::EbtStruct)) {
			const glslang::TTypeList* TypeList = aType->getStruct();
			std::vector<variable> VariableList = std::vector<variable>(TypeList->size());
			for (size_t i = 0; i < VariableList.size(); i++) {
				glslang::TType* MemberType = (*TypeList)[i].type;
				VariableList[i] = convert_to_variable(MemberType, MemberType->getFieldName().c_str());
			}
			// TODO: Figure out how to get name space
			Variable = variable(type(type::id::STRUCT, aType->getTypeName().c_str(), VariableList), aName);
		}
		else if (BasicType == glslang::TBasicType::EbtSampler) {
			type::id ID;
			switch (aType->getSampler().getBasicType()) {
			default:
				ID = type::id::UNKNOWN;
				break;
			case glslang::TBasicType::EbtFloat:
				ID = type::id::FLOAT;
				break;
			case glslang::TBasicType::EbtDouble:
				ID = type::id::DOUBLE;
				break;
				//case glslang::TBasicType::EbtFloat16:
				//	ID = type::id::HALF;
				//	break;
			case glslang::TBasicType::EbtInt8:
				ID = type::id::CHAR;
				break;
			case glslang::TBasicType::EbtUint8:
				ID = type::id::UCHAR;
				break;
			case glslang::TBasicType::EbtInt16:
				ID = type::id::SHORT;
				break;
			case glslang::TBasicType::EbtUint16:
				ID = type::id::USHORT;
				break;
			case glslang::TBasicType::EbtInt:
				ID = type::id::INT;
				break;
			case glslang::TBasicType::EbtUint:
				ID = type::id::UINT;
				break;
				//case glslang::TBasicType::EbtInt64:
				//	break;
				//case glslang::TBasicType::EbtUint64:
				//	break;
				//case glslang::TBasicType::EbtBool:
				//	break;
			}

			std::string TypeName = type::name_of(ID) + std::to_string(aType->getSampler().vectorSize);

			ID = type::id_of_string(TypeName.c_str());

			Variable = variable(ID, aName);
		}
		else {
			std::string TypeName = "";
			type::id TypeID = type::id::UNKNOWN;
			std::vector<int> ArraySize;

			TypeName = aType->getBasicString();
			if (aType->isVector()) {
				TypeName += std::to_string(aType->getVectorSize());
			}
			if (aType->isMatrix()) {
				TypeName += std::to_string(aType->getMatrixRows()) + "x" + std::to_string(aType->getMatrixCols());
			}

			if (aType->isArray()) {
				const glslang::TArraySizes* lmao = aType->getArraySizes();
				for (int j = 0; j < lmao->getNumDims(); j++) {
					ArraySize.push_back(lmao->getDimSize(j));
				}
			}

			TypeID = type::id_of_string(TypeName.c_str());
			Variable = variable(TypeID, aName, ArraySize);
		}
		return Variable;
	}

}