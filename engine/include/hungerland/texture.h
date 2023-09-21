/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 MIT License

 Copyright (c) 2022 Mikko Romppainen (kajakbros@gmail.com)

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/
#pragma once
#include <hungerland/types.h>

namespace hungerland {
namespace texture {

	///
	/// \brief The Texture class
	///
	/// @ingroup hungerland::texture
	/// @author Mikko Romppainen (kajakbros@gmail.com)
	///
	class Texture {
	public:
		typedef std::shared_ptr<Texture> Ref;
		Texture(unsigned width, unsigned height, unsigned nrChannels);
		Texture(unsigned width, unsigned height, unsigned nrChannels, const uint8_t* data);
		Texture(unsigned width, unsigned height, unsigned nrChannels, const float* data);
		Texture(unsigned width, unsigned height, bool isDepthTexture);
		~Texture();

		void setData(unsigned width, unsigned height, unsigned nrChannels, const float* data);
		void setData(unsigned width, unsigned height, unsigned nrChannels, const uint8_t* data);
		void setRepeat(bool repeat);
		void setFiltering(bool filter);

		void bind(unsigned textureIndex);

		unsigned getId() const;
		unsigned getWidth() const;
		unsigned getHeight() const;

	private:
		unsigned	m_textureId;	// Texture id
		unsigned	m_width;
		unsigned	m_height;
		unsigned	m_nrChannels;

		// Copy not allowed
		Texture() = delete;
		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;
	};
}
}
