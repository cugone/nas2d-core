// ==================================================================================
// = NAS2D
// = Copyright © 2008 - 2014 New Age Software
// ==================================================================================
// = NAS2D is distributed under the terms of the zlib license. You are free to copy,
// = modify and distribute the software under the terms of the zlib license.
// = 
// = Acknowledgement of your use of NAS2D is appriciated but is not required.
// ==================================================================================

#ifndef _NAS_IMAGERESOURCE_
#define _NAS_IMAGERESOURCE_

#include "Resource.h"

#ifdef __APPLE__
#include "SDL2_image/SDL_image.h"
#elif __linux__
#include "SDL2/SDL_image.h"
#elif _WIN32
#include "GL/glew.h"
#define NO_SDL_GLEXT
#include "SDL_image.h"
#endif

#include <vector>
#include <map>

namespace NAS2D {

class Rectangle_2d;


/**
 * \class Image
 * \brief Image Class
 *
 * Stores an image as a representation of pixels in memory.
 */
class Image: public Resource
{
public:
	Image(const std::string& filePath);
	Image(void* buffer, int bytesPerPixel, int width, int height);
	Image(int width, int height);
	Image();

	Image(const Image &src);
	Image& operator=(const Image& rhs);

	~Image();

	int width() const;
	int height() const;

	const Rectangle_2d& rect() const;

	Color_4ub pixelColor(int x, int y) const;

	void desaturate();

protected:
	friend class Renderer;
	friend class OGL_Renderer;

	unsigned int texture_id() const;
	unsigned int fbo_id();

private:

	struct ImageInfo
	{
		ImageInfo(): pixels_raw(nullptr), textureId(0), fboId(0), w(0), h(0), ref_count(0) {}
		ImageInfo(unsigned int id, unsigned int fbo_id, int w, int h): pixels_raw(nullptr), textureId(id), fboId(fbo_id), w(w), h(h), ref_count(0) {}
		
        //void operator()(unsigned int id, unsigned int fbo_id, int w, int h) { textureId = id; fboId = fbo_id; w = w; h = h; }

		SDL_Surface*	pixels_raw;

		unsigned int textureId;
		unsigned int fboId;
		int w;
		int h;

		int ref_count;
	};

	typedef std::map<std::string, ImageInfo> TextureIdMap;

	void load();
	void loadDefault();
	void generateTexture(void* buffer, int bytesPerPixel, int width, int height);

	bool checkTextureId();

	Rectangle_2d	mRect;			/**< Width/Height information about the Image. */

	unsigned int	mTextureId;		/**< OpenGL Texture ID. */

	static TextureIdMap			_IdMap;		/*< Lookup table for OpenGL Texture ID's. */
	static int					_Arbitrary;	/*< Counter for arbitrary image ID's. */
};


/**
 * \typedef	ImageListPtr
 * \brief	A list of pointers to Image objects.
 */
typedef std::vector<Image*> ImageListPtr;


/**
 * \typedef	ImageList
 * \brief	A list of Image objects.
 */
typedef std::vector<Image> ImageList;

} // namespace

#endif
