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
	void Mesh::setVBOData(int index, const std::vector<float>& data, size_t numComponents) {
		glBindVertexArray(vao);
		checkGLError();

		glBindBuffer(GL_ARRAY_BUFFER, vbos[index]);
		checkGLError();
		glBufferData(GL_ARRAY_BUFFER, data.size()*sizeof(data[0]), &data[0], GL_STATIC_DRAW);
		checkGLError();
		glVertexAttribPointer(index, numComponents, GL_FLOAT, GL_FALSE, numComponents * sizeof(float), (void*)0);
		checkGLError();
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		checkGLError();

		glBindVertexArray(0);
		checkGLError();
	}

	void Mesh::setVBOData(int index, const std::vector<glm::vec2>& data) {
		glBindVertexArray(vao);
		checkGLError();

		glBindBuffer(GL_ARRAY_BUFFER, vbos[index]);
		checkGLError();
		glBufferData(GL_ARRAY_BUFFER, data.size()*sizeof(data[0]), &data[0], GL_STATIC_DRAW);
		checkGLError();
		glVertexAttribPointer(index, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		checkGLError();
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		checkGLError();

		glBindVertexArray(0);
		checkGLError();
	}

	void Mesh::release() {
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(sizeof(vbos)/sizeof(vbos[0]), vbos);
	}

	std::shared_ptr<Mesh> create(const std::vector<glm::vec2>& positions,const std::vector<glm::vec2>& textureCoords) {
		std::unique_ptr<Mesh> res = std::make_unique<Mesh>();
		// Create VAOs and VBOs for sprite
		glGenVertexArrays(1, &res->vao);
		checkGLError();
		glGenBuffers(sizeof(res->vbos)/sizeof(res->vbos[0]), res->vbos);
		checkGLError();
		// And set data
		res->setVBOData(0,positions);
		res->setVBOData(1,textureCoords);
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
		res->setVBOData(0,positions, numPositionComponents);
		res->setVBOData(1,textureCoords, numTexCoordComponents);
		return res;
	}

	void render(const Mesh& mesh, int mode, unsigned count) {
		// Bind
		glBindVertexArray(mesh.vao);
		checkGLError();
		int numVertexArrys = sizeof(mesh.vbos)/sizeof(mesh.vbos[0]);
		for(size_t i=0; i<numVertexArrys; ++i){
			glEnableVertexAttribArray(i);
			checkGLError();
		}
		// Draw
		glDrawArrays(mode, 0, count);
		checkGLError();
		// Unbind
		for(size_t i=0; i<numVertexArrys; ++i){
			glDisableVertexAttribArray(i);
			checkGLError();
		}
		glBindVertexArray(0);
		checkGLError();
	};

} // End - namespace mesh

namespace quad {
std::shared_ptr<mesh::Mesh> create() {
	static const std::vector<glm::vec2> POSITIONS({
		glm::vec2( 0.5f, -0.5f),
		glm::vec2( 0.5f,  0.5f),
		glm::vec2(-0.5f,  0.5f),
		glm::vec2( 0.5f, -0.5f),
		glm::vec2(-0.5f,  0.5f),
		glm::vec2(-0.5f, -0.5f)
	});
	static const std::vector<glm::vec2> TEXTURE_COORDS({
		glm::vec2(1,1),
		glm::vec2(1,0),
		glm::vec2(0,0),
		glm::vec2(1,1),
		glm::vec2(0,0),
		glm::vec2(0,1)
	});
	return mesh::create(POSITIONS,TEXTURE_COORDS);
}

void render(const mesh::Mesh& mesh) {
	mesh::render(mesh, GL_TRIANGLES, 6);
};


void setPositions(mesh::Mesh& mesh, const std::vector<glm::vec2>& positions) {
	mesh.setVBOData(0,positions);
}

}  // End - namespace quad

}
