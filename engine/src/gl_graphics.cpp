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
			util::ERR("OpenGL Error ("+std::to_string(err) + "): " + getErrorString(err));
		}
	}

	namespace shader_std {
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

		static std::string mapVSSource(){
			return
				std::string("#version 330 core\n") +
				std::string("layout (location = 0) in vec2 inPosition;\n") +
				std::string("layout (location = 1) in vec4 inTexCoord;\n") +
				std::string("uniform mat4 P;\n") +
				std::string("uniform vec2 offset = vec2(0, 0);\n") +
				std::string("uniform vec2 parallax = vec2(1, 1);\n") +
				std::string("out vec2 texCoord;") +
				std::string("out vec2 worldPos;") +
				std::string("void main() {\n") +
				std::string("    vec4 pos = vec4(inPosition.x+parallax.x+offset.x,inPosition.y+parallax.y+offset.y, 0.0, 1.0);\n") +
				std::string("    vec4 p = P*pos;\n") +
				std::string("    texCoord  = inTexCoord.xy;\n") +
				std::string("    worldPos.x = inTexCoord.z;\n") +
				std::string("    worldPos.y = inTexCoord.w;\n") +
				std::string("    gl_Position = p;\n") +
				std::string("}\n");
		}

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
				std::string("FragColor = color;\n") +
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
				std::string("vec4 color = texture(texture0, texCoord);\n") +
				shader +
				std::string("FragColor = color;\n}\n");
		}

		static std::string mapImageLayerFSSource() {
			return
				std::string("#version 330 core\n") +
				"in vec2 texCoord;\n"
				"uniform sampler2D image;\n"
				"uniform float opacity = 1;\n"
				"uniform vec2 repeat = vec2(0);\n"
				"out vec4 FragColor;\n"
				"void main() {\n"
				"	vec4 color = vec4(0, 0, 0, 0);\n"
				"   float doPixel = 1;\n"
				"	if(repeat.x == 0 && (texCoord.x < 0.0 || texCoord.x > 1.0)) { \n"
				"	     doPixel = 0;\n"
				"	}"
				"	if(repeat.y == 0 && (texCoord.y < 0.0 || texCoord.y > 1.0)) { \n"
				"	     doPixel = 0;\n"
				"	}"
				"   if(doPixel > 0.5) { \n"
				"        color = texture(image, texCoord);\n"
				"	}\n"
				"   color.a = min(color.a, opacity);\n"
				"	FragColor = color;\n"
				"}\n";
		}

		static std::string tileFSSource() {
			return std::string("") +
				"vec2 getTilePosition(float tileIndex, vec2 tilesetSize) {\n"
				// Fixes rounding imprecision on AMD cards
				"	const float epsilon = 0.000005;\n"
				"	\n"
				"	float index = tileIndex - 1.0;\n"
				"	return vec2(mod(index + epsilon, tilesetSize.x), floor((index / tilesetSize.x) + epsilon)) / tilesetSize;\n\n"
				"}\n"
				"#define FLIP_HORIZONTAL 8u\n"
				"#define FLIP_VERTICAL 4u\n"
				"#define FLIP_DIAGONAL 2u\n"
				"vec2 getTileOffset(float flip, vec2 texCoord, vec2 textureSize, vec2 tileSize, vec2 tilesetSize) {\n"
				"	vec2 texelSize = vec2(1.0) / textureSize;\n"
				"	vec2 offset = mod(texCoord, texelSize);\n"
				"	vec2 ratio = offset / texelSize;\n"
				"	offset = ratio * (1.0 / tileSize);\n"
				"	offset *= tileSize / tilesetSize;\n"
				"	if(flip == FLIP_DIAGONAL) {\n"
				"		float temp = offset.x;\n"
				"		offset.x = offset.y;\n"
				"		offset.y = temp;\n"
				"		temp = tileSize.x / tileSize.y;\n"
				"		offset.x *= temp;\n"
				"		offset.y /= temp;\n"
				"		offset.x = tileSize.x - offset.x;\n"
				"		offset.y = tileSize.y - offset.y;\n"
				"	} else if(flip == FLIP_VERTICAL) {\n"
				"		offset.y = tileSize.y - offset.y;\n"
				"	} else if(flip == FLIP_HORIZONTAL) {\n"
				"		offset.x = tileSize.x - offset.x;\n"
				"	}\n"
				"	return offset;\n"
				"}\n";
		}

		static std::string mapTileMapFSSource() {
			return
				std::string("#version 330 core\n") +
				tileFSSource() +
				"in vec2 texCoord;\n"
				"uniform vec2 tileSize;\n"
				"uniform vec2 tilesetSize ;\n"
				"uniform float opacity;\n"
				"uniform sampler2D lookupMap;\n"
				"uniform sampler2D tileMap;\n"
				"out vec4 FragColor;\n"
				"void main() {\n"
				"	vec4 values = texture(lookupMap, texCoord);\n"
				"	if(values.r > 0.0) {\n"
				"		vec2 position = getTilePosition(values.r, tilesetSize);\n"
				"		vec2 offset = getTileOffset(values.g, texCoord, textureSize(lookupMap, 0), tileSize, tilesetSize);\n"
				"       vec4 color = texture(tileMap, position + offset);\n"
				"		color.a = min(opacity, color.a);\n"
				"		FragColor = color;\n"
				"	} else {\n"
				"		FragColor = vec4(0,0,0,0);\n"
				"	}\n"
				"}";
		}
	} // End - namespace shader_source

	namespace shaders {
		shader::Shader::Ref createPasstrough() {
			return std::make_unique<shader::Shader>(shader_std::projectionVSSource(), shader_std::textureFSSource("","",""));
		}

		shader::Shader::Ref createTileLayer() {
			return std::make_shared<shader::Shader>(shader_std::mapVSSource(), shader_std::mapTileMapFSSource());
		}

		shader::Shader::Ref createImageLayer() {
			return std::make_shared<shader::Shader>(shader_std::mapVSSource(), shader_std::mapImageLayerFSSource());
		}

		shader::Shader::Ref createShade(const std::vector<shader::Constant>& constants, const std::string& fragmentShaderMain, const std::string& globals) {
			return std::make_shared<shader::Shader>(shader_std::shadeVSSource(), shader_std::shadeFSSource(shader::to_string(constants), globals, fragmentShaderMain));
		}

		shader::Shader::Ref createSprite(const std::vector<shader::Constant>& constants, const std::string& surfaceShader, const std::string& globals) {
			return std::make_shared<shader::Shader>(shader_std::modelProjectionVSSource(), shader_std::textureFSSource("", globals, surfaceShader));
		}
	} // End - namespace shaders
}
