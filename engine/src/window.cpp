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

#include <glad/gl.h>		// Include glad
#include <GLFW/glfw3.h>		// Include glfw
#include <chrono>			// for Timer

// If you want to take screenshots, you must speciy following:
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

// If you want to load textures, you must speciy following:
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace hungerland {
namespace window {
	struct Image {
		Image(const std::string& fileName) {
			data = stbi_load(fileName.c_str(), &size.x, &size.y, &bpp, 0);
		}

		~Image() {
			stbi_image_free(data);
		}

		int2d_t size = {0, 0};
		int bpp = 0;
		uint8_t *data = 0;
	};

	class Timer {
	public:
		Timer()
			: start(std::chrono::high_resolution_clock::now()) {
		}

		float getDeltaTime() {
			auto end = std::chrono::high_resolution_clock::now();
			float deltaTime = std::chrono::duration_cast<std::chrono::duration<float>>(end - start).count();
			start = end;
			totalTime += deltaTime;
			return deltaTime;
		}

	private:
		Timer(const Timer&) = delete;
		std::chrono::time_point<std::chrono::high_resolution_clock> start;
		float totalTime;
	};

	bool UserInput::keyState(const std::map<int, bool>& keyMap, int keyCode) {
		auto it = keyMap.find(keyCode);
		if(it == keyMap.end()){
			return false;
		}
		return it->second;
	}

	void UserInput::setKey(int keyCode, bool state) {
		m_curKeys[keyCode] = state;
	}

	void UserInput::nextFrame() {
		m_prevKeys = m_curKeys;
	}

	int UserInput::getKeyState(int keyCode) const {
		if(keyState(m_curKeys,keyCode) ){
			return true;
		}
		return false;
	}

	int UserInput::getKeyPressed(int keyCode) const {
		return keyState(m_curKeys,keyCode) && !keyState(m_prevKeys,keyCode);
	}

	int UserInput::getKeyReleased(int keyCode) const {
		return !keyState(m_curKeys,keyCode) && keyState(m_prevKeys,keyCode);
	};


	std::unique_ptr<engine::Engine> g_engine;

	Window::Window(size2d_t size, const std::string& title)
		: m_size(size)
		, m_window(0)
	{
		if(!g_engine) g_engine = std::make_unique<engine::Engine>();
		// Create window and check that creation was succesful.
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		m_window = glfwCreateWindow(m_size.x, m_size.y, title.c_str(), 0, 0);
		if (!m_window) {
			throw std::runtime_error("Failed to create window!");
			return;
		}

		// Set current context
		glfwMakeContextCurrent(m_window);
		// Load GL functions using glad
		gladLoadGL(glfwGetProcAddress);

		glfwSetWindowUserPointer(m_window, this);
		// Specify the key callback as c++-lambda to glfw
		glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
			// Close window if escape is pressed by the user.
			if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
				glfwSetWindowShouldClose(window, GLFW_TRUE);
			}
			Window* pThis = (Window*)glfwGetWindowUserPointer(window);
			if(action == GLFW_PRESS){
				pThis->m_inputMap.setKey(key, true);
			}
			if(action == GLFW_RELEASE){
				pThis->m_inputMap.setKey(key, false);
			}
		});

		// Query the size of the framebuffer (window content) from glfw.
		int screenWidth, screenHeight;
		glfwGetFramebufferSize(m_window, &screenWidth, &screenHeight);
		glViewport(0, 0, screenWidth, screenHeight);
		m_screen = std::make_unique<screen::FrameBuffer>();
		m_screen->setScreen(0, screenWidth, 0, screenHeight);

	}

	Window::~Window() {
		// Destroy window
		glfwDestroyWindow(m_window);
		m_window = 0;
	}

	void Window::screenshot(const std::string filename) {
		m_screenshotFileName = filename;
	}

	bool Window::shouldClose() {
		glfwMakeContextCurrent(m_window);
		glfwPollEvents();
		return m_window == 0 || glfwWindowShouldClose(m_window);
	}

	void Window::setTitle(const std::string& title) {
		glfwMakeContextCurrent(m_window);
		glfwSetWindowTitle(m_window, title.c_str());
	}

	void Window::setWindowSize(size2d_t size){
		m_size = size;
		glfwMakeContextCurrent(m_window);
		glfwSetWindowSize(m_window, m_size.x, m_size.y);
	}

	void Window::setWindowPosition(int2d_t pos){
		glfwMakeContextCurrent(m_window);
		glfwSetWindowPos(m_window, pos.x, pos.y);
	}

	int2d_t Window::getWindowPosition() const {
		int2d_t res;
		glfwMakeContextCurrent(m_window);
		glfwGetWindowPos(m_window, &res.x, &res.y);
		return res;
	}

	std::shared_ptr<texture::Texture> Window::loadTexture(const std::string& filename) {
		glfwMakeContextCurrent(m_window);
		auto it = m_textures.find(filename);
		if(it != m_textures.end()) {
			return it->second;
		}
		Image image(filename);
		return m_textures[filename] = std::make_shared<texture::Texture>(image.size.x, image.size.y, image.bpp, image.data);
	}

	void Window::playSound(const std::string& fileName){
		g_engine->playSound(fileName);
	}

	int Window::run(UpdateFunc updateGame, RenderFunc render) {
		Timer frameTimer;
		std::array<float,10> deltaTimes;
		size_t frames = 0;
		auto getDt = [&]() {
			auto dt = frameTimer.getDeltaTime();
			deltaTimes[frames % deltaTimes.size()] = dt;
			if(frames < deltaTimes.size()) {
				return deltaTimes[frames];
			}
			float dts=0;
			for(auto dt : deltaTimes){
				dts += dt;
			}
			return dts / deltaTimes.size();
		};

		while(shouldClose() == false) {
			// Render
			glfwMakeContextCurrent(m_window);
			render(*this->m_screen);
			glfwSwapBuffers(m_window);

			// Update
			updateGame(*this, getDt());

			// Save screenshot
			if(m_screenshotFileName.length()>0){
				int width, height;
				int channels = 4;
				glfwGetFramebufferSize(m_window, &width, &height);
				std::vector<uint8_t> lastFrame;
				lastFrame.resize(channels*width*height);
				glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &lastFrame[0]);
				for(size_t i=0; i<lastFrame.size(); ++i){
					if((i%4) == 3){
						lastFrame[i] = 0xff;
					}
				}
				stbi_flip_vertically_on_write(true);
				stbi_write_png(m_screenshotFileName.c_str(), width, height, channels, &lastFrame[0], width*channels);
				stbi_flip_vertically_on_write(false);
				m_screenshotFileName = "";
			}
			m_inputMap.nextFrame();
			// Poll other window events.
			glfwPollEvents();
			++frames;
		}
		return 0;
	};
}
}
