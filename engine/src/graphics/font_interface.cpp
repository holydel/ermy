#include "font_interface.h"
#include <freetype/ttnameid.h>
#include <ermy_application.h>
#include <ermy_log.h>
#include <ermy_utils.h>
#include <fstream>
#include <iostream>

#include FT_TRUETYPE_TABLES_H
#include FT_TRUETYPE_TAGS_H

using namespace ermy;

struct FreeTypeLibraryWrapper
{
	FT_Library lib = nullptr;
	FreeTypeLibraryWrapper()
	{
		if (FT_Init_FreeType(&lib))
		{
			ERMY_ERROR(u8"Failed to initialize FreeType library.");
		}
	}

	~FreeTypeLibraryWrapper()
	{
		FT_Done_FreeType(lib);
	}
};

static FreeTypeLibraryWrapper& GetFreeTypeLibrary()
{
	static FreeTypeLibraryWrapper libraryWrapper;
	return libraryWrapper;
}

struct MSDFLibraryWrapper
{
	msdfgen::FreetypeHandle* lib = nullptr;
	MSDFLibraryWrapper()
	{
		lib = msdfgen::initializeFreetype();
	}

	~MSDFLibraryWrapper()
	{
		msdfgen::deinitializeFreetype(lib);
	}
};

static MSDFLibraryWrapper& GetMSDFLibrary()
{
	static MSDFLibraryWrapper libraryWrapper;
	return libraryWrapper;
}

const char* get_platform_name(FT_UShort platform_id) {
	switch (platform_id) {
	case TT_PLATFORM_APPLE_UNICODE: return "Apple Unicode";
	case TT_PLATFORM_MACINTOSH:     return "Macintosh";
	case TT_PLATFORM_ISO:          return "ISO";
	case TT_PLATFORM_MICROSOFT:     return "Microsoft";
	case TT_PLATFORM_CUSTOM:        return "Custom";
	case TT_PLATFORM_ADOBE:         return "Adobe";
	default:                        return "Unknown";
	}
}

const char* get_encoding_name(FT_UShort platform_id, FT_UShort encoding_id) {
	switch (platform_id) {
	case TT_PLATFORM_MICROSOFT:
		switch (encoding_id) {
		case TT_MS_ID_SYMBOL_CS:    return "Symbol";
		case TT_MS_ID_UNICODE_CS:   return "Unicode BMP";
		case TT_MS_ID_SJIS:        return "ShiftJIS";
		case TT_MS_ID_GB2312:       return "PRC";
		case TT_MS_ID_BIG_5:       return "Big5";
		case TT_MS_ID_WANSUNG:      return "Wansung";
		case TT_MS_ID_JOHAB:        return "Johab";
		case TT_MS_ID_UCS_4:        return "UCS-4";
		default:                    return "Unknown Microsoft";
		}

	case TT_PLATFORM_MACINTOSH:
		switch (encoding_id) {
		case TT_MAC_ID_ROMAN:      return "Roman";
		case TT_MAC_ID_JAPANESE:    return "Japanese";
		case TT_MAC_ID_TRADITIONAL_CHINESE:     return "Traditional Chinese";
		case TT_MAC_ID_KOREAN:      return "Korean";
		case TT_MAC_ID_ARABIC:     return "Arabic";
		case TT_MAC_ID_HEBREW:      return "Hebrew";
		case TT_MAC_ID_GREEK:       return "Greek";
		case TT_MAC_ID_RUSSIAN:    return "Russian";
		case TT_MAC_ID_SIMPLIFIED_CHINESE:     return "Simplified Chinese";
		default:                   return "Unknown Macintosh";
		}

	case TT_PLATFORM_APPLE_UNICODE:
		return "Unicode 2.0+";

	default:
		return "Unknown Encoding";
	}
}

Font* Font::CreateFromFile(const char8_t* u8filename)
{
	FontImpl* result = new FontImpl();
	result->LoadTTFFont(u8filename);

	return result;
}

FontImpl::FontImpl()
{
	//ermy::rendering::Format atlasTexelFormat = ermy::rendering::Format::UNKNOWN;
	//bool needMeshes = false;

	//switch (fontType)
	//{
	//case Font::FontType::Grayscale:
	//case Font::FontType::SDF:
	//	atlasTexelFormat = ermy::rendering::Format::R8_UNORM;
	//	break;

	//case Font::FontType::MSDFT:
	//	atlasTexelFormat = ermy::rendering::Format::RGBA8_UNORM;
	//	break;

	//case Font::FontType::OutlineMesh2D:
	//case Font::FontType::OutlineMesh3D:
	//	needMeshes = true;
	//	break;
	//default:
	//	ERMY_ERROR(u8"Unknown Font Type");
	//}

	//if (atlasTexelFormat != ermy::rendering::Format::UNKNOWN) //need texture atlas
	//{
	//	const auto& fontCfg = Application::GetApplication()->staticConfig.render.fontConfig;
	//	rendering::TextureDesc tdesc = rendering::TextureDesc::Create2D(fontCfg.initialTextureSize, fontCfg.initialTextureSize, atlasTexelFormat);
	//	atlasID = rendering::CreateDedicatedTexture(tdesc);
	//}

}

FontImpl::~FontImpl()
{
	if (font_msdf)
	{
		msdfgen::destroyFont(font_msdf);
		font_msdf = nullptr;
	}

	if (face_ft)
	{
		FT_Done_Face(face_ft);
		face_ft = nullptr;
	}
}

void extract_kerning_pairs(FT_Face face) {
	// Check if the face supports SFNT tables (TrueType/OpenType)
	if (!(face->face_flags & FT_FACE_FLAG_SFNT)) {
		printf("Font is not SFNT (TrueType/OpenType), no 'kern' table available.\n");
		return;
	}

	FT_Byte* buffer = NULL;
	FT_ULong length = 0;

	// Load 'kern' table (legacy kerning)
	FT_Error err = FT_Load_Sfnt_Table(face, TTAG_kern, 0, NULL, &length);
	if (err != 0) {
		printf("No 'kern' table found or not supported by FT_Load_Sfnt_Table (err=%d).\n", err);
		printf("Note: Many modern fonts use GPOS for kerning, which is not accessible via FT_Load_Sfnt_Table.\n");
		printf("Use FT_Get_Kerning or a shaping library (e.g., HarfBuzz) for GPOS kerning.\n");
		return;
	}
	buffer = (FT_Byte*)malloc(length);
	FT_Load_Sfnt_Table(face, TTAG_kern, 0, buffer, &length);

	// Determine version (Apple format, version 0)
	uint16_t version = (buffer[0] << 8) | buffer[1];
	if (version != 0) {
		printf("Unsupported version or Microsoft format, skipping.\n");
		free(buffer);
		return;
	}

	uint16_t nTables = (buffer[2] << 8) | buffer[3];
	uint32_t offset = 4;

	// Parse each subtable
	for (uint16_t i = 0; i < nTables; i++) {
		uint16_t length = (buffer[offset + 2] << 8) | buffer[offset + 3];
		uint16_t coverage = (buffer[offset + 4] << 8) | buffer[offset + 5];
		uint8_t format = coverage >> 8;

		if (format == 0) { // Format 0: simple pair list
			uint16_t nPairs = (buffer[offset + 6] << 8) | buffer[offset + 7];
			uint32_t pair_offset = offset + 14; // Skip header and search params

			// Extract each pair
			for (uint16_t j = 0; j < nPairs; j++) {
				uint16_t left = (buffer[pair_offset] << 8) | buffer[pair_offset + 1];
				uint16_t right = (buffer[pair_offset + 2] << 8) | buffer[pair_offset + 3];
				int16_t value = (buffer[pair_offset + 4] << 8) | buffer[pair_offset + 5];
				printf("Kerning pair: left=%u, right=%u, value=%d\n", left, right, value);
				pair_offset += 6;
			}
		}

		offset += length;
	}

	free(buffer);
}

void FontImpl::LoadTTFFont(const char8_t* u8filename)
{
	using namespace msdfgen;

	auto error = FT_New_Face(GetFreeTypeLibrary().lib, (const char*)u8filename, 0, &face_ft);
	if (error) {
		ERMY_ERROR(u8"Failed to load font face.");
		return;
	}

	int FONT_SIZE = 64;
	error = FT_Set_Char_Size(face_ft, 0, FONT_SIZE * 64, 300, 300);
	if (error) {
		ERMY_ERROR(u8"Failed to set character size.");
		return;
	}

	for (int i = 0; i < face_ft->num_charmaps; i++) {
		FT_CharMap charmap = face_ft->charmaps[i];

		ERMY_LOG(u8"Font charmap %d: Platform: %s, Encoding: %s", i, get_platform_name(charmap->platform_id), get_encoding_name(charmap->platform_id, charmap->encoding_id));
	}

	error = FT_Select_Charmap(face_ft, FT_ENCODING_UNICODE);
	if (error)
	{
		ERMY_ERROR(u8"Font doesn't have unicode charmap!");
	}
	int numChars = face_ft->num_glyphs;



	ERMY_LOG(u8"Loaded font face: %s, Num CharMaps: %d, Num Chars: %d", face_ft->family_name, face_ft->num_charmaps, numChars);


	font_msdf = msdfgen::adoptFreetypeFont(face_ft);

	msdfgen::FontMetrics metrics = {};
	getFontMetrics(metrics, font_msdf, msdfgen::FontCoordinateScaling::FONT_SCALING_EM_NORMALIZED);

	ERMY_LOG(u8"Font metrics: ascenderY: %f, descenderY: %f, emSize: %f, lineHeight: %f, underlineY: %f", metrics.ascenderY, metrics.descenderY, metrics.emSize, metrics.lineHeight, metrics.underlineY);

	extract_kerning_pairs(face_ft);
}

ermy::u32 FontImpl::GetNumberOfGlyphs()
{
	if (face_ft)
		return (ermy::u32)face_ft->num_glyphs;

	return 0;
}

template <typename T>
bool saveVectorToRawFile(const std::vector<T>& vec, const std::string& filename) {
	std::ofstream outfile(filename, std::ios::binary);
	if (!outfile) {
		//std::cerr << "Failed to open file: " << filename << std::endl;
		return false;
	}

	// Write raw bytes of the vector's data
	outfile.write(reinterpret_cast<const char*>(vec.data()), vec.size() * sizeof(T));

	if (outfile.fail()) {
		//std::cerr << "Error writing to file" << std::endl;
		return false;
	}

	outfile.close();
	return true;
}

std::vector<ermy::u8> FontImpl::GeneratePreviewRGBA(int width, int height)
{
	std::vector<ermy::u8> result(width * height * 4, 0);

	char32_t glyphsToPreview[] = { U'0', U'3', U'7', U'8',
								   U'A', U'B', U'V', U'M',
								   U'b', U'm', U'q', U'p',
								   U'©', U'#', U'&', U'~' };

	int glyphWidth = width / 4;
	int glyphHeight = height / 4;

	FT_Set_Pixel_Sizes(face_ft, 32, 32);

	for (int r = 0; r < 4; ++r)
	{
		for (int c = 0; c < 4; ++c)
		{
			char32_t glyph = glyphsToPreview[r * 4 + c];
			auto glyphIndex = FT_Get_Char_Index(face_ft, glyph);

			// Load the glyph
			FT_Error error = FT_Load_Glyph(face_ft, glyphIndex, FT_LOAD_RENDER);
			if (error)
			{
				ERMY_ERROR(u8"Failed to load glyph for preview.");
				continue;
			}

			// Access the glyph's bitmap
			FT_Bitmap& bitmap = face_ft->glyph->bitmap;

			// Calculate the position in the preview grid
			int startX = c * glyphWidth + (glyphWidth - bitmap.width) * 0.5f;
			int startY = r * glyphHeight + (glyphHeight - bitmap.rows) * 0.5f;

			// Copy the glyph's bitmap into the RGBA buffer
			for (int y = 0; y < bitmap.rows; ++y)
			{
				for (int x = 0; x < bitmap.width; ++x)
				{
					int bufferX = startX + x;
					int bufferY = startY + y;

					if (bufferX < width && bufferY < height)
					{
						int bufferIndex = (bufferY * width + bufferX) * 4;
						ermy::u8 value = bitmap.buffer[y * bitmap.pitch + x];

						// Set RGBA values (grayscale to RGBA)
						result[bufferIndex + 0] = value; // R
						result[bufferIndex + 1] = value; // G
						result[bufferIndex + 2] = value; // B
						result[bufferIndex + 3] = 255;   // A
					}
				}
			}
		}
	}

	return result;
}

AtlasImpl::AtlasImpl(FontImpl* hostFont, ermy::Font::AtlasType type, int glyphSizeIn)
{
	font = hostFont;
	atlasType = type;
	glyphSize = glyphSizeIn;
}

AtlasImpl::~AtlasImpl()
{

}
void AtlasImpl::FillTexCoords(const char8_t* u8string, std::vector<ermy::Font::GlyphAtlasData>& out)
{
	out.clear();

	std::vector<char32_t> codepoints = ermy_utils::string::getCodepoints(u8string);

	for (char32_t cp : codepoints)
	{
	}
}

ermy::Font::FullAtlasInfo AtlasImpl::GenerateFullAtlas()
{
	using namespace msdfgen;
	auto face_ft = font->face_ft;
	auto font_msdf = font->font_msdf;

	ermy::Font::FullAtlasInfo result;

	u32 glyphsCount = font->GetNumberOfGlyphs();

	u64 numPixels = glyphSize * glyphSize * glyphsCount;

	u32 width = ermy_utils::math::alignUpPow2(sqrt(numPixels));
	u32 alignedGlyphSize = ermy_utils::math::alignUp(glyphSize, 4);

	u32 height = ermy_utils::math::alignUp(numPixels / width, alignedGlyphSize);

	result.width = width;
	result.height = height;

	int pixelSize = 1;
	if (atlasType == ermy::Font::AtlasType::MSDFT)
	{
		pixelSize = 4;
	}

	result.pixelsData.resize(width * height * pixelSize);

	FT_ULong charcode;
	FT_UInt gindex;

	charcode = FT_Get_First_Char(face_ft, &gindex);
	int realCharacters = 0;
	int visibleCharacters = 0;
	std::vector<char32_t> availableCharacters;
	availableCharacters.reserve(glyphsCount);

	Shape shape;

	float outX = 0;
	float outY = 0;
	//face_ft->size->generic.
	while (gindex != 0) {
		availableCharacters.push_back(charcode);
		if (loadGlyph(shape, font_msdf, charcode, FONT_SCALING_EM_NORMALIZED)) {
			if (!shape.contours.empty())
			{
				auto glyphMetrics = face_ft->glyph->metrics;

				shape.normalize();
				//                      max. angle
				edgeColoringSimple(shape, 3.0);
				//          output width, height
				Bitmap<float, 4> msdf(glyphSize, glyphSize);
				//                            scale, translation (in em's)
				SDFTransformation t(Projection(float(glyphSize), Vector2(0.125, 0.125)), Range(0.125));
				generateMTSDF(msdf, shape, t);

				for (int y = 0; y < glyphSize; y++)
				{
					int outRowOffset = (outY + glyphSize) * width;

					for (int x = 0; x < glyphSize; x++)
					{
						int pixelOffset = (outRowOffset + x + outX) * pixelSize;

						if (atlasType == ermy::Font::AtlasType::MSDFT)
						{
							result.pixelsData[pixelOffset + 0] = (std::byte)pixelFloatToByte(msdf(x, y)[0]);
							result.pixelsData[pixelOffset + 1] = (std::byte)pixelFloatToByte(msdf(x, y)[1]);
							result.pixelsData[pixelOffset + 2] = (std::byte)pixelFloatToByte(msdf(x, y)[2]);
							result.pixelsData[pixelOffset + 3] = (std::byte)pixelFloatToByte(msdf(x, y)[3]);
						}
						else if (atlasType == ermy::Font::AtlasType::SDF)
						{
							result.pixelsData[pixelOffset + 0] = (std::byte)pixelFloatToByte(msdf(x, y)[0]);
						}
					}
				}

				outX += glyphSize;
				if (outX > (width - glyphSize))
				{
					outX = 0;
					outY += glyphSize;
				}

				visibleCharacters++;
			}

			printf("%d/%d\n", realCharacters, glyphsCount);
		}


		charcode = FT_Get_Next_Char(face_ft, charcode, &gindex);
		realCharacters++;
	}

	ermy_utils::image::SaveImage("D:/temp/test.png", (const u8*)result.pixelsData.data(), width, height, pixelSize);
	//saveVectorToRawFile(result.pixelsData, "mtsdf.raw");
	//save result.pixelsData to D:/Temp raw


	return result;
}

VertexCacheImpl::VertexCacheImpl(FontImpl* hostFont)
{
	font = hostFont;
}

VertexCacheImpl::~VertexCacheImpl()
{

}

void VertexCacheImpl::FillMeshData(const char8_t* u8string, ermy::Font::MeshType meshType, std::vector<ermy::Font::GlyphMeshData>& out)
{
	out.clear();

	std::vector<char32_t> codepoints = ermy_utils::string::getCodepoints(u8string);

	for (char32_t cp : codepoints)
	{
	}
}

ermy::Font::FullVertexCacheInfo VertexCacheImpl::GenerateFullVertexCache()
{
	ermy::Font::FullVertexCacheInfo result;

	return result;
}