#pragma once
#include <glad/gl.h>		// Include glad
#include <vector>
#include <memory>

namespace hungerland {

	class Texture;

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
		/**
		 * Default constructor.
		 */
		FrameBuffer();

		/**
		 * Destructor.
		 */
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
		 * @param	index	Index where to add texture.
		 * @param	tex		Texture to be added.
		 */
		void addColorTexture( int index, std::shared_ptr<Texture> tex );

		void setDepthTexture(std::shared_ptr<Texture> tex);

		/**
		 * Returns texture by index, which is added by addTexture.
		 *
		 * @param	index	Index where to return texture.
		 *
		 */
		std::shared_ptr<Texture> getTexture(int index) const;

	private:
		void bind();
		void unbind();
		std::vector< std::shared_ptr<Texture> >		m_textures;
		std::vector<GLenum>			m_drawBuffers;
		GLuint						m_fboId;
		unsigned int                m_rboId;

		// Non-allowed methods (declared but not defined anywhere, result link error if used)
		FrameBuffer( const FrameBuffer& );
		FrameBuffer& operator =( const FrameBuffer& );
	};

} // End - mikroplot
