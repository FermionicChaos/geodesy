#include <geodesy/core/util/log.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

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

//// Included for compiling
//#include "../gpu/ResourceLimits.h"

// Font Loading
#include <ft2build.h>
#include FT_FREETYPE_H

//// Image Loading
//#define FREEIMAGE_LIB
//#include <FreeImage.h>

// Model Loading
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

//// System Audio Interface
//#include <portaudio.h>

// System Window Interface
#include <GLFW/glfw3.h>

//FT_Error;
//PaError

namespace geodesy::core::util {

	const char* log::message::type_to_string(type aType) {
		switch (aType) {
		case INFO:		return "Info";
		case WARNING:	return "Warning";
		case ERROR:		return "Error";
		default: 		return "";
		}
	}

	const char* log::message::code_to_string(code aCode) {
		switch (aCode) {
		case UNKNOWN											: return "Unknown";
		case SUCCESS											: return "Success";
		case NOT_INITIALIZED									: return "not initialized";
		case UNANTICIPATED_HOST_ERROR							: return "unanticipated host error";
		case INVALID_CHANNEL_COUNT								: return "invalid channel count";
		case INVALID_SAMPLE_RATE								: return "invalid sample rate";
		case INVALID_DEVICE										: return "invalid device";
		case INVALID_FLAG										: return "invalid flag";
		case SAMPLE_FORMAT_NOT_SUPPORTED						: return "sample format not supported";
		case BAD_IO_DEVICE_COMBINATION							: return "bad io device combination";
		case INSUFFICIENT_MEMORY								: return "insufficient memory";
		case BUFFER_TOO_BIG										: return "buffer too big";
		case BUFFER_TOO_SMALL									: return "buffer too small";
		case NULL_CALLBACK										: return "null callback";
		case BAD_STREAM_PTR										: return "bad stream ptr";
		case TIMED_OUT											: return "timed out";
		case INTERNAL_ERROR										: return "internal error";
		case DEVICE_UNAVAILABLE									: return "device unavailable";
		case INCOMPATIBLE_HOST_API_SPECIFIC_STREAM_INFO			: return "incompatible host api specific stream info";
		case STREAM_IS_STOPPED									: return "stream is stopped";
		case STREAM_IS_NOT_STOPPED								: return "stream is not stopped";
		case INPUT_OVERFLOWED									: return "input overflowed";
		case OUTPUT_UNDERFLOWED									: return "output underflowed";
		case HOST_API_NOT_FOUND									: return "host api not found";
		case INVALID_HOST_API									: return "invalid host api";
		case CAN_NOT_READ_FROM_A_CALLBACK_STREAM				: return "can not read from a callback stream";
		case CAN_NOT_WRITE_TO_A_CALLBACK_STREAM					: return "can not write to a callback stream";
		case CAN_NOT_READ_FROM_AN_OUTPUT_ONLY_STREAM			: return "can not read from an output only stream";
		case CAN_NOT_WRITE_TO_AN_INPUT_ONLY_STREAM				: return "can not write to an input only stream";
		case INCOMPATIBLE_STREAM_HOST_API						: return "incompatible stream host api";
		case BAD_BUFFER_PTR										: return "bad buffer ptr";
		case NO_CURRENT_CONTEXT									: return "no current context";
		case INVALID_ENUM										: return "invalid enum";
		case INVALID_VALUE										: return "invalid value";
		case OUT_OF_MEMORY										: return "out of memory";
		case API_UNAVAILABLE									: return "api unavailable";
		case VERSION_UNAVAILABLE								: return "version unavailable";
		case PLATFORM_ERROR										: return "platform error";
		case FORMAT_UNAVAILABLE									: return "format unavailable";
		case NO_WINDOW_CONTEXT									: return "no window context";
		case NOT_READY											: return "not ready";
		case TIMEOUT											: return "timeout";
		case EVENT_SET											: return "event set";
		case EVENT_RESET										: return "event reset";
		case INCOMPLETE											: return "incomplete";
		case OUT_OF_HOST_MEMORY 								: return "out of host memory";
		case OUT_OF_DEVICE_MEMORY 								: return "out of device memory";
		case INITIALIZATION_FAILED 								: return "initialization failed";
		case DEVICE_LOST 										: return "device lost";
		case MEMORY_MAP_FAILED 									: return "memory map failed";
		case LAYER_NOT_PRESENT 									: return "layer not present";
		case EXTENSION_NOT_PRESENT 								: return "extension not present";
		case FEATURE_NOT_PRESENT 								: return "feature not present";
		case INCOMPATIBLE_DRIVER 								: return "incompatible driver";
		case TOO_MANY_OBJECTS									: return "too many objects";
		case FORMAT_NOT_SUPPORTED								: return "format not supported";
		case FRAGMENTED_POOL									: return "fragmented pool";
		case OUT_OF_POOL_MEMORY 								: return "out of pool memory";
		case INVALID_EXTERNAL_HANDLE 							: return "invalid external handle";
		case FRAGMENTATION 										: return "fragmentation";
		case INVALID_OPAQUE_CAPTURE_ADDRESS 					: return "invalid opaque capture address";
		case PIPELINE_COMPILE_REQUIRED							: return "pipeline compile required";
		case SURFACE_LOST 										: return "surface lost";
		case NATIVE_WINDOW_IN_USE 								: return "native window in use";
		case SUBOPTIMAL											: return "suboptimal";
		case OUT_OF_DATE 										: return "out of date";
		case INCOMPATIBLE_DISPLAY 								: return "incompatible display";
		case VALIDATION_FAILED									: return "validation failed";
		case INVALID_SHADER 									: return "invalid shader";
		case INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT			: return "invalid drm format modifier plane layout";
		case NOT_PERMITTED 										: return "not permitted";
		case FULL_SCREEN_EXCLUSIVE_MODE_LOST					: return "full screen exclusive mode lost";
		case THREAD_IDLE										: return "thread idle";
		case THREAD_DONE										: return "thread done";
		case OPERATION_DEFERRED									: return "operation deferred";
		case OPERATION_NOT_DEFERRED								: return "operation not deferred";
		case COMPRESSION_EXHAUSTED								: return "compression exhausted ";
		default													: return "";
		}
	}

	const char* log::message::api_to_string(api aAPI) {
		switch (aAPI) {
		case GEODESY:		return "geodesy";
		case GLSLANG: 		return "glslang";
		case FREETYPE: 		return "FreeType";
		case FREEIMAGE: 	return "FreeImage";
		case ASSIMP: 		return "Assimp";
		case PORTAUDIO: 	return "PortAudio";
		case GLFW: 			return "GLFW";
		case VULKAN: 		return "Vulkan";
		default:			return "";
		}
	}

	log::message::message() {
		this->IssuerAPI		= GEODESY;
		this->Type			= INFO;
		this->Code			= SUCCESS;
		this->Content		= "";
	}
	
	log::message::message(api aComplainerAPI, type aMessageType, code aMessageCode, std::string aContent) : message() {
		this->IssuerAPI = aComplainerAPI;
		this->Type = aMessageType;
		this->Code = aMessageCode;
		this->Content = aContent;
	}
	
	log::message::message(api aComplainerAPI, int aComplainerAPICode, std::string aContent) : message() {
		this->IssuerAPI = aComplainerAPI;
		this->infer_code(aComplainerAPI, aComplainerAPICode);
		this->Content = aContent;
	}

	void log::message::infer_code(api aAPI, int aAPICode) {
		switch (aAPI) {
		case GEODESY:
			break;
		case GLSLANG:
			break;
		case FREETYPE:
			break;
		case FREEIMAGE:
			break;
		case ASSIMP:
			break;
		//case PORTAUDIO:
		//	this->Type = log::ERROR;
		//	switch (aAPICode) {
		//	case paNoError:
		//		this->Type = INFO;
		//		this->Code = SUCCESS;
		//		break;
		//	case paNotInitialized:
		//		this->Code = NOT_INITIALIZED;
		//		break;
		//	case paUnanticipatedHostError:
		//		this->Code = UNANTICIPATED_HOST_ERROR;
		//		break;
		//	case paInvalidChannelCount:
		//		this->Code = INVALID_CHANNEL_COUNT;
		//		break;
		//	case paInvalidSampleRate:
		//		this->Code = INVALID_SAMPLE_RATE;
		//		break;
		//	case paInvalidDevice:
		//		this->Code = INVALID_DEVICE;
		//		break;
		//	case paInvalidFlag:
		//		this->Code = INVALID_FLAG;
		//		break;
		//	case paSampleFormatNotSupported:
		//		this->Code = SAMPLE_FORMAT_NOT_SUPPORTED;
		//		break;
		//	case paBadIODeviceCombination:
		//		this->Code = BAD_IO_DEVICE_COMBINATION;
		//		break;
		//	case paInsufficientMemory:
		//		this->Code = INSUFFICIENT_MEMORY;
		//		break;
		//	case paBufferTooBig:
		//		this->Code = BUFFER_TOO_BIG;
		//		break;
		//	case paBufferTooSmall:
		//		this->Code = BUFFER_TOO_SMALL;
		//		break;
		//	case paNullCallback:
		//		this->Code = NULL_CALLBACK;
		//		break;
		//	case paBadStreamPtr:
		//		this->Code = BAD_STREAM_PTR;
		//		break;
		//	case paTimedOut:
		//		this->Code = TIMED_OUT;
		//		break;
		//	case paInternalError:
		//		this->Code = INTERNAL_ERROR;
		//		break;
		//	case paDeviceUnavailable:
		//		this->Code = DEVICE_UNAVAILABLE;
		//		break;
		//	case paIncompatibleHostApiSpecificStreamInfo:
		//		this->Code = INCOMPATIBLE_HOST_API_SPECIFIC_STREAM_INFO;
		//		break;
		//	case paStreamIsStopped:
		//		this->Code = STREAM_IS_STOPPED;
		//		break;
		//	case paStreamIsNotStopped:
		//		this->Code = STREAM_IS_NOT_STOPPED;
		//		break;
		//	case paInputOverflowed:
		//		this->Code = INPUT_OVERFLOWED;
		//		break;
		//	case paOutputUnderflowed:
		//		this->Code = OUTPUT_UNDERFLOWED;
		//		break;
		//	case paHostApiNotFound:
		//		this->Code = HOST_API_NOT_FOUND;
		//		break;
		//	case paInvalidHostApi:
		//		this->Code = INVALID_HOST_API;
		//		break;
		//	case paCanNotReadFromACallbackStream:
		//		this->Code = CAN_NOT_READ_FROM_A_CALLBACK_STREAM;
		//		break;
		//	case paCanNotWriteToACallbackStream:
		//		this->Code = CAN_NOT_WRITE_TO_A_CALLBACK_STREAM;
		//		break;
		//	case paCanNotReadFromAnOutputOnlyStream:
		//		this->Code = CAN_NOT_READ_FROM_AN_OUTPUT_ONLY_STREAM;
		//		break;
		//	case paCanNotWriteToAnInputOnlyStream:
		//		this->Code = CAN_NOT_WRITE_TO_AN_INPUT_ONLY_STREAM;
		//		break;
		//	case paIncompatibleStreamHostApi:
		//		this->Code = INCOMPATIBLE_STREAM_HOST_API;
		//		break;
		//	case paBadBufferPtr:
		//		this->Code = BAD_BUFFER_PTR;
		//		break;
		//	}
		//	break;
		case GLFW:
			this->Type = log::ERROR;
			switch (aAPICode) {
			case GLFW_NOT_INITIALIZED:
				this->Code = NOT_INITIALIZED;
				break;
			case GLFW_NO_CURRENT_CONTEXT:
				this->Code = NO_CURRENT_CONTEXT;
				break;
			case GLFW_INVALID_ENUM:
				this->Code = INVALID_ENUM;
				break;
			case GLFW_INVALID_VALUE:
				this->Code = INVALID_VALUE;
				break;
			case GLFW_OUT_OF_MEMORY:
				this->Code = OUT_OF_MEMORY;
				break;
			case GLFW_API_UNAVAILABLE:
				this->Code = API_UNAVAILABLE;
				break;
			case GLFW_VERSION_UNAVAILABLE:
				this->Code = VERSION_UNAVAILABLE;
				break;
			case GLFW_PLATFORM_ERROR:
				this->Code = PLATFORM_ERROR;
				break;
			case GLFW_FORMAT_UNAVAILABLE:
				this->Code = FORMAT_UNAVAILABLE;
				break;
			case GLFW_NO_WINDOW_CONTEXT:
				this->Code = NO_WINDOW_CONTEXT;
				break;
			default:
				this->Code = UNKNOWN;
				break;
			}
			break;
		case VULKAN:
			switch (aAPICode) {
			case VK_SUCCESS:
				this->Type = INFO;
				this->Code = SUCCESS;
				break;
			case VK_NOT_READY:
				this->Type = INFO;
				this->Code = NOT_READY;
				break;
			case VK_TIMEOUT:
				this->Type = INFO;
				this->Code = TIMEOUT;
				break;
			case VK_EVENT_SET:
				this->Type = INFO;
				this->Code = EVENT_SET;
				break;
			case VK_EVENT_RESET:
				this->Type = INFO;
				this->Code = EVENT_RESET;
				break;
			case VK_INCOMPLETE:
				this->Type = INFO;
				this->Code = INCOMPLETE;
				break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				this->Type = ERROR;
				this->Code = OUT_OF_HOST_MEMORY;
				break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				this->Type = ERROR;
				this->Code = OUT_OF_DEVICE_MEMORY;
				break;
			case VK_ERROR_INITIALIZATION_FAILED:
				this->Type = ERROR;
				this->Code = INITIALIZATION_FAILED;
				break;
			case VK_ERROR_DEVICE_LOST:
				this->Type = ERROR;
				this->Code = DEVICE_LOST;
				break;
			case VK_ERROR_MEMORY_MAP_FAILED:
				this->Type = ERROR;
				this->Code = MEMORY_MAP_FAILED;
				break;
			case VK_ERROR_LAYER_NOT_PRESENT:
				this->Type = ERROR;
				this->Code = LAYER_NOT_PRESENT;
				break;
			case VK_ERROR_EXTENSION_NOT_PRESENT:
				this->Type = ERROR;
				this->Code = EXTENSION_NOT_PRESENT;
				break;
			case VK_ERROR_FEATURE_NOT_PRESENT:
				this->Type = ERROR;
				this->Code = FEATURE_NOT_PRESENT;
				break;
			case VK_ERROR_INCOMPATIBLE_DRIVER:
				this->Type = ERROR;
				this->Code = INCOMPATIBLE_DRIVER;
				break;
			case VK_ERROR_TOO_MANY_OBJECTS:
				this->Type = ERROR;
				this->Code = TOO_MANY_OBJECTS;
				break;
			case VK_ERROR_FORMAT_NOT_SUPPORTED:
				this->Type = ERROR;
				this->Code = FORMAT_NOT_SUPPORTED;
				break;
			case VK_ERROR_FRAGMENTED_POOL:
				this->Type = ERROR;
				this->Code = FRAGMENTED_POOL;
				break;
			case VK_ERROR_UNKNOWN:
				this->Type = ERROR;
				this->Code = UNKNOWN;
				break;
			case VK_ERROR_OUT_OF_POOL_MEMORY:
				this->Type = ERROR;
				this->Code = OUT_OF_POOL_MEMORY;
				break;
			case VK_ERROR_INVALID_EXTERNAL_HANDLE:
				this->Type = ERROR;
				this->Code = INVALID_EXTERNAL_HANDLE;
				break;
			case VK_ERROR_FRAGMENTATION:
				this->Type = ERROR;
				this->Code = FRAGMENTATION;
				break;
			case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
				this->Type = ERROR;
				this->Code = INVALID_OPAQUE_CAPTURE_ADDRESS;
				break;
			case VK_PIPELINE_COMPILE_REQUIRED:
				this->Type = INFO;
				this->Code = PIPELINE_COMPILE_REQUIRED;
				break;
			case VK_ERROR_SURFACE_LOST_KHR:
				this->Type = ERROR;
				this->Code = SURFACE_LOST;
				break;
			case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
				this->Type = ERROR;
				this->Code = NATIVE_WINDOW_IN_USE;
				break;
			case VK_SUBOPTIMAL_KHR:
				this->Type = INFO;
				this->Code = SUBOPTIMAL;
				break;
			case VK_ERROR_OUT_OF_DATE_KHR:
				this->Type = ERROR;
				this->Code = OUT_OF_DATE;
				break;
			case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
				this->Type = ERROR;
				this->Code = INCOMPATIBLE_DISPLAY;
				break;
			case VK_ERROR_VALIDATION_FAILED_EXT:
				this->Type = ERROR;
				this->Code = VALIDATION_FAILED;
				break;
			case VK_ERROR_INVALID_SHADER_NV:
				this->Type = ERROR;
				this->Code = INVALID_SHADER;
				break;
			case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
				this->Type = ERROR;
				this->Code = INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT;
				break;
			case VK_ERROR_NOT_PERMITTED_KHR:
				this->Type = ERROR;
				this->Code = NOT_PERMITTED;
				break;
			case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
				this->Type = ERROR;
				this->Code = FULL_SCREEN_EXCLUSIVE_MODE_LOST;
				break;
			case VK_THREAD_IDLE_KHR:
				this->Type = INFO;
				this->Code = THREAD_IDLE;
				break;
			case VK_THREAD_DONE_KHR:
				this->Type = INFO;
				this->Code = THREAD_DONE;
				break;
			case VK_OPERATION_DEFERRED_KHR:
				this->Type = INFO;
				this->Code = OPERATION_DEFERRED;
				break;
			case VK_OPERATION_NOT_DEFERRED_KHR:
				this->Type = INFO;
				this->Code = OPERATION_NOT_DEFERRED;
				break;
			case VK_ERROR_COMPRESSION_EXHAUSTED_EXT:
				this->Type = ERROR;
				this->Code = COMPRESSION_EXHAUSTED;
				break;
			}
			break;
		default:
			break;
		}
	}

	const char* log::convert_vulkan_code(VkResult aResult) {
		const char* temp;
		switch (aResult) {
		default:														return "Error: Unknown";
		case VK_SUCCESS:												return "Success";
		case VK_NOT_READY:												return "Not Ready.";
		case VK_TIMEOUT:												return "Timeout.";
		case VK_EVENT_SET:												return "Event Set.";
		case VK_EVENT_RESET:											return "Event Reset.";
		case VK_INCOMPLETE:												return "Incomplete.";
		case VK_ERROR_OUT_OF_HOST_MEMORY:								return "Error: Out of Host Memory.";
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:								return "Error: Out of Device Memory.";
		case VK_ERROR_INITIALIZATION_FAILED:							return "Error: Initialization failed.";
		case VK_ERROR_DEVICE_LOST:										return "Error: Device Lost";
		case VK_ERROR_MEMORY_MAP_FAILED:								return "Error: Memory Map Failed.";
		case VK_ERROR_LAYER_NOT_PRESENT:								return "Error: Layer Not Present.";
		case VK_ERROR_EXTENSION_NOT_PRESENT:							return "Error: Extension not present.";
		case VK_ERROR_FEATURE_NOT_PRESENT:								return "Error: Feature Not Present.";
		case VK_ERROR_INCOMPATIBLE_DRIVER:								return "Error: Incompatible Driver.";
		case VK_ERROR_TOO_MANY_OBJECTS:									return "Error: Too many objects.";
		case VK_ERROR_FORMAT_NOT_SUPPORTED:								return "Error: Format not supported.";
		case VK_ERROR_FRAGMENTED_POOL:									return "Error: Fragmented pool.";
		case VK_ERROR_UNKNOWN:											return "Error: Unknown.";
		case VK_ERROR_OUT_OF_POOL_MEMORY:								return "Error: Out of pool memory.";
		case VK_ERROR_INVALID_EXTERNAL_HANDLE:							return "Error: Invalid external handle.";
		case VK_ERROR_FRAGMENTATION:									return "Error: Fragmentation.";
		case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:					return "Error: Invalid opaque capture address.";
		case VK_ERROR_SURFACE_LOST_KHR:									return "Error: Surface lost (KHR)";
		case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:							return "Error: Native window in use (KHR)";
		case VK_SUBOPTIMAL_KHR:											return "Suboptimal (khr)";
		case VK_ERROR_OUT_OF_DATE_KHR:									return "Error: Out of date (khr)";
		case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:							return "Error: Incompatible display.";
		case VK_ERROR_VALIDATION_FAILED_EXT:							return "Error: Validation failed (ext).";
		case VK_ERROR_INVALID_SHADER_NV:								return "Error: Invalid shader (nv)";
		case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:		return "Error: Invalid DRM format modifier plane layout (ext)";
		case VK_ERROR_NOT_PERMITTED_EXT:								return "Error: Not permitted extension";
		case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:				return "Error: Fullscreen exclusive mode lost.";
		case VK_THREAD_IDLE_KHR:										return "Thread Idle (khr)";
		case VK_THREAD_DONE_KHR:										return "Thread Done (khr)";
		case VK_OPERATION_DEFERRED_KHR:									return "Operation Deferred (khr)";
		case VK_OPERATION_NOT_DEFERRED_KHR:								return "Operation not deferred (khr)";
		case VK_PIPELINE_COMPILE_REQUIRED_EXT:							return "Pipeline compile required. (ext)";
		}
		/*
		VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR = VK_ERROR_INVALID_EXTERNAL_HANDLE,
		VK_ERROR_FRAGMENTATION_EXT = VK_ERROR_FRAGMENTATION,
		VK_ERROR_INVALID_DEVICE_ADDRESS_EXT = VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS,
		VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR = VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS,
		VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT = VK_PIPELINE_COMPILE_REQUIRED_EXT,
		VK_RESULT_MAX_ENUM = 0x7FFFFFFF
		*/
	}

	void log::operator<<(log aNewLog) {
		Message.insert(Message.end(), aNewLog.Message.begin(), aNewLog.Message.end());
	}

	void log::operator<<(message aNewMessage) {
		Message.push_back(aNewMessage);
	}

}