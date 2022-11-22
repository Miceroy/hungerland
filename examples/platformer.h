#include <hungerland/util.h>
#include <hungerland/map.h>
#include <mikroplot/window.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtx/matrix_operation.hpp>

namespace math = glm;
using namespace std;
using namespace hungerland;

namespace platformer {
	/// CONFIG:
	static const size_t	WINDOW_SIZE_X = 1920;
	static const size_t	WINDOW_SIZE_Y = 1080;

	///
	/// \brief platform::world::loadScene function.
	/// \param f
	/// \param world
	/// \param mapFilename
	/// \param backgroundFilename
	/// \param objectTextureFiles
	///
	template<typename Functor, typename World, typename Config>
	void loadScene(Functor f, World& world, size_t index, const Config& cfg) {
		// Create map layers by map and tileset.
		world.mapLayers = hungerland::map::load(f, cfg.mapFiles[index]);

		// Load background texture
		world.background = f.loadTexture(cfg.backgroundFiles[index]);
		if(world.background == 0) {
			util::ERROR("Failed to load background image (index=" + std::to_string(index)+ ") file: \"" + cfg.backgroundFiles[index] + "\"!");
		}
		util::INFO("Loaded background texture: " + cfg.backgroundFiles[index]);

		// Load object textures
		for(const auto& filename : cfg.characterTextureFiles) {
			auto texture = f.loadTexture(filename);
			if(texture== 0) {
				util::ERROR("Failed to load object texture file: \"" + filename + "\"!");
			}
			util::INFO("Loaded object texture: " + filename);
			world.characterTextures.push_back(texture);
		}

		// Load item textures
		for(const auto& filename : cfg.characterTextureFiles) {
			auto texture = f.loadTexture(filename);
			if(texture== 0) {
				util::ERROR("Failed to load item textures: \"" + filename + "\"!");
			}
			util::INFO("Loaded object texture: " + filename);
			world.characterTextures.push_back(texture);
		}

		// Get map x and y sizes
		auto mapSize = world.mapLayers.getMapSize();
		// and adjust camera and to center y and left of map.
		world.camera.position = math::vec3(0, mapSize.y/2, 0);
		world.player.position = math::vec3(0, mapSize.y/2, 0);
	}

	///
	/// \brief platform::world::reset
	/// \param f
	/// \param name
	/// \param initialMap
	///
	template<typename Functor, typename World, typename Window, typename Config>
	auto reset(Window& window, const std::string& name, const Config& cfg) {
		auto s = World();
		s.sceneName = name;
		loadScene(window, s, 0, cfg);
		return s;
	};

	struct Config {
		std::vector<std::string> backgroundFiles;
		std::vector<std::string> mapFiles;
		std::vector<std::string> characterTextureFiles;
		std::vector<std::string> itemTextureFiles;
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

		// Camera follows player x
		state.camera.position.x = state.player.position.x;
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
		std::vector<std::shared_ptr<mikroplot::Texture> > characterTextures;
		std::vector<std::shared_ptr<mikroplot::Texture> > itemTextures;
		TileMap mapLayers;
		//glm::vec3 cameraPos;
		GameObject camera;
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
		auto windSize = window.getWindowSize<glm::vec2>();
		// Aseta origo ruudun vasempaan alareunaan:
		static const auto SIZE_X = float(WINDOW_SIZE_X/2);
		static const auto SIZE_Y = float(WINDOW_SIZE_Y/2);
		static const auto SCALE_X = state.mapLayers.getTileSize().x;
		static const auto SCALE_Y = state.mapLayers.getTileSize().y;
		auto pr = to_glm(window.setScreen(-SIZE_X, SIZE_X , SIZE_Y, -SIZE_Y));
		auto projOffset = glm::vec3(SCALE_X/2,SCALE_Y/2,0); // Offset of half tiles to make look pritty.
		glm::mat4 projection = glm::ortho(-SIZE_X, SIZE_X , SIZE_Y, -SIZE_Y);
		projection = glm::translate(projection,projOffset);

		// Render Tilemap
		glm::mat4 mat = glm::mat4(1);
		auto HALF_TILE = glm::vec3(0.5,0.5,0);
		auto mapPos = state.camera.position + HALF_TILE;
		mapPos.x *= SCALE_X;
		mapPos.y *= SCALE_Y;
		mat = glm::translate(mat, mapPos);
		glm::vec2 cameraDelta(0);
		cameraDelta.x = state.camera.position.x * SCALE_X;
		cameraDelta.y = state.camera.position.y * SCALE_Y;
		state.mapLayers.render(to_vec(projection*glm::inverse(mat)), cameraDelta);

		// Render Player
		mat = glm::mat4(1);
		auto texture = state.characterTextures[0].get();
		auto playerPos = state.player.position - state.camera.position;
		playerPos.x *= SCALE_X;
		playerPos.y *= SCALE_Y;
		mat = glm::translate(mat, playerPos+projOffset);
		mat = glm::scale(mat, glm::vec3(SCALE_Y,SCALE_X,1));
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
EXAMPLE CPP FILE

namespace example {
	static const std::string GAME_NAME = "Platformer Game";
	static const std::string GAME_VERSION = "v0.0.1";
	static const std::string GAME_LONG_NAME = GAME_NAME + " " + GAME_VERSION;
}

// Example Main function
int main() {
	using namespace mikroplot;
	using namespace platformer;
	using namespace app;
	using namespace example;
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
*/
