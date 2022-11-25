#include "../platformer.h"

namespace my_game_app {
	// Resource files:
	static const std::vector<std::string> MAP_FILES = {
		"assets/test_world1.tmx",
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

// Main function
int main() {
	using namespace my_game_app;

	// Create application window and run it.
	Window window(WINDOW_SIZE_X, WINDOW_SIZE_Y, "");
	app::Functor f;
	typedef app::World<app::GameObject> World;
	f.loadTexture = [&window](const std::string& fileName, bool repeat) {
		return window.loadTexture(fileName, repeat);
	};

	auto state = app::reset<World>(f, GAME_LONG_NAME, CONFIG);
	return window.run([&](Window& window, float dt, float time) {
		platformer::player::Input playerInput;
		// Configure input buttons:
		playerInput.dx			= window.getKeyState(KEY_RIGHT)				- window.getKeyState(KEY_LEFT);
		playerInput.accelerate	= window.getKeyState(KEY_LEFT_SHIFT)		+ window.getKeyState(KEY_RIGHT_SHIFT);
		playerInput.wantJump	= window.getKeyPressed(KEY_LEFT_CONTROL)	+ window.getKeyPressed(KEY_RIGHT_CONTROL);
		app::render(window, platformer::update(state, f, playerInput, dt), time);
	});
}
