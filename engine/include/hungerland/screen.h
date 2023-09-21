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
	/// @ingroup hungerland::screen
	/// @author Mikko Romppainen (kajakbros@gmail.com)
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
		void drawSprite(const glm::mat4& transform, const texture::Texture& texture, const std::vector<shader::Constant>& constants={}, const std::string& surfaceShader="", const std::string& globals="");

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
		std::shared_ptr<shader::Shader>         m_ssqShader;
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
	/// @ingroup hungerland::screen
	/// @author Mikko Romppainen (kajakbros@gmail.com)
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

