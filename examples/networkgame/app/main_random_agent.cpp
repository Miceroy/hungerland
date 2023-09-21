#include "client_app.h"

int main(int argc, const char* argv[]) {
	// Create Client application data:
	auto client = client_app::createClient(argc, argv);
	// Load assets:
	const std::vector<std::shared_ptr<sf::Texture> > textures = sfml_application::loadTextures({
		"assets/ground.png",
		"assets/goal.png",
		"assets/wall.png",
		"assets/player.png",
		"assets/enemy.png",
		"assets/item.png",
	});
	auto readInput =  sfml_application::randomInput;
	// Run application:
	return client_app::run(client, readInput, textures);
}
