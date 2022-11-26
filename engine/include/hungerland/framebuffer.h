#pragma once
#include <vector>
#include <memory>

namespace hungerland {
namespace texture {
	class Texture;
}
namespace graphics {

	/**
	 * Class for FrameBuffer.
	 *
	 * Framebuffer consists of several textures (render textures). Each texture must be same size.
	 *
	 * @ingroup engine
	 * @author Mikko Romppainen (mikko.romppainen@kajak.fi)
	 */
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
