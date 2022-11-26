#include <hungerland/graphics.h>
#include <hungerland/util.h>
#include <hungerland/gl_utils.h>
#include <glad/gl.h>
#include <assert.h>
#include <stdio.h>
#include <string>
#include <fstream>
#include <streambuf>
#include <stdexcept>

namespace hungerland {

	void checkGLError() {
		GLenum err = glGetError();
		if (err != GL_NO_ERROR) {
			auto getErrorString = [](GLenum err) -> const char* const {
				switch (err) {
				case GL_NO_ERROR: return "GL_NO_ERROR";
				case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
				case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
				case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
				case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
				case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
				default: return "Unknown error!";
				}
			};
			printf("OpenGL Error (%d): \"%s\"\n", (int)err, getErrorString(err));
			util::ERROR("OpenGL Error ("+std::to_string(err) + "): " + getErrorString(err));
		}
	}



	/*namespace mesh {
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
	*/

}
