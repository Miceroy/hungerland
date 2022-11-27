#include "../platformer.h"

namespace my_game_app {
	// Resource files:
	static const std::vector<std::string> MAP_FILES = {
		"assets/test_world2.tmx",
	};

	static const std::vector<std::string> BACKGROUND_FILES = {
	};

	static const std::vector<std::string> CHARACTER_TEXTURE_FILES = {
		"assets/Player/Player_NoAnimation.png",
	};

	static const std::vector<std::string> ITEM_TEXTURE_FILES = {
	};

	// Application config:
	static const size_t	WINDOW_SIZE_X = platformer::SCREEN_SIZE_X/2;
	static const size_t	WINDOW_SIZE_Y = platformer::SCREEN_SIZE_Y/2;
	static const std::string GAME_NAME = "Platformer Example";
	static const std::string GAME_VERSION = "v0.0.1";
	static const std::string GAME_LONG_NAME = GAME_NAME + " " + GAME_VERSION;

	static const app::Config CONFIG = {
		BACKGROUND_FILES,
		MAP_FILES,
		CHARACTER_TEXTURE_FILES,
		ITEM_TEXTURE_FILES
	};
}
#include <hungerland/texture.h>

// Main function
int main() {
	using namespace my_game_app;
	using namespace hungerland;

	// Create application window and run it.
	window::Window window({WINDOW_SIZE_X, WINDOW_SIZE_Y}, "");
	app::Functor f;
	typedef app::World<app::GameObject> World;
	f.loadTexture = [&window](const std::string& fileName, bool repeat) {
		auto texture = window.loadTexture(fileName);
		if(repeat) {
			texture->setRepeat(true);
		}
		return texture;
	};
	window.setTitle(GAME_LONG_NAME);

	auto state = app::reset<World>(f, GAME_LONG_NAME, CONFIG);
	float totalTime = 0;
	int lastFrame = -1;
	return window.run([&](window::Window& window, float dt) {
		totalTime += dt;
		auto& input = window.getInput();
		if(int(totalTime) > lastFrame){
			window.setTitle(GAME_LONG_NAME + "    FPS="+std::to_string(1.0f/dt).substr(0,5));
			lastFrame = int(totalTime);
		}
		// Configure input buttons:
		platformer::player::Input playerInput;
		playerInput.dx			= input.getKeyState(window::KEY_RIGHT)			- input.getKeyState(window::KEY_LEFT);
		playerInput.accelerate	= input.getKeyState(window::KEY_LEFT_SHIFT)		+ input.getKeyState(window::KEY_RIGHT_SHIFT);
		playerInput.wantJump	= input.getKeyPressed(window::KEY_LEFT_CONTROL)	+ input.getKeyPressed(window::KEY_RIGHT_CONTROL);
		state = platformer::update(state, f, playerInput, dt);
		return true;
	}, [&state,&window](screen::Screen& screen) {
		app::render(screen, state);
	});
}
