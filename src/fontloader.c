#include "../libs/deckify.h"

void loadFont(const char* fontPath, FT_Library* ft, FT_Face* face, int fontSize) {
	if (FT_Init_FreeType(ft)) {
		fprintf(stderr, "Could not initialize FreeType library\n");
		exit(EXIT_FAILURE);
	}

	if (FT_New_Face(*ft, fontPath, 0, face)) {
		fprintf(stderr, "Could not load font: %s\n", fontPath);
		exit(EXIT_FAILURE);
	}

	FT_Set_Pixel_Sizes(*face, 0, fontSize);
}

Glyph *generateFontTexture(FT_Face face) {
	int charcount = face->num_glyphs;
	Glyph* glyphs = malloc((charcount + 1) * sizeof(Glyph));
	if (!glyphs) {
		fprintf(stderr, "Failed to allocate memory for glyphs\n");
		exit(EXIT_FAILURE);
	}
	glyphs[charcount] = (Glyph){0, -1, -1, -1, -1, 0};

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	for (int c = 0; c < charcount; c++) {
		int glyphIndex = FT_Get_Char_Index(face, c);
		if (FT_Load_Glyph(face, glyphIndex, FT_LOAD_RENDER)) {
			fprintf(stderr, "Failed to load glyph: %d\n", c);
			continue;
		}

		Glyph* glyph = &glyphs[c];
		glyph->width = face->glyph->bitmap.width;
		glyph->height = face->glyph->bitmap.rows;
		glyph->bearingX = face->glyph->bitmap_left;
		glyph->bearingY = face->glyph->bitmap_top;
		glyph->advance = face->glyph->advance.x >> 6;

		glGenTextures(1, &glyph->textureID);
		glBindTexture(GL_TEXTURE_2D, glyph->textureID);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			glyph->width,
			glyph->height,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	return glyphs;
}

void getGlyphs(const char* fontPath, int fontSize, Font *font) {
	FT_Library ft;
	FT_Face face;
	
	loadFont(fontPath, &ft, &face, fontSize);
	Glyph* glyphs = generateFontTexture(face);
	font->glyphs = glyphs;
	font->lineHeight = face->height >> 6;
	font->ascender = face->ascender >> 6;
	font->descender = face->descender >> 6;
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
}

void freeGlyphs(Glyph* glyphs) {
	for (int i = 0; glyphs[i].width != -1; i++) {
		glDeleteTextures(1, &glyphs[i].textureID);
	}
	free(glyphs);
}

