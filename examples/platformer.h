//// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= ////
//// CyberSpace - Design, art: Roope Romppainen, Code: Mikko Romppainen
////
////
//// Copyright (c) 2022 Mikko Romppainen.
////
//// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= ////
#include <hungerland/util.h>
#include <hungerland/map.h>
#include <mikroplot/window.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace math = glm;
using namespace std;
using namespace hungerland;

namespace platformer {
	/// CONFIG:
	static const size_t	WINDOW_SIZE_X = 1920;
	static const size_t	WINDOW_SIZE_Y = 1080;
	static const float	SCALE_X = 48.0f;
	static const float	SCALE_Y = 48.0f;

	///
	/// \brief platform::world::loadScene function.
	/// \param f
	/// \param world
	/// \param mapFilename
	/// \param backgroundFilename
	/// \param objectTextureFiles
	///
	template<typename Functor, typename World>
	void loadScene(Functor f, World& world, const std::string& mapFilename, const std::string& backgroundFilename, std::vector<std::string> objectTextureFiles) {

		// Load map
		if(false == world.map.load(mapFilename)) {
			util::ERROR("Failed to load map file: \"" + mapFilename + "\"!");
		}
		util::INFO("Loaded Tiled map: " + mapFilename);

		// Create tileset textures from map tilesets.
		std::vector< std::shared_ptr<mikroplot::Texture> > mapTextures;
		for(const auto& ts : world.map.getTilesets()) {
			auto texture = f.loadTexture(ts.getImagePath());
			if(texture == 0) {
				util::ERROR("Failed to load tileset texture file: \"" + ts.getImagePath() + "\"!");
			}
			util::INFO("Loaded tileset texture: " + ts.getImagePath());
			mapTextures.push_back(texture);
		}

		// Create map layers by map and tileset.
		world.mapLayers = hungerland::MapLayers(world.map, mapTextures);

		// Load background texture
		world.background = f.loadTexture(backgroundFilename);
		if(world.background == 0) {
			util::ERROR("Failed to load background image file: \"" + backgroundFilename + "\"!");
		}
		util::INFO("Loaded background texture: " + mapFilename);

		// Load object textures
		for(const auto& filename : objectTextureFiles) {
			auto texture = f.loadTexture(filename);
			if(texture== 0) {
				util::ERROR("Failed to load object texture file: \"" + filename + "\"!");
			}
			util::INFO("Loaded object texture: " + filename);
			world.objectTextures.push_back(texture);
		}

		// Get map tilecount
		auto tileCount = world.map.getTileCount();
		// and adjust camera and to center y and left of map.
		world.cameraPos = math::vec3(0, tileCount.y/2, 0);
		world.player.position = math::vec3(0, tileCount.y/2, 0);
	}

	///
	/// \brief platform::world::reset
	/// \param f
	/// \param name
	/// \param initialMap
	///
	template<typename Functor, typename World, typename Window>
	auto reset(Window& window, const std::string& name, std::string initialMap) {
		Functor f;
		f.loadTexture = [&window](const std::string& fileName) {
			return window.loadTexture(fileName);
		};
		// File names
		std::vector<std::string> objectTextureFiles = {
			"assets/images/Player.png",
			"assets/images/Enemy World 1.png"
		};
		auto mapFilename = "assets/"+initialMap+".tmx";
		auto backgroundFilename = "assets/images/BG World 1.png";

		auto s = World();
		s.sceneName = name;

		loadScene(f, s, mapFilename, backgroundFilename, objectTextureFiles);

		return s;
	};


	///
	/// \brief platform::world::update
	/// \param window
	/// \param state
	/// \param f
	/// \param dt
	/// \return
	///
	template<typename Window, typename World, typename Functor>
	const auto& update(const Window& window, World& state, Functor f, float dt) {
		// Move camera to right

		//state.cameraPos.x += 1.0f*dt;
		float dx = window.getKeyState(mikroplot::KEY_RIGHT) - window.getKeyState(mikroplot::KEY_LEFT);

		if(window.getKeyState(mikroplot::KEY_LEFT_SHIFT)+window.getKeyState(mikroplot::KEY_RIGHT_SHIFT)) {
			state.player.position.x += 25.0f*dx*dt;
		} else {
			state.player.position.x += 5.0f*dx*dt;
		}

		// Camera follows player
		state.cameraPos = state.player.position;
		return state;
	};

	namespace player {
		///
		/// \brief 	world::player::update
		/// \param world
		/// \param player
		/// \param dt
		///
		template<typename UserInterface, typename World, typename Player>
		auto update(const UserInterface& ui, World world, const Player& player, float dt) {
			return world;
		};
	}

	///
	/// \brief The World class
	///
	template<typename GameObject>
	struct World {
		std::string sceneName;
		std::shared_ptr<mikroplot::Texture> background;
		std::vector<std::shared_ptr<mikroplot::Texture> > objectTextures;
		tmx::Map map;
		MapLayers mapLayers;
		glm::vec3 cameraPos;
		GameObject player;
		std::vector<GameObject> actors;
	};
}


namespace app {
	struct GameObject {
		glm::vec3 position;
	};

	struct Game {
	};


	mikroplot::Grid Sprite(std::string name) {
		return mikroplot::gridN(32, 12);
	}

	auto to_glm(std::vector<float> m){
		glm::mat4 mat;
		for(size_t i=0; i<4; ++i) {
			for(size_t j=0; j<4; ++j) {
				mat[i][j] = m[i*4 + j];
			}
		}
		return mat;
	}
	auto to_mat(glm::mat4 m){
		std::vector< std::vector<float> > mat;
		for(size_t i=0; i<4; ++i) {
			mat.push_back(std::vector<float>());
			for(size_t j=0; j<4; ++j) {
				mat[i].push_back(m[i][j]);
			}
		}
		return mat;
	}
	auto to_vec(glm::mat4 mat){
		std::vector<float> v;
		for(size_t i=0; i<4; ++i) {
			for(size_t j=0; j<4; ++j) {
				v.push_back(mat[i][j]);
			}
		}
		return v;
	}


	///
	/// \brief render
	/// \param window
	/// \param state
	/// \param time
	///
	void render(mikroplot::Window& window, const platformer::World<GameObject>& state, float time) {
		using namespace platformer;
		window.setTitle(state.sceneName);
		window.setClearColor();
		// Aseta origo ruudun vasempaan alareunaan:
		static const auto SIZE_X = WINDOW_SIZE_X/2.0f;
		static const auto SIZE_Y = WINDOW_SIZE_Y/2.0f;
		auto projection = to_glm(window.setScreen(-SIZE_X, SIZE_X , SIZE_Y, -SIZE_Y));

		// Background
		auto bgPos = glm::scale(glm::mat4(1.0f), glm::vec3(1920,1080,0));
		window.drawSprite(to_mat(bgPos), state.background.get());

		// Tilemap
		glm::mat4 mat = glm::mat4(1);
		glm::vec3 camPos = state.cameraPos;
		auto mapPos = camPos;
		mapPos.x *= SCALE_X;
		mapPos.y *= SCALE_Y;
		mat = glm::translate(mat, mapPos);
		state.mapLayers.render(to_vec(projection*glm::inverse(mat)));

		// Player
		mat = glm::mat4(1);
		auto texture = state.objectTextures[0].get();
		auto playerPos = state.player.position - state.cameraPos;
		playerPos.x += 0.5f;
		playerPos.y += 0.5f;
		playerPos.x *= SCALE_X;
		playerPos.y *= SCALE_Y;
		mat = glm::translate(mat, playerPos);
		mat = glm::scale(mat, glm::vec3(texture->getWidth(),texture->getHeight(),1));
		window.drawSprite(to_mat(mat), texture);
	}

	struct Functor {
		GameObject spawn(const std::string& type) {
			if(type=="Background") {
				return GameObject{};
			}
			return GameObject{};
		}

		std::function<std::shared_ptr<mikroplot::Texture>(const std::string&)> loadTexture;

	}; // end - struct Functor

} // end - namespace app

/*



// Main function
int main() {
	using namespace mikroplot;
	using namespace platformer;
	using namespace app;
	// Create application window and run it.
	mikroplot::Window window(WINDOW_SIZE_X/2, WINDOW_SIZE_Y/2, "");
	Functor f;
	typedef World<GameObject> World;
	f.loadTexture = [&window](const std::string& fileName) {
		return window.loadTexture(fileName);
	};
	auto state = reset<World,Functor>(f, GAME_LONG_NAME, "world1");
	return 	window.run([&](mikroplot::Window& window, float dt, float time) {
		app::render(window, update(window, state, f, dt), time);
	});
}
*/
