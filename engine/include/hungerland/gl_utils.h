#pragma once
#include <glad/gl.h>
#include <assert.h>
#include <stdio.h>
#include <string>
#include <fstream>
#include <streambuf>
#include <stdexcept>

namespace hungerland {

static void checkGLError() {
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
		throw std::runtime_error("OpenGL Error ("+std::to_string(err) + "): " + getErrorString(err));
		assert(0);
	}
}

static std::string readFile(const std::string& fileName){
	std::ifstream f(fileName);
	return std::string(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
}

}
