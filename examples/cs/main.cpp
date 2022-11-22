//// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= ////
//// CyberSpace - Design, art: Roope Romppainen, Code: Mikko Romppainen
////
////
//// Copyright (c) 2022 Mikko Romppainen.
////
//// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= ////
#include "../platformer.h"

namespace cs {
	// Resource files:
	static const std::vector<std::string> MAP_FILES = {
		"assets/world1.tmx",
	};

	static const std::vector<std::string> BACKGROUND_FILES = {
		"assets/images/BG World 1.png",
	};

	static const std::vector<std::string> CHARACTER_TEXTURE_FILES = {
		"assets/images/Player.png",
		"assets/images/Enemy World 1.png",
	};

	static const std::vector<std::string> ITEM_TEXTURE_FILES = {
	};

	// Application config:
	static const std::string GAME_NAME = "Cyber Space";
	static const std::string GAME_VERSION = "v0.0.2";
	static const std::string GAME_LONG_NAME = GAME_NAME + " " + GAME_VERSION;
}

// Main function
int main() {
	using namespace mikroplot;
	using namespace platformer;
	using namespace app;
	using namespace cs;
	// Create application window and run it.
	mikroplot::Window window(WINDOW_SIZE_X/2, WINDOW_SIZE_Y/2, "");
	Functor f;
	typedef World<GameObject> World;
	f.loadTexture = [&window](const std::string& fileName) {
		return window.loadTexture(fileName);
	};
	static const platformer::Config CONFIG = {
		BACKGROUND_FILES,
		MAP_FILES,
		CHARACTER_TEXTURE_FILES,
		ITEM_TEXTURE_FILES
	};
	auto state = reset<Functor,World>(f, GAME_LONG_NAME, CONFIG);
	return 	window.run([&](mikroplot::Window& window, float dt, float time) {
		app::render(window, update(window, state, f, dt), time);
	});
}
