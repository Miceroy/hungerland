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
#include <array>
#include <glad/gl.h>
#include <GLFW/glfw3.h>		// Include glfw
#include <imgui.h>		// Include glfw
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <chrono>			// for Timer

// If you want to take screenshots, you must speciy following:
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

// If you want to load textures, you must speciy following:
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace hungerland {
namespace window {
	Image::Image(const std::string& fileName) {
		data = stbi_load(fileName.c_str(), &size.x, &size.y, &bpp, 0);
	}

	Image::~Image() {
		stbi_image_free(data);
	}



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

	Window::Window(size2d_t size, const std::string& title, bool resizable)
		: m_size(size)
		, m_window(0)
	{
		if(!g_engine) g_engine = std::make_unique<engine::Engine>();
		// Create window and check that creation was succesful.

		glfwWindowHint(GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);
		m_window = glfwCreateWindow(int(m_size.x), int(m_size.y), title.c_str(), 0, 0);
		if (!m_window) {
			throw std::runtime_error("Failed to create window!");
			return;
		}

		// Set current context
		glfwMakeContextCurrent(m_window);
		glfwSwapInterval(1);

		// Load GL functions using glad
		gladLoadGL(glfwGetProcAddress);

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsLight();

		// Setup Platform/Renderer backend
		ImGui_ImplGlfw_InitForOpenGL(m_window, true);
		ImGui_ImplOpenGL3_Init("#version 150");


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
		printf("Viewport: %d, %d\n", screenWidth, screenHeight);
		m_screen = std::make_unique<screen::FrameBuffer>();
		m_screen->setScreen(screen::Rect{0.0f, float(screenWidth), 0.0f, float(screenHeight)});

	}

	Window::~Window() {
		// Cleanup
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		// Destroy window
		glfwDestroyWindow(m_window);
		m_window = 0;
	}

	void Window::screenshot(const std::string filename) {
		m_screenshotFileName = filename;
	}

	bool Window::shouldClose() {
		glfwMakeContextCurrent(m_window);
		m_inputMap.nextFrame();
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
		glfwSetWindowSize(m_window, int(m_size.x), int(m_size.y));
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
		if(image.data == 0) return 0;
		return m_textures[filename] = std::make_shared<texture::Texture>(image.size.x, image.size.y, image.bpp, image.data);
	}

	void Window::playSound(const std::string& fileName){
		g_engine->playSound(fileName);
	}


	void Window::render(RenderFunc renderFunc) {
		glfwMakeContextCurrent(m_window);
		int screenWidth, screenHeight;
		glfwGetFramebufferSize(m_window, &screenWidth, &screenHeight);
		glViewport(0, 0, screenWidth, screenHeight);

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// User render
		renderFunc(*this->m_screen);
		
		// Render ImGui
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(m_window);
	}

	int Window::run(UpdateFunc updateGame, RenderFunc renderFunc) {
		Timer frameTimer;
		std::array<float,10> deltaTimes;
		size_t frames = 0;
		auto getDt1 = [&]() {
			auto dt = frameTimer.getDeltaTime();
			deltaTimes[frames % deltaTimes.size()] = dt;
			if(frames < deltaTimes.size()) {
				return deltaTimes[frames];
			}
			float dts=0;
			for(auto dt : deltaTimes) {
				if (dt < 0.10f) {
					dts += dt;
				} else {
					dts += 0.10f;
				}
			}
			auto delta = dts / deltaTimes.size();

			return delta;
		};

		auto getDt = [&]() {
			float dt = frameTimer.getDeltaTime();
			if (dt > 0.2f) {
				dt = 0.2f;
			}
			return dt;
		};

		bool running = true;
		while(shouldClose() == false && running) {
			// Render
			render(renderFunc);

			// Update
			running = updateGame(*this, getDt1());

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
			// Poll other window events.
			++frames;
		}
		return shouldClose() ? 0 : -1;
	};
}
}
