/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 MIT License

 Copyright (c) 2022 Mikko Romppainen

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
#include <hungerland/screen.h>
#include <hungerland/graphics.h>
#include <hungerland/framebuffer.h>
#include <hungerland/texture.h>
#include <hungerland/mesh.h>
#include <hungerland/gl_utils.h>
#include <glad/gl.h>		// Include glad


namespace hungerland {
namespace screen {

	Screen::Screen()
		: m_left(0)
		, m_right(0)
		, m_bottom(0)
		, m_top(0)
		, m_shadeFbo() {
		m_ssqShader = std::make_unique<shader::Shader>(graphics::projectionVSSource(), graphics::textureFSSource("","",""));

		glEnable(GL_BLEND);
		checkGLError();
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		checkGLError();

		// Create sprite and screen size quad meshes
		m_sprite = quad::create();
		m_ssq = quad::create();
	}

	void Screen::clear(float r, float g, float b, float a) {
		glClearColor(r, g, b, a);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		m_shadeFbo->use([&]() {
			glClearColor(r, g, b, a);
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		});
	}

	glm::mat4 Screen::setScreen(float left, float right, float bottom, float top) {
		if(m_left==left && m_right==right && m_bottom==bottom && m_top==top){
			return m_projection;
		}
		const float m_near = -1.0f;
		const float m_far = 1.0f;
		m_left = left;
		m_right = right;
		m_bottom = bottom;
		m_top = top;

		const float sx = right-left;
		const float sy = top-bottom;

		const auto ssqTopLeft        = glm::vec2(-sx*0.5f, -sy*0.5f);
		const auto ssqTopRight       = glm::vec2(-sx*0.5f,  sy*0.5f);
		const auto ssqBottomLeft     = glm::vec2( sx*0.5f, -sy*0.5f);
		const auto ssqBottomRight    = glm::vec2( sx*0.5f,  sy*0.5f);
		const std::vector<glm::vec2> screenSizeQuad({
			ssqBottomLeft,
			ssqBottomRight,
			ssqTopRight,
			ssqBottomLeft,
			ssqTopRight,
			ssqTopLeft
		});

		quad::setPositions(*m_ssq, screenSizeQuad);

		m_projection = {
			2.0f/(m_right-m_left),  0.0f,                       0.0f,                   0.0f,
			0.0f,                   2.0f/(m_top-m_bottom),      0.0f,                   0.0f,
			0.0f,                   0.0f,                      -2.0f/(m_far-m_near),    0.0f,
			0.0f,                   0.0f,                       0.0f,                   1.0f
		};

		// Create FBOs
		m_shadeFbo = std::make_unique<graphics::FrameBuffer>();
		m_shadeFbo->addColorTexture(0, std::make_shared<texture::Texture>(std::abs(sx), std::abs(sy), false));

		m_ssqShader->use([&](shader::ShaderPass shader) {
			shader.setUniform("texture0", 0);
			shader.setUniformm("P", &m_projection[0][0]);
		});
		return m_projection;
	}

	void Screen::drawSprite(const std::vector< std::vector<float> >& transf, const texture::Texture* texture, const std::vector<shader::Constant>& constants, const std::string& surfaceShader, const std::string& globals) {
		std::vector<float> matModel;
		for(size_t y=0; y<transf.size(); ++y){
			for(size_t x=0; x<transf[y].size(); ++x){
				matModel.push_back(transf[y][x]);
			}
		}

		shader::Shader spriteShader(graphics::modelProjectionVSSource(), graphics::textureFSSource("", globals, surfaceShader));
		spriteShader.use([&](shader::ShaderPass shader) {
			shader.setUniformm("P", &m_projection[0][0]);
			shader.setUniformm("M", &matModel[0]);
			shader.setUniform("texture0", 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture->getId());
			for(auto& c : constants){
				shader.setUniformv(c.first,c.second);
			}
			// Draw sprite
			quad::render(*m_sprite);
		});
	}

	void Screen::drawScreenSizeQuad(const texture::Texture& texture) {
		m_ssqShader->use([&](shader::ShaderPass shader) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture.getId());
			// Render screen size quad
			quad::render(*m_ssq);
		});
	}

	void FrameBuffer::shade(const std::string& fragmentShaderMain, const std::string& globals){
		shade(std::vector<shader::Constant>(), fragmentShaderMain, globals);
	}

	void FrameBuffer::shade(const std::vector<shader::Constant>& inputConstants, const std::string& fragmentShaderMain, const std::string& globals) {
		shader::Shader shadeShader(graphics::shadeVSSource(), graphics::shadeFSSource(shader::to_string(inputConstants), globals, fragmentShaderMain));
		m_shadeFbo->use([&](){
			shadeShader.use([&](shader::ShaderPass shader) {
				shader.setUniformm("M", &m_projection[0][0]);
				auto maxX = std::max(m_right, m_left);
				auto minX = std::min(m_right, m_left);
				auto maxY = std::max(m_top, m_bottom);
				auto minY = std::min(m_top, m_bottom);
				shader.setUniformv("leftBottom", {m_left, m_bottom});
				shader.setUniformv("rightTop", {m_right, m_top});
				shader.setUniform("min", minX, minY);
				shader.setUniform("max", maxX, maxY);
				shader.setUniform("size", maxX-minX, maxY-minY);
				for(auto& c : inputConstants){
					shader.setUniformv(c.first, c.second);
				}
				// Render screen size quad
				quad::render(*m_ssq);
			});
		});
	}

	const texture::Texture& FrameBuffer::getShadeTexture() const {
		return m_shadeFbo->getTexture(0);
	}

}
}
