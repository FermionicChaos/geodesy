#include <geodesy/core/gfx/font.h>

#include <stdlib.h>
#include <string.h>

// Font Loading
#include <ft2build.h>
#include FT_FREETYPE_H

static FT_Library LibraryHandle;

namespace geodesy::core::gfx {

	font::font() {
		this->zero_out();
	}

	font::font(std::string aFilePath) : io::file(aFilePath) {

		FT_Face Face;
		FT_Error ErrorCodeFT;
		ErrorCodeFT = FT_New_Face(LibraryHandle, aFilePath.c_str(), 0, &Face);
		ErrorCodeFT = FT_Set_Pixel_Sizes(Face, 0, 128);

		m = 128; n = 128; l = 128;
		sx = (float*)malloc(l * sizeof(float));
		sy = (float*)malloc(l * sizeof(float));
		bx = (float*)malloc(l * sizeof(float));
		by = (float*)malloc(l * sizeof(float));
		ax = (float*)malloc(l * sizeof(float));
		ay = (float*)malloc(l * sizeof(float));

		//SymbolSet.EnableMipmap = false;
		//SymbolSet.InternalFormat = GL_RED;
		//SymbolSet.reserve_host_memory(texture::TEXTURE_3D, m, n, l, texture::RED, texture::UNSIGNED_BYTE);
		//unsigned char* nptr = (unsigned char*)SymbolSet.hptr;

		int i, j, k;
		for (k = 0; k < l; k++) {
			FT_Load_Char(Face, k, FT_LOAD_RENDER);

			sx[k] = ((float)(Face->glyph->bitmap.width));
			sy[k] = ((float)(Face->glyph->bitmap.rows));
			bx[k] = ((float)(Face->glyph->bitmap_left));
			by[k] = ((float)(Face->glyph->bitmap_top));
			ax[k] = ((float)((Face->glyph->advance.x >> 6)));
			ay[k] = ((float)((Face->glyph->advance.y >> 6)));

			for (j = 0; j < n; j++) {
				for (i = 0; i < m; i++) {
					if ((i < Face->glyph->bitmap.width) && (j < Face->glyph->bitmap.rows)) {
						//hptr[i + j * m + k * m * n] = Face->glyph->bitmap.buffer[i + j * Face->glyph->bitmap.width];
					}
					else {
						//hptr[i + j * m + k * m * n] = '\0';
					}
				}
			}

		}

		FT_Done_Face(Face);

	}

	font::font(const char* aFilePath) : io::file(aFilePath) {

	}

	font::~font() {

	}

	bool font::initialize() {
		return (FT_Init_FreeType(&LibraryHandle) == 0);
	}

	bool font::terminate() {
		FT_Done_FreeType(LibraryHandle);
		return false;
	}

	void font::zero_out() {
		this->m = 0; this->n = 0; this->l = 0;
		this->hptr = NULL;
		this->sx = NULL;
		this->sy = NULL;
		this->bx = NULL;
		this->by = NULL;
		this->ax = NULL;
		this->ay = NULL;
	}

}

// This Stuff WORKS, do NOT LOSE IT.
/*
// Font Loading
#include <ft2build.h>
#include FT_FREETYPE_H

static FT_Library FontLibrary;

bool font::initialize() {
	if (FT_Init_FreeType(&FontLibrary)) { return true; }
	return false;
}

bool font::terminate() {
	FT_Done_FreeType(FontLibrary);
	return false;
}

font::font() {
	sx = NULL; sy = NULL;
	bx = NULL; by = NULL;
	ax = NULL; ay = NULL;
	m = 0; n = 0; l = 0;
}

font::~font() {
	// Gozer: Choose the form of the destructor!
	if (sx != NULL) { free(sx); sx = NULL; }
	if (sy != NULL) { free(sy); sy = NULL; }
	if (bx != NULL) { free(bx); bx = NULL; }
	if (by != NULL) { free(by); by = NULL; }
	if (ax != NULL) { free(ax); ax = NULL; }
	if (ay != NULL) { free(ay); ay = NULL; }

	SymbolSet.release_host_memory();
	SymbolSet.release_device_memory();

	m = 0; n = 0; l = 0;
}

bool font::read(const char * Path) {
	if (l != 0) { return true; }
	FT_Face Face;
	FT_Error ErrorCodeFT; 
	ErrorCodeFT = FT_New_Face(FontLibrary, Path, 0, &Face);
	ErrorCodeFT = FT_Set_Pixel_Sizes(Face, 0, 128);
	
	m = 128; n = 128; l = 128;
	sx = (float*)malloc(l*sizeof(float));
	sy = (float*)malloc(l*sizeof(float));
	bx = (float*)malloc(l*sizeof(float));
	by = (float*)malloc(l*sizeof(float));
	ax = (float*)malloc(l*sizeof(float));
	ay = (float*)malloc(l*sizeof(float));

	SymbolSet.EnableMipmap = false;
	SymbolSet.InternalFormat = GL_RED;
	SymbolSet.reserve_host_memory(texture::TEXTURE_3D, m, n, l, texture::RED, texture::UNSIGNED_BYTE);

	unsigned char *nptr = (unsigned char*)SymbolSet.hptr;
	int i, j, k;
	for (k = 0; k < l; k++) {
		FT_Load_Char(Face, k, FT_LOAD_RENDER);

		sx[k] = ((float)(Face->glyph->bitmap.width));
		sy[k] = ((float)(Face->glyph->bitmap.rows));
		bx[k] = ((float)(Face->glyph->bitmap_left));
		by[k] = ((float)(Face->glyph->bitmap_top));
		ax[k] = ((float)((Face->glyph->advance.x >> 6)));
		ay[k] = ((float)((Face->glyph->advance.y >> 6)));

		for (j = 0; j < n; j++) {
			for (i = 0; i < m; i++) {
				if ((i < Face->glyph->bitmap.width) && (j < Face->glyph->bitmap.rows)) {
					nptr[i + j*m + k*m*n] = Face->glyph->bitmap.buffer[i + j*Face->glyph->bitmap.width];
				}
				else {
					nptr[i + j*m + k*m*n] = '\0';
				}
			}
		}

	}

	FT_Done_Face(Face);

	return false;
}

bool font::load() {
	SymbolSet.reserve_device_memory(NULL, 0, 0, 0, NULL, NULL);
	//SymbolSet.send_to_device();
	return false;
}

bool font::clear() {
	if (sx != NULL) { free(sx); sx = NULL; }
	if (sy != NULL) { free(sy); sy = NULL; }
	if (bx != NULL) { free(bx); bx = NULL; }
	if (by != NULL) { free(by); by = NULL; }
	if (ax != NULL) { free(ax); ax = NULL; }
	if (ay != NULL) { free(ay); ay = NULL; }

	SymbolSet.release_host_memory();
	SymbolSet.release_device_memory();

	m = 0; n = 0; l = 0;
	return false;
}

//void font::print() {
//	// Prints character set on terminal
//	unsigned char *nptr = (unsigned char*)hptr;
//	int i, j, k;
//
//	//for (k = 0; k < l; k++) {
//	//	std::cout << "Character = " << char(k);
//	//	std::cout << "\t sy[" << k << "] = " << sx[k];
//	//	std::cout << "\t sx[" << k << "] = " << sy[k];
//	//	std::cout << "\t ax[" << k << "] = " << ax[k];
//	//	std::cout << "\t ay[" << k << "] = " << ay[k];
//	//	std::cout << "\t bx[" << k << "] = " << bx[k];
//	//	std::cout << "\t by[" << k << "] = " << by[k];
//	//	std::cout << std::endl;
//	//}
//
//	for (k = 0; k < l; k++) {
//		for (j = 0; j < n; j++) {
//			for (i = 0; i < m; i++) {
//				if ((i < sx[k]) && (j < sy[k])) {
//					if (nptr[i + j * m + k * m*n] != 0x00) {
//						std::cout << "@";
//					}
//					else {
//						std::cout << "*";
//					}
//				}
//				else {
//					std::cout << "-";
//				}
//			}
//			std::cout << std::endl;
//		}
//		std::cout << std::endl;
//		std::cout << std::endl;
//		std::cout << std::endl;
//	}
//}
*/
