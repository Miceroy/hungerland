#include "standalone_app.h"

int main(int argc, const char* argv[]) {
	// Create Standalone application data:
	auto app = standalone_app::createGameApp(argc, argv);
	// Load assets:
	const std::vector<std::shared_ptr<sf::Texture> > textures = sfml_application::loadTextures({
		"assets/ground.png",
		"assets/goal.png",
		"assets/wall.png",
		"assets/player.png",
		"assets/enemy.png",
		"assets/item.png",
	});
	auto readInput =  sfml_application::arrowKeysInput;
	// Run application:
	return standalone_app::run(app, readInput, textures);
}
