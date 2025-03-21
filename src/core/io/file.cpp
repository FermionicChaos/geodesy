#include <geodesy/core/io/file.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <filesystem>
#include <algorithm>

// Loadable Files.
#include <geodesy/core/io/dynalib.h>
#include <geodesy/core/gpu/image.h>
#include <geodesy/core/gpu/shader.h>
#include <geodesy/core/gfx/font.h>
#include <geodesy/core/gfx/model.h>

namespace geodesy::core::io {

	// This is just a lookup table for file type extensions.
	// Extension ID, Extension String, Category
	std::vector<file::registry_item> file::Registry = {
		{ file::extension::DYNALIB_DYN,				file::loader::DYNALIB, 				{ "dll", "so", "dylib" }	},
		{ file::extension::IMAGE_BMP,				file::loader::IMAGE, 				{ "bmp" }					},
		{ file::extension::IMAGE_ICO,				file::loader::IMAGE, 				{ "ico" }					},
		{ file::extension::IMAGE_JPEG,				file::loader::IMAGE, 				{ "jpg", "jpeg" }			},
		{ file::extension::IMAGE_JNG,				file::loader::IMAGE, 				{ "jng" }					},
		{ file::extension::IMAGE_KOALA,				file::loader::IMAGE, 				{ "koala" }					},
		{ file::extension::IMAGE_LBM,				file::loader::IMAGE, 				{ "lbm" }					},
		{ file::extension::IMAGE_IFF,				file::loader::IMAGE, 				{ "iff" }					},
		{ file::extension::IMAGE_MNG,				file::loader::IMAGE, 				{ "mng" }					},
		{ file::extension::IMAGE_PBM,				file::loader::IMAGE, 				{ "pbm" }					},
		{ file::extension::IMAGE_PBMRAW,			file::loader::IMAGE, 				{ "pbmraw" }				},
		{ file::extension::IMAGE_PCD,				file::loader::IMAGE, 				{ "pcd" }					},
		{ file::extension::IMAGE_PCX,				file::loader::IMAGE, 				{ "pcx" }					},
		{ file::extension::IMAGE_PGM,				file::loader::IMAGE, 				{ "pgm" }					},
		{ file::extension::IMAGE_PGMRAW,			file::loader::IMAGE, 				{ "pgmraw" }				},
		{ file::extension::IMAGE_PNG,				file::loader::IMAGE, 				{ "png" }					},
		{ file::extension::IMAGE_PPM,				file::loader::IMAGE, 				{ "ppm" }					},
		{ file::extension::IMAGE_PPMRAW,			file::loader::IMAGE, 				{ "ppmraw" }				},
		{ file::extension::IMAGE_RAS,				file::loader::IMAGE, 				{ "ras" }					},
		{ file::extension::IMAGE_TARGA,				file::loader::IMAGE, 				{ "targa" }					},
		{ file::extension::IMAGE_TIFF,				file::loader::IMAGE, 				{ "tiff" }					},
		{ file::extension::IMAGE_WBMP,				file::loader::IMAGE, 				{ "wbmp" }					},
		{ file::extension::IMAGE_PSD,				file::loader::IMAGE, 				{ "psd" }					},
		{ file::extension::IMAGE_CUT,				file::loader::IMAGE, 				{ "cut" }					},
		{ file::extension::IMAGE_XBM,				file::loader::IMAGE, 				{ "xbm" }					},
		{ file::extension::IMAGE_XPM,				file::loader::IMAGE, 				{ "xpm" }					},
		{ file::extension::IMAGE_DDS,				file::loader::IMAGE, 				{ "dds" }					},
		{ file::extension::IMAGE_GIF,				file::loader::IMAGE, 				{ "gif" }					},
		{ file::extension::IMAGE_HDR,				file::loader::IMAGE, 				{ "hdr" }					},
		{ file::extension::IMAGE_FAXG3,				file::loader::IMAGE, 				{ "faxg3" }					},
		{ file::extension::IMAGE_SGI,				file::loader::IMAGE, 				{ "sgi" }					},
		{ file::extension::IMAGE_EXR,				file::loader::IMAGE, 				{ "exr" }					},
		{ file::extension::IMAGE_J2K,				file::loader::IMAGE, 				{ "j2k" }					},
		{ file::extension::IMAGE_JP2,				file::loader::IMAGE, 				{ "jp2" }					},
		{ file::extension::IMAGE_PFM,				file::loader::IMAGE, 				{ "pfm" }					},
		{ file::extension::IMAGE_PICT,				file::loader::IMAGE, 				{ "pict" }					},
		{ file::extension::IMAGE_RAW,				file::loader::IMAGE, 				{ "raw" }					},
		{ file::extension::IMAGE_WEBP,				file::loader::IMAGE, 				{ "webp" }					},
		{ file::extension::IMAGE_JXR,				file::loader::IMAGE, 				{ "jxr" }					},
		{ file::extension::SHADER_VSH,				file::loader::SHADER, 				{ "vsh", "vert" }			},
		{ file::extension::SHADER_TCSH,				file::loader::SHADER, 				{ "tcsh" }					},
		{ file::extension::SHADER_TESH,				file::loader::SHADER, 				{ "tesh" }					},
		{ file::extension::SHADER_GSH,				file::loader::SHADER, 				{ "gsh" }					},
		{ file::extension::SHADER_PSH,				file::loader::SHADER, 				{ "psh", "fsh", "frag" }	},		
		{ file::extension::SHADER_RGSH,				file::loader::SHADER, 				{ "rgsh" }					},
		{ file::extension::SHADER_AHSH,				file::loader::SHADER, 				{ "ahsh" }					},
		{ file::extension::SHADER_CHSH,				file::loader::SHADER, 				{ "chsh" }					},
		{ file::extension::SHADER_MSSH,				file::loader::SHADER, 				{ "mssh" }					},
		{ file::extension::SHADER_INSH,				file::loader::SHADER, 				{ "insh" }					},
		{ file::extension::SHADER_CASH,				file::loader::SHADER, 				{ "cash" }					},
		{ file::extension::SHADER_GLSL,				file::loader::SHADER, 				{ "glsl" }					},
		{ file::extension::SHADER_SPV,				file::loader::SHADER, 				{ "spv" }					},
		{ file::extension::FONT_TTF,				file::loader::FONT, 				{ "ttf" }					},
		{ file::extension::FONT_TTC,				file::loader::FONT, 				{ "ttc" }					},
		{ file::extension::FONT_OTF,				file::loader::FONT, 				{ "otf" }					},
		{ file::extension::FONT_PFM,				file::loader::FONT, 				{ "pfm" }					},
		{ file::extension::MODEL_3MF,				file::loader::MODEL, 				{ "3mf" }					},
		{ file::extension::MODEL_DAE,				file::loader::MODEL, 				{ "dae" }					},
		{ file::extension::MODEL_BVH,				file::loader::MODEL, 				{ "bvh" }					},
		{ file::extension::MODEL_3DS,				file::loader::MODEL, 				{ "3ds" }					},
		{ file::extension::MODEL_ASE,				file::loader::MODEL, 				{ "ase" }					},
		{ file::extension::MODEL_GLTF,				file::loader::MODEL, 				{ "gltf" }					},
		{ file::extension::MODEL_FBX,				file::loader::MODEL, 				{ "fbx" }					},
		{ file::extension::MODEL_PLY,				file::loader::MODEL, 				{ "ply" }					},
		{ file::extension::MODEL_DXF,				file::loader::MODEL, 				{ "dxf" }					},
		{ file::extension::MODEL_IFC,				file::loader::MODEL, 				{ "ifc" }					},
		{ file::extension::MODEL_NFF,				file::loader::MODEL, 				{ "nff" }					},
		{ file::extension::MODEL_SMD,				file::loader::MODEL, 				{ "smd" }					},
		{ file::extension::MODEL_VTA,				file::loader::MODEL, 				{ "vta" }					},
		{ file::extension::MODEL_MDL,				file::loader::MODEL, 				{ "mdl" }					},
		{ file::extension::MODEL_MD2,				file::loader::MODEL, 				{ "md2" }					},
		{ file::extension::MODEL_MD3,				file::loader::MODEL, 				{ "md3" }					},
		{ file::extension::MODEL_PK3,				file::loader::MODEL, 				{ "pk3" }					},
		{ file::extension::MODEL_MDC,				file::loader::MODEL, 				{ "mdc" }					},
		{ file::extension::MODEL_MD5MESH,			file::loader::MODEL, 				{ "md5mesh" }				},
		{ file::extension::MODEL_MD5ANIM,			file::loader::MODEL, 				{ "md5anim" }				},
		{ file::extension::MODEL_MD5CAMERA,			file::loader::MODEL, 				{ "md5camera" }				},
		{ file::extension::MODEL_X,					file::loader::MODEL, 				{ "x" }						},
		{ file::extension::MODEL_Q3O,				file::loader::MODEL, 				{ "q3o" }					},
		{ file::extension::MODEL_Q3S,				file::loader::MODEL, 				{ "q3s" }					},
		{ file::extension::MODEL_AC,				file::loader::MODEL, 				{ "ac" }					},
		{ file::extension::MODEL_AC3D,				file::loader::MODEL, 				{ "ac3d" }					},
		{ file::extension::MODEL_STL,				file::loader::MODEL, 				{ "stl" }					},
		{ file::extension::MODEL_IRRMESH,			file::loader::MODEL, 				{ "irrmesh" }				},
		{ file::extension::MODEL_IRR,				file::loader::MODEL, 				{ "irr" }					},
		{ file::extension::MODEL_OFF,				file::loader::MODEL, 				{ "off" }					},
		{ file::extension::MODEL_OBJ,				file::loader::MODEL, 				{ "obj" }					},
		{ file::extension::MODEL_TER,				file::loader::MODEL, 				{ "ter" }					},
		{ file::extension::MODEL_HMP,				file::loader::MODEL, 				{ "hmp" }					},
		{ file::extension::MODEL_MESH_XML,			file::loader::MODEL, 				{ "mesh_xml" }				},
		{ file::extension::MODEL_SKELETON_XML,		file::loader::MODEL, 				{ "skeleton_xml" }			},
		{ file::extension::MODEL_MATERIAL,			file::loader::MODEL, 				{ "material" }				},
		{ file::extension::MODEL_OGEX,				file::loader::MODEL, 				{ "ogex" }					},
		{ file::extension::MODEL_MS3D,				file::loader::MODEL, 				{ "ms3d" }					},
		{ file::extension::MODEL_LWO,				file::loader::MODEL, 				{ "lwo" }					},
		{ file::extension::MODEL_LWS,				file::loader::MODEL, 				{ "lws" }					},
		{ file::extension::MODEL_CSM,				file::loader::MODEL, 				{ "csm" }					},
		{ file::extension::MODEL_COB,				file::loader::MODEL, 				{ "cob" }					},
		{ file::extension::MODEL_SCN,				file::loader::MODEL, 				{ "scn" }					},
		{ file::extension::MODEL_XGL,				file::loader::MODEL, 				{ "xgl" }					},
	};

	std::vector<std::shared_ptr<file>> file::manager::open(std::vector<std::string> aFilePathList) {
		std::vector<std::shared_ptr<file>> OpenedFiles(aFilePathList.size());
		for (size_t i = 0; i < aFilePathList.size(); i++) {
			OpenedFiles[i] = this->open(aFilePathList[i]);
		}
		return OpenedFiles;
	}

	std::shared_ptr<file> file::manager::open(std::string aFilePath) {
		std::shared_ptr<file> OpenedFile;
		std::filesystem::path FilePath(aFilePath);
		// Convert aFilePath to an absolute path.
		std::filesystem::path AbsolutePath = std::filesystem::absolute(FilePath);
		// Check if the file exists.
		if (!(std::filesystem::exists(AbsolutePath) && std::filesystem::is_regular_file(AbsolutePath))) return OpenedFile;
		// Convert the absolute path to a string.
		std::string AbsolutePathString = AbsolutePath.string();
		// Replace all back slashes with forward slashes.
		std::replace(AbsolutePathString.begin(), AbsolutePathString.end(), '\\', '/');
		// Check if file has already been opened, or if the existing file is still valid.
		if ((this->LoadedFiles.count(AbsolutePathString) == 0) || (this->LoadedFiles[AbsolutePathString].expired())) {
			// Attempt to open file.
			OpenedFile = file::open(AbsolutePathString, this);

			// If file doesn't exist, return nullptr.
			if (OpenedFile == nullptr) return nullptr;

			// Store opened file in weak pointer registry.
			this->LoadedFiles[AbsolutePathString] = OpenedFile;
		}
		else {
			// File is already opened, generate a new shared pointer.
			OpenedFile = this->LoadedFiles[AbsolutePathString].lock();
		}
		return OpenedFile;
	}

	file::extension file::string_to_eid(std::string aExtension) {
		for (size_t i = 0; i < Registry.size(); i++) {
			for (size_t j = 0; j < Registry[i].Extension.size(); j++)
				if (Registry[i].Extension[j] == aExtension) {
					return Registry[i].ExtensionID;
				}
		}
		return EID_UNKNOWN;
	}

	file::loader file::eid_to_iid(extension aExtensionID) {
		for (size_t i = 0; i < Registry.size(); i++) {
			if (Registry[i].ExtensionID == aExtensionID) {
				return Registry[i].LoaderID;
			}
		}
		return IID_UNKOWN;
	}

	std::string file::eid_to_string(extension aExtensionID) {
		for (size_t i = 0; i < Registry.size(); i++) {
			if (Registry[i].ExtensionID == aExtensionID) {
				return Registry[i].Extension[0];
			}
		}
		return "";
	}

	std::shared_ptr<file> file::open(std::string aFilePath, manager* aFileManager) {
		std::string Extension;
		// Write a piece of code that extracts the extension from the file path.
		// Iterate from the string backwards until a '.' is found.
		// Then extract the extension from the string.
		for (size_t i = aFilePath.size() - 1; i > 0; i--) {
			if (aFilePath[i] == '.') {
				Extension = aFilePath.substr(i + 1, aFilePath.size() - i - 1);
				break;
			}
		}
		file::extension ExtensionID = string_to_eid(Extension);
		file::loader LoaderID = eid_to_iid(ExtensionID);
		switch (LoaderID) {
		default: 			return nullptr;
		case DYNALIB: 		return std::make_shared<dynalib>(aFilePath);
		case IMAGE: 		return std::make_shared<gpu::image>(aFilePath);
		case SHADER: 		return std::make_shared<gpu::shader>(aFilePath);
		//case FONT: 			return std::make_shared<gfx::font>(aFilePath);
		case MODEL: 		return std::shared_ptr<gfx::model>(new gfx::model(aFilePath, aFileManager));
		}
	}

	file::file() {
		this->Path = "";
		this->Directory = "";
		this->Name = "";
		this->Extension = "";
		this->ExtensionID = EID_UNKNOWN;
		this->HostSize = 0;
		this->HostData = NULL;
	}

	file::file(std::string aFilePath) : file() {
		// Disable autoloading.
		// this->read(aFilePath);

		// Assuming the provided path is valid, seperate into directory, name, and extension.
		this->Path = aFilePath;

		// Extract the directory from the path.
		size_t LastSlash = aFilePath.find_last_of('/');
		if (LastSlash != std::string::npos) {
			this->Directory = aFilePath.substr(0, LastSlash);
		}

		// Extract the name from the path.
		size_t LastDot = aFilePath.find_last_of('.');
		if (LastDot != std::string::npos) {
			this->Name = aFilePath.substr(LastSlash + 1, LastDot - LastSlash - 1);
		}

		// Extract the extension from the path.
		if (LastDot != std::string::npos) {
			this->Extension = aFilePath.substr(LastDot + 1, aFilePath.size() - LastDot - 1);
		}

		// Convert the extension to an extension ID.
		this->ExtensionID = string_to_eid(this->Extension);
	}

	bool file::read(std::string aFilePath) {
		size_t ReturnValue = 0;
		FILE* Handle = fopen(aFilePath.c_str(), "rb");
		
		if (Handle == NULL) return true;
		long FileSize = 0;
		fseek(Handle, 0L, SEEK_END);
		FileSize = ftell(Handle);
		fseek(Handle, 0L, SEEK_SET);

		this->HostData = malloc((FileSize + 1) * sizeof(char));
		if (this->HostData == NULL) return true;

		memset(this->HostData, 0xAA, (FileSize + 1) * sizeof(char));

		this->HostSize = FileSize;
		ReturnValue = fread(this->HostData, sizeof(char), this->HostSize, Handle);

		char* String = (char*)this->HostData;
		String[this->HostSize] = '\0';

		fclose(Handle);
		return false;
	}

}

