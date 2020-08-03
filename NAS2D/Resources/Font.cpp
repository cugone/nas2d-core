// ==================================================================================
// = NAS2D
// = Copyright © 2008 - 2019 New Age Software
// ==================================================================================
// = NAS2D is distributed under the terms of the zlib license. You are free to copy,
// = modify and distribute the software under the terms of the zlib license.
// =
// = Acknowledgement of your use of NAS2D is appriciated but is not required.
// ==================================================================================
#include "Font.h"

#include "../Exception.h"
#include "../Filesystem.h"
#include "../Utility.h"
#include "../MathUtils.h"
#include "../Renderer/PointInRectangleRange.h"

#include <GL/glew.h>

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <iostream>
#include <cmath>
#include <algorithm>
#include <cstddef>
#include <stdexcept>


extern unsigned int generateTexture(SDL_Surface* surface);


using namespace NAS2D;
using namespace NAS2D::Exception;


std::map<std::string, Font::FontInfo> fontMap;


namespace {
	struct ColorMasks
	{
		unsigned int red;
		unsigned int green;
		unsigned int blue;
		unsigned int alpha;
	};

	constexpr ColorMasks MasksLittleEndian{0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000};
	constexpr ColorMasks MasksBigEndian{0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff};
	constexpr ColorMasks MasksDefault = (SDL_BYTEORDER == SDL_LIL_ENDIAN) ? MasksLittleEndian : MasksBigEndian;

	const int ASCII_TABLE_COUNT = 256;
	const int GLYPH_MATRIX_SIZE = 16;
	const int BITS_32 = 32;

	bool load(const std::string& path, unsigned int ptSize);
	bool loadBitmap(const std::string& path, int glyphWidth, int glyphHeight, int glyphSpace);
	Vector<int> generateGlyphMap(TTF_Font* font, const std::string& name);
	SDL_Surface* generateFontSurface(TTF_Font* font, Vector<int> characterSize);
	Vector<int> maxCharacterDimensions(const std::vector<Font::GlyphMetrics>& glyphMetricsList);
	Vector<int> roundedCharacterDimensions(Vector<int> maxSize);
	void fillInCharacterDimensions(TTF_Font* font, std::vector<Font::GlyphMetrics>& glyphMetricsList);
	void fillInTextureCoordinates(std::vector<Font::GlyphMetrics>& glyphMetricsList, Vector<int> characterSize, Vector<int> textureSize);
}


/**
 * Instantiate a Font using a TrueType or OpenType font.
 *
 * \param	filePath	Path to a font file.
 * \param	ptSize		Point size of the font. Defaults to 12pt.
 *
 */
Font::Font(const std::string& filePath, unsigned int ptSize) :
	mResourceName{filePath + "_" + std::to_string(ptSize) + "pt"}
{
	::load(filePath, ptSize);
}


/**
 * Instantiate a Font as a bitmap font.
 *
 * \param	filePath	Path to a font file.
 * \param	glyphWidth	Width of glyphs in the bitmap Font.
 * \param	glyphHeight	Height of glyphs in the bitmap Font.
 * \param	glyphSpace	Space between glyphs when rendering a bitmap font. This value can be negative.
 *
 */
Font::Font(const std::string& filePath, int glyphWidth, int glyphHeight, int glyphSpace) :
	mResourceName{filePath}
{
	loadBitmap(filePath, glyphWidth, glyphHeight, glyphSpace);
}


/**
 * Copy c'tor.
 *
 * \param	rhs	Font to copy.
 */
Font::Font(const Font& rhs) :
	mResourceName{rhs.mResourceName}
{
}


/**
* D'tor
*/
Font::~Font()
{
	glDeleteTextures(1, &fontMap[mResourceName].textureId);
}


/**
 * Copy assignment operator.
 *
 * \param rhs Font to copy.
 */
Font& Font::operator=(const Font& rhs)
{
	if (this == &rhs) { return *this; }

	glDeleteTextures(1, &fontMap[mResourceName].textureId);

	mResourceName = rhs.mResourceName;

	return *this;
}


Vector<int> Font::glyphCellSize() const
{
	return fontMap[mResourceName].glyphSize;
}


Vector<int> Font::size(std::string_view string) const
{
	return {width(string), height()};
}


/**
 * Gets the width in pixels of a string rendered using the Font.
 *
 * \param	string		String to get the width of.
 */
int Font::width(std::string_view string) const
{
	if (string.empty()) { return 0; }

	int width = 0;
	auto& gml = fontMap[mResourceName].metrics;
	if (gml.empty()) { return 0; }

	for (auto character : string)
	{
		auto glyph = std::clamp<std::size_t>(static_cast<uint8_t>(character), 0, 255);
		width += gml[glyph].advance + gml[glyph].minX;
	}

	return width;
}


/**
 * Gets the height in pixels of the Font.
 */
int Font::height() const
{
	return fontMap[mResourceName].height;
}


/**
 * The maximum pixel ascent of all glyphs in the Font.
 */
int Font::ascent() const
{
	return fontMap[mResourceName].ascent;
}


/**
 * Returns the point size of the Font.
 */
unsigned int Font::ptSize() const
{
	return fontMap[mResourceName].pointSize;
}


const std::vector<Font::GlyphMetrics>& Font::metrics() const
{
	return fontMap[mResourceName].metrics;
}


unsigned int Font::textureId() const
{
	return fontMap[mResourceName].textureId;
}


namespace {
	/**
	 * Loads a TrueType or OpenType font from a file.
	 *
	 * \param	path	Path to the TTF or OTF font file.
	 * \param	ptSize	Point size to use when loading the font.
	 */
	bool load(const std::string& path, unsigned int ptSize)
	{
		std::string fontname = path + "_" + std::to_string(ptSize) + "pt";

		if (TTF_WasInit() == 0)
		{
			if (TTF_Init() != 0)
			{
				throw std::runtime_error("Font load function failed: " + std::string{TTF_GetError()});
			}
		}

		File fontBuffer = Utility<Filesystem>::get().open(path);
		if (fontBuffer.empty())
		{
			throw std::runtime_error("Font file is empty: " + path);
		}

		TTF_Font *font = TTF_OpenFontRW(SDL_RWFromConstMem(fontBuffer.raw_bytes(), static_cast<int>(fontBuffer.size())), 0, static_cast<int>(ptSize));
		if (!font)
		{
			throw std::runtime_error("Font load function failed: " + std::string{TTF_GetError()});
		}

		fontMap[fontname].pointSize = ptSize;
		fontMap[fontname].height = TTF_FontHeight(font);
		fontMap[fontname].ascent = TTF_FontAscent(font);
		fontMap[fontname].glyphSize = generateGlyphMap(font, fontname);
		TTF_CloseFont(font);

		return true;
	}


	/**
	 * Internal function that loads a bitmap font from an file.
	 *
	 * \param	path		Path to the image file.
	 * \param	glyphWidth	Width of glyphs in the bitmap font.
	 * \param	glyphHeight	Height of the glyphs in the bitmap font.
	 * \param	glyphSpace	Spacing to use when drawing glyphs.
	 */
	bool loadBitmap(const std::string& path, int glyphWidth, int glyphHeight, int glyphSpace)
	{
		File fontBuffer = Utility<Filesystem>::get().open(path);
		if (fontBuffer.empty())
		{
			throw std::runtime_error("Font file is empty: " + path);
		}

		SDL_Surface* fontSurface = IMG_Load_RW(SDL_RWFromConstMem(fontBuffer.raw_bytes(), static_cast<int>(fontBuffer.size())), 0);
		if (!fontSurface)
		{
			throw std::runtime_error("Font loadBitmap function failed: " + std::string{SDL_GetError()});
		}

		const auto fontSurfaceSize = Vector{fontSurface->w, fontSurface->h};
		const auto glyphSize = Vector{glyphWidth, glyphHeight};
		const auto expectedSize = glyphSize * GLYPH_MATRIX_SIZE;
		if (fontSurfaceSize != expectedSize)
		{
			SDL_FreeSurface(fontSurface);
			const auto vectorToString = [](auto vector) { return "{" + std::to_string(vector.x) + ", " + std::to_string(vector.y) + "}"; };
			throw font_invalid_glyph_map("Unexpected font image size. Expected: " + vectorToString(expectedSize) + " Actual: " + vectorToString(fontSurfaceSize));
		}

		auto& fontInfo = fontMap[path];
		auto& glm = fontInfo.metrics;
		glm.resize(ASCII_TABLE_COUNT);
		for (auto& metrics : glm)
		{
			metrics.minX = glyphSize.x;
			metrics.advance = glyphSpace;
		}
		fillInTextureCoordinates(glm, glyphSize, fontSurfaceSize);

		unsigned int textureId = generateTexture(fontSurface);

		// Add generated texture id to texture ID map.
		fontInfo.textureId = textureId;
		fontInfo.pointSize = static_cast<unsigned int>(glyphSize.y);
		fontInfo.height = glyphSize.y;
		fontInfo.glyphSize = glyphSize;
		SDL_FreeSurface(fontSurface);

		return true;
	}


	/**
	 * Generates a glyph map of all ASCII standard characters from 0 - 255.
	 *
	 * Internal function used to generate a glyph texture map from an TTF_Font struct.
	 */
	Vector<int> generateGlyphMap(TTF_Font* font, const std::string& name)
	{
		auto& glm = fontMap[name].metrics;

		fillInCharacterDimensions(font, glm);

		const auto charBoundsSize = maxCharacterDimensions(glm);
		const auto roundedCharSize = roundedCharacterDimensions(charBoundsSize);
		const auto roundedMatrixSize = roundedCharSize * GLYPH_MATRIX_SIZE;

		fillInTextureCoordinates(glm, roundedCharSize, roundedMatrixSize);

		SDL_Surface* fontSurface = generateFontSurface(font, roundedCharSize);
		unsigned int textureId = generateTexture(fontSurface);

		// Add generated texture id to texture ID map.
		fontMap[name].textureId = textureId;
		SDL_FreeSurface(fontSurface);

		return roundedCharSize;
	}


	SDL_Surface* generateFontSurface(TTF_Font* font, Vector<int> characterSize)
	{
		const auto matrixSize = characterSize * GLYPH_MATRIX_SIZE;
		SDL_Surface* fontSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, matrixSize.x, matrixSize.y, BITS_32, MasksDefault.red, MasksDefault.green, MasksDefault.blue, MasksDefault.alpha);

		SDL_Color white = { 255, 255, 255, 255 };
		for (const auto glyphPosition : PointInRectangleRange(Rectangle{0, 0, GLYPH_MATRIX_SIZE, GLYPH_MATRIX_SIZE}))
		{
			const std::size_t glyph = static_cast<std::size_t>(glyphPosition.y) * GLYPH_MATRIX_SIZE + glyphPosition.x;

			// Avoid glyph 0, which has size 0 for some fonts
			// SDL_TTF will produce errors for a glyph of size 0
			if (glyph == 0) { continue; }

			SDL_Surface* characterSurface = TTF_RenderGlyph_Blended(font, static_cast<uint16_t>(glyph), white);
			if (!characterSurface)
			{
				SDL_FreeSurface(fontSurface);
				throw std::runtime_error("Font::generateGlyphMap(): " + std::string(TTF_GetError()));
			}

			SDL_SetSurfaceBlendMode(characterSurface, SDL_BLENDMODE_NONE);
			const auto pixelPosition = glyphPosition.skewBy(characterSize);
			SDL_Rect rect = { pixelPosition.x, pixelPosition.y, 0, 0 };
			SDL_BlitSurface(characterSurface, nullptr, fontSurface, &rect);
			SDL_FreeSurface(characterSurface);
		}

		return fontSurface;
	}


	Vector<int> maxCharacterDimensions(const std::vector<Font::GlyphMetrics>& glyphMetricsList)
	{
		Vector<int> size{0, 0};

		for (const auto metrics : glyphMetricsList)
		{
			size.x = std::max({size.x, metrics.minX + metrics.maxX, metrics.advance});
			size.y = std::max({size.y, metrics.minY + metrics.maxY});
		}
		return size;
	}


	Vector<int> roundedCharacterDimensions(Vector<int> maxSize)
	{
		const auto maxSizeUint32 = maxSize.to<uint32_t>();
		const auto roundedUpSize = Vector{roundUpPowerOf2(maxSizeUint32.x), roundUpPowerOf2(maxSizeUint32.y)};
		return roundedUpSize.to<int>();
	}


	void fillInCharacterDimensions(TTF_Font* font, std::vector<Font::GlyphMetrics>& glyphMetricsList)
	{
		// Build table of character sizes
		for (Uint16 i = 0; i < ASCII_TABLE_COUNT; i++)
		{
			auto& metrics = glyphMetricsList.emplace_back();
			TTF_GlyphMetrics(font, i, &metrics.minX, &metrics.maxX, &metrics.minY, &metrics.maxY, &metrics.advance);
		}
	}


	void fillInTextureCoordinates(std::vector<Font::GlyphMetrics>& glyphMetricsList, Vector<int> characterSize, Vector<int> textureSize)
	{
		const auto floatTextureSize = textureSize.to<float>();
		const auto uvSize = characterSize.to<float>().skewInverseBy(floatTextureSize);
		for (const auto glyphPosition : PointInRectangleRange(Rectangle{0, 0, GLYPH_MATRIX_SIZE, GLYPH_MATRIX_SIZE}))
		{
			const std::size_t glyph = static_cast<std::size_t>(glyphPosition.y) * GLYPH_MATRIX_SIZE + glyphPosition.x;
			const auto uvStart = glyphPosition.skewBy(characterSize).to<float>().skewInverseBy(floatTextureSize);
			glyphMetricsList[glyph].uvRect = Rectangle<float>::Create(uvStart, uvSize);
		}
	}
}
