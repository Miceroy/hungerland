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
#include <hungerland/window.h>
#include <hungerland/framebuffer.h>
#include <hungerland/texture.h>
#include <hungerland/mesh.h>
#include <hungerland/engine.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>		// Include glfw
#include <hungerland/util.h>
#include <stdexcept>

// Miniaudio
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>



namespace hungerland {
namespace engine {

	Engine::Engine() {
		// Set c++-lambda as error call back function for glfw.
		glfwSetErrorCallback([](int error, const char* description) {
			fprintf(stderr, "Error %dm_spriteVao: %s\n", error, description);
		});
		// Try to initialize glfw
		if (!glfwInit()) {
			throw std::runtime_error("Failed to initialize OpenGL!");
			return;
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
		//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
		//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

		m_audioEngine = new ma_engine;
		auto result = ma_engine_init(NULL, m_audioEngine);
		if (result != MA_SUCCESS) {
			throw std::runtime_error("Failed to initialize audio engine!");
			return;
		}
	}

	Engine::~Engine() {
		ma_engine_uninit(m_audioEngine);
		delete m_audioEngine;
		// Terminate glfw
		glfwTerminate();
	}

	void Engine::playSound(const std::string& fileName) {
		auto result = ma_engine_play_sound(m_audioEngine, fileName.c_str(), NULL);
		if (result != MA_SUCCESS) {
			util::ERR("Failed to play sound from file: \""+fileName+"\"!");
		}
	}
}
}
