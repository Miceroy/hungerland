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
#include <vector>
#include <memory>

namespace hungerland {
namespace texture {
	class Texture;
}
namespace graphics {

	///
	/// \brief The hungerland::engine::FrameBuffer class
	///
	/// Framebuffer consists of several textures (render textures). Each texture must be same size.
	///
	/// @ingroup hungerland::graphics
	/// @author Mikko Romppainen (kajakbros@gmail.com)
	///
	class FrameBuffer
	{
	public:
		FrameBuffer();
		virtual ~FrameBuffer();

		template<typename F>
		void use(F f){
			bind();
			f();
			unbind();
		}

		/**
		 * Adds new output texture to framebuffer.
		 *
		 * There is no error check for adding of textures to same index, so be carefull not to
		 * add textures with same index.
		 *
		 * @param	index		Index where to add texture.
		 * @param	texture		Texture to be added.
		 */
		void addColorTexture( int index, std::shared_ptr<texture::Texture> texture);

		///
		/// \brief setDepthTexture
		/// \param texture
		///
		void setDepthTexture(std::shared_ptr<texture::Texture> texture);

		/**
		 * Returns texture by index, which is added by addTexture.
		 *
		 * @param	index		Index where to return texture.
		 *
		 */
		const texture::Texture& getTexture(int index) const;

	private:

		void bind();
		void unbind();
		std::vector< std::shared_ptr<texture::Texture> >		m_textures;
		std::vector<unsigned>			m_drawBuffers;
		unsigned						m_fboId;
		unsigned                 m_rboId;

		// Copy not allowed
		FrameBuffer(const FrameBuffer&) = delete;
		FrameBuffer& operator=(const FrameBuffer&) = delete;
	};
}
} // End - mikroplot
