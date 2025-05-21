#include "font_interface.h"
#include <freetype/ttnameid.h>
#include <ermy_application.h>
#include <ermy_log.h>
#include <ermy_utils.h>

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

Font* Font::CreateFromFile(const char8_t* u8filename, Font::AtlasType atlasType)
{
	FontImpl* result = new FontImpl(atlasType);
	result->LoadTTFFont(u8filename);
	
	return result;
}

FontImpl::FontImpl(Font::AtlasType atlasTypeIn)
	:atlasType(atlasTypeIn)
{
	ermy::rendering::Format atlasTexelFormat = ermy::rendering::Format::RGBA8_UNORM;
	switch (atlasType)
	{
	case Font::AtlasType::Grayscale:
	case Font::AtlasType::SDF:
		atlasTexelFormat = ermy::rendering::Format::R8_UNORM;
		break;

	case Font::AtlasType::MSDFT:
		atlasTexelFormat = ermy::rendering::Format::RGBA8_UNORM;
		break;
	}

	const auto& fontCfg = Application::GetApplication()->staticConfig.render.fontConfig;

	rendering::TextureDesc tdesc = rendering::TextureDesc::Create2D(fontCfg.initialTextureSize, fontCfg.initialTextureSize, atlasTexelFormat);
	atlasID = rendering::CreateDedicatedTexture(tdesc);
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

rendering::TextureID FontImpl::GetAtlas()
{
	return atlasID;
}

void FontImpl::FillTexCoords(const char8_t* u8string, std::vector<Font::GlyphData>& out)
{
	using namespace msdfgen;

	out.clear();

	//parse char8_t on unicode_t
	std::vector<char32_t> codepoints = ermy_utils::string::getCodepoints(u8string);

	for (char32_t cp : codepoints)
	{
		//try found in cache if miss - render
		

		//if (loadGlyph(shape, font_msdf, cp, FONT_SCALING_EM_NORMALIZED)) {
		//	shape.normalize();
		//	//                      max. angle
		//	edgeColoringSimple(shape, 3.0);
		//	//          output width, height
		//	Bitmap<float, 4> msdf(64, 64);
		//	//                            scale, translation (in em's)
		//	SDFTransformation t(Projection(32.0, Vector2(0.125, 0.125)), Range(0.125));
		//	generateMTSDF(msdf, shape, t);
		//}
	}

	int a = 42;
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


	if (atlasType == Font::AtlasType::MSDFT || atlasType == Font::AtlasType::SDF)
	{
		font_msdf = msdfgen::adoptFreetypeFont(face_ft);
	}
}

ermy::u32 FontImpl::GetNumberOfGlyphs()
{
	if (face_ft)
		return (ermy::u32)face_ft->num_glyphs;

	return 0;
}

ermy::Font::FullAtlasInfo FontImpl::GenerateFullAtlas(int glyphSize)
{
	using namespace msdfgen;

	ermy::Font::FullAtlasInfo result;

	u32 glyphsCount = GetNumberOfGlyphs();

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

	result.pixelsData.resize(numPixels * pixelSize);

	FT_ULong charcode;
	FT_UInt gindex;

	charcode = FT_Get_First_Char(face_ft, &gindex);
	int realCharacters = 0;
	std::vector<char32_t> availableCharacters;
	availableCharacters.reserve(glyphsCount);

	Shape shape;

	while (gindex != 0) {
		availableCharacters.push_back(charcode);
		if (loadGlyph(shape, font_msdf, charcode, FONT_SCALING_EM_NORMALIZED)) {
			shape.normalize();
			//                      max. angle
			edgeColoringSimple(shape, 3.0);
			//          output width, height
			Bitmap<float, 4> msdf(glyphSize, glyphSize);
			//                            scale, translation (in em's)
			SDFTransformation t(Projection(float(glyphSize), Vector2(0.125, 0.125)), Range(0.125));
			generateMTSDF(msdf, shape, t);

			printf("%d/%d\n", realCharacters, glyphsCount);
		}


		charcode = FT_Get_Next_Char(face_ft, charcode, &gindex);
		realCharacters++;
	}

	return result;
}