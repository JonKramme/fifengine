/***************************************************************************
 *   Copyright (C) 2005-2008 by the FIFE team                              *
 *   http://www.fifengine.de                                               *
 *   This file is part of FIFE.                                            *
 *                                                                         *
 *   FIFE is free software; you can redistribute it and/or                 *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA          *
 ***************************************************************************/

// Standard C++ library includes

// 3rd party library includes
#include <SDL.h>

// FIFE includes
// These includes are split up in two parts, separated by one empty line
// First block: files included from the FIFE root src directory
// Second block: files included from the same folder
#include "util/base/exception.h"
#include "util/math/fife_math.h"
#include "util/log/logger.h"
#include "video/devicecaps.h"

#include "renderbackendsdl.h"
#include "sdlimage.h"
#include "SDL_image.h"
#include "SDL_getenv.h"

namespace FIFE {
	static Logger _log(LM_VIDEO);

	RenderBackendSDL::RenderBackendSDL(const SDL_Color& colorkey) : RenderBackend(colorkey) {
		m_clear = true;
	}


	RenderBackendSDL::~RenderBackendSDL() {
		deinit();
	}

	const std::string& RenderBackendSDL::getName() const {
		static std::string backend_name = "SDL";
		return backend_name;
	}

	void RenderBackendSDL::init(const std::string& driver) {
		char* buf;
		if (driver != "") {
			std::string envVar = std::string("SDL_VIDEODRIVER=") + driver;
			buf = const_cast<char*>(envVar.c_str());
			putenv(buf);
		}

		if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
			throw SDLException(SDL_GetError());

		SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL); // temporary hack
	}

	void RenderBackendSDL::clearBackBuffer() {
		SDL_Rect rect;
		rect.x = 0;
		rect.y = 0;
		rect.w = getWidth();
		rect.h = getHeight();
		SDL_SetClipRect(m_screen->getSurface(), &rect);
		SDL_FillRect(m_screen->getSurface(), 0, 0x00);
	}

	Image* RenderBackendSDL::createMainScreen(const ScreenMode& mode, const std::string& title, const std::string& icon){
		unsigned int width = mode.getWidth();
		unsigned int height = mode.getHeight();
		unsigned char bitsPerPixel = mode.getBPP();
		bool fs = mode.isFullScreen();
		Uint32 flags = mode.getSDLFlags();

		if(icon != "") {
			SDL_Surface *img = IMG_Load(icon.c_str());
			if(img != NULL) {
				SDL_WM_SetIcon(img, 0);
			}
		}

		SDL_Surface* screen = NULL;

		if( 0 == bitsPerPixel ) {
			/// autodetect best mode
			unsigned char possibleBitsPerPixel[] = {16, 24, 32, 0};
			int i = 0;
			while( true ) {
				bitsPerPixel = possibleBitsPerPixel[i];
				if( !bitsPerPixel ) {
					// Last try, sometimes VideoModeOK seems to lie.
					// Try bpp=0
					screen = SDL_SetVideoMode(width, height, bitsPerPixel, flags);
					if( !screen ) {
						throw SDLException("Videomode not available");
					}
					break;
				}
				bitsPerPixel = SDL_VideoModeOK(width, height, bitsPerPixel, flags);
				if ( bitsPerPixel ) {
					screen = SDL_SetVideoMode(width, height, bitsPerPixel, flags);
					if( screen ) {
						break;
					}
				}
				++i;
			}
		} else {
			if ( !SDL_VideoModeOK(width, height, bitsPerPixel, flags) ) {
				throw SDLException("Videomode not available");
			}
			screen = SDL_SetVideoMode(width, height, bitsPerPixel, flags);
		}
		FL_LOG(_log, LMsg("RenderBackendSDL")
			<< "Videomode " << width << "x" << height
			<< " at " << int(screen->format->BitsPerPixel) << " bpp");

		SDL_WM_SetCaption(title.c_str(), NULL);

		if (!screen) {
			throw SDLException(SDL_GetError());
		}

		m_screen = new SDLImage(screen);
		return m_screen;
	}

	void RenderBackendSDL::startFrame() {
		if (m_clear) {
			clearBackBuffer();
		}
	}

	void RenderBackendSDL::endFrame() {
		SDL_Flip(m_screen->getSurface());
	}

	Image* RenderBackendSDL::createImage(SDL_Surface* surface) {
		return new SDLImage(surface);
	}

	Image* RenderBackendSDL::createImage(const uint8_t* data, unsigned int width, unsigned int height) {
		return new SDLImage(data, width, height);
	}

	bool RenderBackendSDL::putPixel(int x, int y, int r, int g, int b, int a) {
		return static_cast<SDLImage*>(m_screen)->putPixel(x, y, r, g, b, a);
	}

	void RenderBackendSDL::drawLine(const Point& p1, const Point& p2, int r, int g, int b, int a) {
		static_cast<SDLImage*>(m_screen)->drawLine(p1, p2, r, g, b, a);
	}

	void RenderBackendSDL::drawTriangle(const Point& p1, const Point& p2, const Point& p3, int r, int g, int b, int a) {
		static_cast<SDLImage*>(m_screen)->drawTriangle(p1, p2, p3, r, g, b, a);
	}

	void RenderBackendSDL::drawRectangle(const Point& p, uint16_t w, uint16_t h, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
		static_cast<SDLImage*>(m_screen)->drawRectangle(p, w, h, r, g, b, a);
	}

	void RenderBackendSDL::fillRectangle(const Point& p, uint16_t w, uint16_t h, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
		static_cast<SDLImage*>(m_screen)->fillRectangle(p, w, h, r, g, b, a);
	}

	void RenderBackendSDL::drawQuad(const Point& p1, const Point& p2, const Point& p3, const Point& p4,  int r, int g, int b, int a) {
		static_cast<SDLImage*>(m_screen)->drawQuad(p1, p2, p3, p4, r, g, b, a);
	}

	void RenderBackendSDL::drawVertex(const Point& p, const uint8_t size, int r, int g, int b, int a){
		static_cast<SDLImage*>(m_screen)->drawVertex(p, 2, r, g, b, a);
	}

}//FIFE
