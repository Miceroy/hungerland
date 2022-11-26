#pragma once
#include <hungerland/shader.h>
#include <hungerland/math.h>

namespace hungerland {
namespace mesh {
	class Mesh;
}
namespace texture {
	class Texture;
}
namespace graphics {
	class FrameBuffer;
}

namespace screen {

	///
	/// \brief The Screen class
	///
	class Screen  {
	public:
		Screen();

		///
		/// \brief setScreen sets Orthogonal projection.
		/// \param left
		/// \param right
		/// \param bottom
		/// \param top
		/// \return
		///
		glm::mat4 setScreen(float left, float right, float bottom, float top);

		///
		/// \brief clear
		/// \param r
		/// \param g
		/// \param b
		/// \param a
		///
		void clear(float r, float g, float b, float a);

		///
		/// \brief drawSprite
		/// \param transform
		/// \param texture
		/// \param constants
		/// \param surfaceShader
		/// \param globals
		///
		void drawSprite(const std::vector< std::vector<float> >& transform, const texture::Texture* texture,  const std::vector<shader::Constant>& constants={}, const std::string& surfaceShader="", const std::string& globals="");

		///
		/// \brief drawScreenSizeQuad
		/// \param texture
		///
		void drawScreenSizeQuad(const texture::Texture& texture);

	protected:
		float                           m_left;
		float                           m_right;
		float                           m_bottom;
		float                           m_top;
		glm::mat4						m_projection;

	private:
		std::unique_ptr<graphics::FrameBuffer>	m_shadeFbo;
		std::unique_ptr<shader::Shader>         m_ssqShader;
		std::shared_ptr<mesh::Mesh>				m_ssq;
		std::shared_ptr<mesh::Mesh>				m_sprite;

	private:
		// Copy not allowed
		Screen(const Screen&) = delete;
		Screen& operator=(const Screen&) = delete;
	};

	///
	/// \brief The FrameBuffer class
	///
	class FrameBuffer : public Screen  {
	public:
		///
		/// \brief shade
		/// \param fragmentShader
		/// \param globals
		///
		void shade(const std::string& fragmentShader, const std::string& globals="");

		///
		/// \brief shade
		/// \param inputConstants
		/// \param fragmentShader
		/// \param globals
		///
		void shade(const std::vector<shader::Constant>& inputConstants, const std::string& fragmentShader, const std::string& globals="");

		///
		/// \brief getShadeTexture
		/// \return
		///
		const texture::Texture& getShadeTexture() const;

	private:
		std::unique_ptr<graphics::FrameBuffer>    m_shadeFbo;
		std::unique_ptr<shader::Shader>         m_ssqShader;
		std::unique_ptr<mesh::Mesh>     m_ssq;
		std::unique_ptr<mesh::Mesh>     m_sprite;
	};
}
} // End - hungerland

