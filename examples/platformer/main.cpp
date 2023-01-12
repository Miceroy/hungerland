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

	static const view::Config CONFIG = {
		BACKGROUND_FILES,
		MAP_FILES,
		CHARACTER_TEXTURE_FILES,
		ITEM_TEXTURE_FILES
	};
}
#include <hungerland/window.h>

// Main function
int main() {
	using namespace my_game_app;
	using namespace platformer;
	using namespace hungerland;

	typedef model::World<model::Character> Model;
	typedef window::Window View;

	// Create application window and run it.
	View window({WINDOW_SIZE_X, WINDOW_SIZE_Y}, "");
	auto state = env::reset<Model>(&window, GAME_LONG_NAME, CONFIG);
	float totalTime = 0;
	int lastFrame = -1;
	return window.run([&](View& window, float dt) {
		totalTime += dt;
		auto& input = window.getInput();
		if(int(totalTime) > lastFrame){
			window.setTitle(GAME_LONG_NAME + "    FPS="+std::to_string(1.0f/dt).substr(0,5));
			lastFrame = int(totalTime);
		}
		if(input.getKeyPressed(window::KEY_F5)) {
			state = env::reset<Model>(&window, GAME_LONG_NAME, CONFIG);
		}
		// Configure input buttons:
		agent::Action playerAction;
		playerAction.dy			= input.getKeyState(window::KEY_UP)				- input.getKeyState(window::KEY_DOWN);
		playerAction.dx			= input.getKeyState(window::KEY_RIGHT)			- input.getKeyState(window::KEY_LEFT);
		playerAction.accelerate	= input.getKeyState(window::KEY_LEFT_SHIFT)		+ input.getKeyState(window::KEY_RIGHT_SHIFT);
		playerAction.wantJump	= input.getKeyPressed(window::KEY_LEFT_CONTROL)	+ input.getKeyPressed(window::KEY_RIGHT_CONTROL);
		state = env::update<Model>(&window, state, playerAction, dt);
		return true;
	}, [&state,&window](screen::Screen& screen) {
		view::render(screen, state);
	});
}
