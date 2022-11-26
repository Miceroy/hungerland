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
#pragma once
#include <hungerland/math.h>
#include <hungerland/shader.h>
#include <memory>
#include <string>
#include <vector>
#include <memory>

namespace hungerland {
	namespace graphics {

		static inline std::string projectionVSSource(){
			return
				std::string("#version 330 core\n") +
				std::string("layout (location = 0) in vec2 inPosition;\n") +
				std::string("layout (location = 1) in vec2 inTexCoord;\n") +
				std::string("uniform mat4 P;") +
				std::string("out vec2 texCoord;") +
				std::string("void main()\n") +
				std::string("{\n") +
				std::string("   texCoord = inTexCoord;\n") +
				std::string("   gl_Position = P*vec4(vec3(inPosition,0.0),1.0);\n") +
				std::string("}");
		}

		static inline std::string modelProjectionVSSource() {
			return
				std::string("#version 330 core\n") +
				std::string("layout (location = 0) in vec2 inPosition;\n") +
				std::string("layout (location = 1) in vec2 inTexCoord;\n") +
				std::string("uniform mat4 P;") +
				std::string("uniform mat4 M;") +
				std::string("out vec2 texCoord;") +
				std::string("void main()\n") +
				std::string("{\n") +
				std::string("   texCoord = inTexCoord;\n") +
				std::string("   gl_Position = P*M*vec4(vec3(inPosition,0.0),1.0);\n") +
				std::string("}");
		}

		static inline std::string shadeVSSource(){
			return std::string(
				std::string("#version 330 core\n") +
				std::string("layout (location = 0) in vec2 inPosition;\n") +
				std::string("layout (location = 1) in vec2 inTexCoord;\n") +
				std::string("out float x;\n") +
				std::string("out float y;\n") +
				std::string("out float z;\n") +
				std::string("out float w;\n") +
				std::string("uniform mat4 M;\n") +
				std::string("uniform vec2 leftBottom;\n") +
				std::string("uniform vec2 rightTop;\n") +
				std::string("void main()\n") +
				std::string("{\n") +
				std::string("   vec4 p = M*vec4(vec3(inPosition,0.0),1.0);\n") +
				std::string("   x = mix(leftBottom.x, rightTop.x, inTexCoord.x);\n") +
				std::string("   y = mix(leftBottom.y, rightTop.y, inTexCoord.y);\n") +
				std::string("   z = p.z;\n") +
				std::string("   w = p.w;\n") +
				std::string("   gl_Position = p;\n") +
				std::string("}"));
		};

		static inline std::string shadeFSSource(const std::string& inputUniforms, const std::string& globals, const std::string& fragmentShaderMain) {
			return std::string(
				std::string("#version 330 core\n") +
				std::string("out vec4 FragColor;\n") +
				std::string("in float x;\n") +
				std::string("in float y;\n") +
				std::string("in float z;\n") +
				std::string("in float w;\n") +
				std::string("uniform vec2 max;\n") +
				std::string("uniform vec2 min;\n") +
				std::string("uniform vec2 size;\n") +
				inputUniforms + "\n" +
				globals + "\n" +
				std::string("\nvoid main(){\n") +
				std::string("vec4 color;\n") +
				fragmentShaderMain + "\n" +
				std::string("gl_FragData[0] = color;\n") +
				std::string("\n}\n"));
		};

		static inline std::string textureFSSource(const std::string& inputUniforms, const std::string& globals, const std::string& shader){
			return
				std::string("#version 330 core\n") +
				std::string("in vec2 texCoord;\n") +
				std::string("out vec4 FragColor;\n")
				+ inputUniforms + "\n" +
				std::string("uniform sampler2D texture0;\n")
				+ globals + "\n" +
				std::string("void main(){\n") +
				std::string("vec4 color = texture2D(texture0, texCoord);\n") +
				shader +
				std::string("gl_FragData[0] = color;\n}\n");
		}
	}

	/*namespace mesh {
		///
		/// \brief The Mesh class
		///
		struct Mesh {
			~Mesh() {
				release();
			}
			unsigned int  vao;
			unsigned int  vbos[2];

			void setVBOData(int index, const std::vector<float>& data, size_t numComponents);
			void setVBOData(int index, const std::vector<glm::vec2>& data);
			void release();
		};

		std::shared_ptr<Mesh> create(const std::vector<glm::vec2>& positions,const std::vector<glm::vec2>& textureCoords);

		std::shared_ptr<Mesh> create(const std::vector<float>& positions, size_t numPositionComponents, const std::vector<float>& textureCoords, size_t numTexCoordComponents);

		void render(const Mesh& mesh, int mode, unsigned count);
	}

	namespace quad {
		std::shared_ptr<mesh::Mesh> create();

		void render(const mesh::Mesh& mesh);

		void setPositions(mesh::Mesh& mesh, const std::vector<glm::vec2>& positions);

	}*/

}
