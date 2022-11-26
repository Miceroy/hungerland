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
#include <hungerland/math.h>
#include <hungerland/shader.h>
#include <memory>
#include <string>
#include <vector>
#include <memory>

namespace hungerland {
namespace shader_source {
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

	static std::string map_spriteVSSource(){
		return
			std::string("#version 330 core\n") +
			"layout (location = 0) in vec2 inPosition;\n"
			"layout (location = 1) in vec4 inTexCoord;\n"
			"uniform mat4 P;\n"
			"uniform vec2 offset = vec2(0, 0);\n"
			"uniform vec2 parallax = vec2(1, 1);\n"
			"out vec2 texCoord;"
			"out vec2 worldPos;"
			"void main() {\n"
			"    vec4 pos = vec4(inPosition.x+parallax.x+offset.x,inPosition.y+parallax.y+offset.y, 0.0, 1.0);\n"
			"    gl_Position = P*pos;\n"
			"    texCoord  = inTexCoord.xy;\n"
			"    worldPos.x = inTexCoord.z;\n"
			"    worldPos.y = inTexCoord.w;\n"
			"}\n";
	}

	static std::string map_bgShader() {
		return
			std::string("#version 330 core\n") +
			"in vec2 texCoord;\n"
			"uniform sampler2D image;\n"
			"uniform float opacity = 1;\n"
			"uniform vec2 repeat = vec2(0);\n"
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
			"	gl_FragData[0] = color;\n"
			"}\n";
	}
/*

	static std::string imageShader() {
		return
			std::string("#version 330 core\n") +
			"in vec2 texCoord;\n"
			"uniform sampler2D image;\n"
			"uniform float u_opacity;\n"
			"void main() {\n"
			"	vec4 color = vec4(0, 0, 0, 0);\n"
			"	color = texture(image, texCoord);\n"
			"	color.a = min(color.a, u_opacity);\n"
			"	gl_FragData[0] = color;\n"
			"}\n";
	}
*/
	static std::string map_tilemapShader() {
		return
			std::string("#version 330 core\n") +
			"#define FLIP_HORIZONTAL 8u\n"
			"#define FLIP_VERTICAL 4u\n"
			"#define FLIP_DIAGONAL 2u\n"
			"in vec2 texCoord;\n"
			"uniform vec2 tileSize;\n"
			"uniform vec2 tilesetSize ;\n"
			"uniform float opacity;\n"
			"uniform sampler2D lookupMap;\n"
			"uniform sampler2D tileMap;\n"
			// Fixes rounding imprecision on AMD cards
			"const float epsilon = 0.000005;\n"
			"void main() {\n"
			"	vec4 values = texture(lookupMap, texCoord);\n"
			"	float flip = values.g;\n"
			"	if(values.r > 0.0) {\n"
			"		float index = float(values.r) - 1.0;\n"
			"		vec2 position = vec2(mod(index + epsilon, tilesetSize.x), floor((index / tilesetSize.x) + epsilon)) / tilesetSize;\n"

			"		vec2 texelSize = vec2(1.0) / textureSize(lookupMap, 0);\n"
			"		vec2 offset = mod(texCoord, texelSize);\n"
			"		vec2 ratio = offset / texelSize;\n"
			"		offset = ratio * (1.0 / tileSize);\n"
			"		offset *= tileSize / tilesetSize;\n"
	#if 1
			"		vec2 tileSize = vec2(1.0) / tilesetSize;\n"
			"		if(flip == FLIP_DIAGONAL) {\n"
			"			float temp = offset.x;\n"
			"			offset.x = offset.y;\n"
			"			offset.y = temp;\n"
			"			temp = tileSize.x / tileSize.y;\n"
			"			offset.x *= temp;\n"
			"			offset.y /= temp;\n"
			"			offset.x = tileSize.x - offset.x;\n"
			"			offset.y = tileSize.y - offset.y;\n"
			"		} else if(flip == FLIP_VERTICAL) {\n"
			"			offset.y = tileSize.y - offset.y;\n"
			"		} else if(flip == FLIP_HORIZONTAL) {\n"
			"			offset.x = tileSize.x - offset.x;\n"
			"		}\n"
	#endif
			"       vec4 color = texture(tileMap, position + offset);\n"
			"		color.a = min(opacity, color.a);\n"
			"		gl_FragData[0] = color;\n"
			"	} else {\n"
			"		gl_FragData[0] = vec4(0,0,0,0);\n"
			"	}\n"
			"}";
	}
} // End - namespace shader_source

namespace shaders {
	static inline auto passtrough() {
		return std::make_unique<shader::Shader>(shader_source::projectionVSSource(), shader_source::textureFSSource("","",""));
	}

	static inline auto tileLayer() {
		return std::make_shared<shader::Shader>(shader_source::map_spriteVSSource(), shader_source::map_tilemapShader());
	}

	static inline auto imageLayer() {
		return std::make_shared<shader::Shader>(shader_source::map_spriteVSSource(), shader_source::map_bgShader());
	}
} // End - namespace shaders

}
