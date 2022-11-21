#pragma once
#include <string>

namespace hungerland {
	static std::string tilemapVSSource(){
		return
			std::string("#version 330 core\n") +
			"layout (location = 0) in vec2 inPosition;\n"
			"layout (location = 1) in vec2 inTexCoord;\n"
			"uniform mat4 P;\n"
			"out vec2 texCoord;"
			"void main() {\n"
			"    gl_Position = P*vec4(inPosition.x, inPosition.y, 0, 1.0);\n"
			"    texCoord  = inTexCoord;\n"
			"}\n";
	}


	static std::string tilemapFSSource(){
		return
			std::string("#version 330 core\n") +
			"#define FLIP_HORIZONTAL 8u\n"
			"#define FLIP_VERTICAL 4u\n"
			"#define FLIP_DIAGONAL 2u\n"
			"in vec2 texCoord;\n"
			"uniform sampler2D tileMap;\n"
			"uniform sampler2D lookupMap;\n"
			"uniform vec2 u_tileSize;\n"
			"uniform vec2 u_tilesetCount ;\n"
			"uniform float u_opacity;\n"
			// Fixes rounding imprecision on AMD cards
			"const float epsilon = 0.000005;\n"
			"void main() {\n"
			"	vec4 values = texture(lookupMap, texCoord);\n"
			"	float flip = values.g;\n"
			"	if(values.r > 0.0) {\n"
			"		float index = float(values.r) - 1.0;\n"
			"		vec2 position = vec2(mod(index + epsilon, u_tilesetCount.x), floor((index / u_tilesetCount.x) + epsilon)) / u_tilesetCount;\n"

			"		vec2 texelSize = vec2(1.0) / textureSize(lookupMap, 0);\n"
			"		vec2 offset = mod(texCoord, texelSize);\n"
			"		vec2 ratio = offset / texelSize;\n"
			"		offset = ratio * (1.0 / u_tileSize);\n"
			"		offset *= u_tileSize / u_tilesetCount;\n"
#if 1
			"		vec2 tileSize = vec2(1.0) / u_tilesetCount;\n"
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
			"		gl_FragData[0] = texture(tileMap, position + offset);\n"
			"		gl_FragData[0].a = min(gl_FragData[0].a, u_opacity);\n"
			"	} else {\n"
			"		gl_FragData[0] = vec4(0,0,0,0);\n"
			"	}\n"
			"}";
	}

}
