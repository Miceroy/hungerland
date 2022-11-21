//// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= ////
//// CyberSpace - Design, art: Roope Romppainen, Code: Mikko Romppainen
////
////
//// Copyright (c) 2022 Mikko Romppainen.
////
//// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= ////
#include "../platformer.h"

namespace cs {
	/// CONFIG:
	static const std::string GAME_NAME = "Cyber Space";
	static const std::string GAME_VERSION = "v0.0.1";
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
	auto state = reset<Functor,World>(f, GAME_LONG_NAME, "world1");
	return 	window.run([&](mikroplot::Window& window, float dt, float time) {
		app::render(window, update(window, state, f, dt), time);
	});
}
