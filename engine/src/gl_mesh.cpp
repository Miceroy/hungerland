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
#include <hungerland/mesh.h>
#include <glad/gl.h>		// Include glad
#include <GLFW/glfw3.h>		// Include glfw
#include <string>
#include <vector>
#include <stdexcept>
#include <stb_image_write.h>
#include <assert.h>
#include <algorithm>
#include <hungerland/graphics.h>
#include <hungerland/framebuffer.h>
#include <hungerland/shader.h>
#include <hungerland/texture.h>
#include <hungerland/texture.h>
#include <hungerland/gl_utils.h>


namespace hungerland {
namespace mesh {
	Mesh::~Mesh() {
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(sizeof(vbos)/sizeof(vbos[0]), vbos);
	}
	void Mesh::setVBOData(int index, const std::vector<float>& data, size_t numComponents, bool dynamic) {
		glBindVertexArray(vao);
		checkGLError();

		glBindBuffer(GL_ARRAY_BUFFER, vbos[index]);
		checkGLError();
		glBufferData(GL_ARRAY_BUFFER, data.size()*sizeof(data[0]), &data[0], dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
		checkGLError();
		glVertexAttribPointer(index, int(numComponents), GL_FLOAT, GL_FALSE, int(numComponents * sizeof(float)), (void*)0);
		checkGLError();
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		checkGLError();

		glBindVertexArray(0);
		checkGLError();
	}

	void Mesh::setVBOData(int index, const std::vector<glm::vec2>& data, bool dynamic) {
		glBindVertexArray(vao);
		checkGLError();

		glBindBuffer(GL_ARRAY_BUFFER, vbos[index]);
		checkGLError();
		glBufferData(GL_ARRAY_BUFFER, data.size()*sizeof(data[0]), &data[0], dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
		checkGLError();
		glVertexAttribPointer(index, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		checkGLError();
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		checkGLError();

		glBindVertexArray(0);
		checkGLError();
	}

	std::shared_ptr<Mesh> create(const std::vector<glm::vec2>& positions, const std::vector<glm::vec2>& textureCoords) {
		std::unique_ptr<Mesh> res = std::make_unique<Mesh>();
		// Create VAOs and VBOs for sprite
		glGenVertexArrays(1, &res->vao);
		checkGLError();
		glGenBuffers(sizeof(res->vbos)/sizeof(res->vbos[0]), res->vbos);
		checkGLError();
		// And set data
		res->setVBOData(0, positions);
		res->setVBOData(1, textureCoords);
		return res;
	}

	std::shared_ptr<Mesh> create(const std::vector<float>& positions, size_t numPositionComponents,
										const std::vector<float>& textureCoords, size_t numTexCoordComponents) {
		std::shared_ptr<Mesh> res = std::make_shared<Mesh>();
		// Create VAOs and VBOs for sprite
		glGenVertexArrays(1, &res->vao);
		checkGLError();
		glGenBuffers(sizeof(res->vbos)/sizeof(res->vbos[0]), res->vbos);
		checkGLError();
		// And set data
		res->setVBOData(0, positions, numPositionComponents);
		res->setVBOData(1, textureCoords, numTexCoordComponents);
		return res;
	}

	void draw(const Mesh& mesh, int mode, unsigned count) {
		// Bind
		glBindVertexArray(mesh.vao);
		checkGLError();
		int numVertexArrys = sizeof(mesh.vbos)/sizeof(mesh.vbos[0]);
		for(size_t i=0; i<numVertexArrys; ++i){
			glEnableVertexAttribArray(int(i));
			checkGLError();
		}
		// Draw
		glDrawArrays(mode, 0, count);
		checkGLError();
		// Unbind
		for(size_t i=0; i<numVertexArrys; ++i){
			glDisableVertexAttribArray((unsigned)i);
			checkGLError();
		}
		glBindVertexArray(0);
		checkGLError();
	};

	void setPositions(mesh::Mesh& mesh, const std::vector<glm::vec2>& positions) {
		mesh.setVBOData(0,positions);
	}

	void setTexCoords(mesh::Mesh& mesh, const std::vector<glm::vec2>& texCoords) {
		mesh.setVBOData(1, texCoords);
	}

} // End - namespace mesh

namespace quad {

	std::shared_ptr<mesh::Mesh> createSprite(float halfSizeX, float halfSizeY) {
		static const std::vector<glm::vec2> POSITIONS({
			glm::vec2( halfSizeX, -halfSizeX),
			glm::vec2( halfSizeX,  halfSizeX),
			glm::vec2(-halfSizeX,  halfSizeX),
			glm::vec2( halfSizeX, -halfSizeX),
			glm::vec2(-halfSizeX,  halfSizeX),
			glm::vec2(-halfSizeX, -halfSizeX)
		});
		static const std::vector<glm::vec2> TEXTURE_COORDS({
			glm::vec2(1,0),
			glm::vec2(1,1),
			glm::vec2(0,1),
			glm::vec2(1,0),
			glm::vec2(0,1),
			glm::vec2(0,0)
		});
		return mesh::create(POSITIONS,TEXTURE_COORDS);
	}


	std::shared_ptr<mesh::Mesh> createImage(float originX, float originY, float sizeX, float sizeY, float texScaleX, float texScaleY){
		std::vector<float> POSITIONS = {
			originX,			originY,
			originX + sizeX,	originY,
			originX,			originY + sizeY,
			originX + sizeX,	originY + sizeY,
		};
		std::vector<float> TEXTURE_COORDS = {
			0,					0,		0.0f,	0.0f,
			texScaleX,			0,		sizeX,	0.0f,
			0,			texScaleY,		0.0f,	sizeY,
			texScaleX,	texScaleY,		sizeX,	sizeY,
		};
		return mesh::create(POSITIONS, 2, TEXTURE_COORDS, 4);
	}

	std::shared_ptr<mesh::Mesh> createScreenSizeQuad(float left, float right, float bottom, float top) {
		const float sx = right-left;
		const float sy = top-bottom;

		const auto ssqTopLeft        = glm::vec2(-sx*0.5f, -sy*0.5f);
		const auto ssqTopRight       = glm::vec2(-sx*0.5f,  sy*0.5f);
		const auto ssqBottomLeft     = glm::vec2( sx*0.5f, -sy*0.5f);
		const auto ssqBottomRight    = glm::vec2( sx*0.5f,  sy*0.5f);
		const std::vector<glm::vec2> POSITIONS({
			ssqBottomLeft,
			ssqBottomRight,
			ssqTopRight,
			ssqBottomLeft,
			ssqTopRight,
			ssqTopLeft
		});
		static const std::vector<glm::vec2> TEXTURE_COORDS({
			glm::vec2(1,0),
			glm::vec2(1,1),
			glm::vec2(0,1),
			glm::vec2(1,0),
			glm::vec2(0,1),
			glm::vec2(0,0)
		});
		return mesh::create(POSITIONS, TEXTURE_COORDS);
	}

	void draw(const mesh::Mesh& mesh) {
		mesh::draw(mesh, GL_TRIANGLES, 6);
	};

	void drawImage(const mesh::Mesh& mesh) {
		mesh::draw(mesh, GL_TRIANGLE_STRIP, 4);
	};

}  // End - namespace quad

}
