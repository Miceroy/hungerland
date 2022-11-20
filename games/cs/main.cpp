//// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= ////
//// CyberSpace - Design, art: Roope Romppainen, Code: Mikko Romppainen
////
////
//// Copyright (c) 2022 Mikko Romppainen.
////
//// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= ////
#include "mikroplot/texture.h"
#include <mikroplot/window.h>
#include <random>
#include <stdexcept>
#include <tuple>
#include <glm/glm.hpp>
#include <tmxlite/Map.hpp>
#include "tmxlite/TileLayer.hpp"
#include "util.h"
#include "MapLayer.hpp"
#include <glm/gtc/matrix_transform.hpp>


namespace cs {
namespace math = glm;
namespace std = ::std;

/// CONFIG:
static const size_t	WINDOW_SIZE_X = 1920;
static const size_t	WINDOW_SIZE_Y = 1080;
static const float	SCALE_X = 96.0f;
static const float	SCALE_Y = 96.0f;
static const std::string GAME_NAME = "Cyber Space";
static const std::string GAME_VERSION = "v0.0.1";
static const std::string GAME_LONG_NAME = GAME_NAME + " " + GAME_VERSION;



namespace world {
	auto TileMap(const tmx::Map& map, const tmx::TileLayer& layer, const std::vector< std::shared_ptr<mikroplot::Texture> >& tileTextures) {
		return MapLayer(map, layer, tileTextures);
	}

	template<typename Functor, typename World>
	void loadMap(Functor f, World& world, const std::string& mapFilename, const std::string& backgroundFilename, std::vector<std::string> objectTextureFiles) {
		// Load map
		if(false == world.map.load(mapFilename)) {
			csut::ERROR("Failed to load map file: \"" + mapFilename + "\"!");
		}
		csut::INFO("Loaded Tiled map: " + mapFilename);

		// Create tileset textures from map tilesets.
		std::vector< std::shared_ptr<mikroplot::Texture> > mapTextures;
		for(const auto& ts : world.map.getTilesets()) {
			auto texture = f.loadTexture(ts.getImagePath());
			if(texture == 0) {
				csut::ERROR("Failed to load tileset texture file: \"" + ts.getImagePath() + "\"!");
			}
			csut::INFO("Loaded tileset texture: " + ts.getImagePath());
			mapTextures.push_back(texture);
		}

		// Create map layers by map and tileset.
		world.mapLayers = cs::MapLayers(world.map, mapTextures);

		// Load background texture
		world.background = f.loadTexture(backgroundFilename);
		if(world.background == 0) {
			csut::ERROR("Failed to load background image file: \"" + backgroundFilename + "\"!");
		}
		csut::INFO("Loaded background texture: " + mapFilename);

		// Load object textures
		for(const auto& filename : objectTextureFiles) {
			auto texture = f.loadTexture(filename);
			if(texture== 0) {
				csut::ERROR("Failed to load object texture file: \"" + filename + "\"!");
			}
			csut::INFO("Loaded object texture: " + filename);
			world.objectTextures.push_back(texture);
		}

		auto tileCount = world.map.getTileCount();
		//world.mapOffset = math::vec3(0, 0, 0);

		world.cameraPos = math::vec3(0, tileCount.y/2, 0);
		world.playerPos = math::vec3(0, tileCount.y/2, 0);
	}

	template<typename Entity>
	struct World {
		std::string sceneName;
		std::shared_ptr<mikroplot::Texture> background;
		std::vector<std::shared_ptr<mikroplot::Texture> > objectTextures;
		tmx::Map map;
		cs::MapLayers mapLayers;
		//glm::vec3 mapOffset;
		glm::vec3 cameraPos;
		glm::vec3 playerPos;
		std::vector<Entity> actors;
	};



	///
	/// \brief reset
	/// \param name
	///
	template<typename Functor, typename Entity>
	auto reset(Functor f, const std::string& name, std::string initialMap) {
		// File names
		std::vector<std::string> objectTextureFiles = {
			"assets/images/Player.png",
			"assets/images/Enemy World 1.png"
		};
		auto mapFilename = "assets/"+initialMap+".tmx";
		auto backgroundFilename = "assets/images/BG World 1.png";

		auto s = World<Entity>();
		s.sceneName = name;

		loadMap(f, s, mapFilename, backgroundFilename, objectTextureFiles);

		return s;
	};

	///
	/// \brief world::update
	/// \param dt
	/// \return
	///
	template<typename Window, typename World, typename Functor>
	const auto& update(const Window& window, World& state, Functor f, float dt) {
		// Move camera to right

		//state.cameraPos.x += 1.0f*dt;
		float dx = window.getKeyState(mikroplot::KEY_RIGHT) - window.getKeyState(mikroplot::KEY_LEFT);

		if(window.getKeyState(mikroplot::KEY_LEFT_SHIFT)+window.getKeyState(mikroplot::KEY_RIGHT_SHIFT)) {
			state.playerPos.x += 25.0f*dx*dt;
		} else {
			state.playerPos.x += 5.0f*dx*dt;
		}

		// Camera follows player
		state.cameraPos = state.playerPos;
		return state;
	};

	namespace player {
		///
		/// \brief 	world::player::update
		/// \param world
		/// \param player
		/// \param dt
		///
		template<typename Entity>
		auto update(world::World<Entity> world, const Entity& player, float dt) {
			return world;
		};
	}

}


}

namespace app {
	struct State {
		glm::vec3 positions;
		std::vector<mikroplot::Texture> textures;
	};

	struct Entity {
		size_t positionId;
		size_t visualId;
		size_t behavioralId;
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
	void render(mikroplot::Window& window, const cs::world::World<Entity>& state, float time) {
		window.setTitle(state.sceneName);
		window.setClearColor();
		// Aseta origo ruudun vasempaan alareunaan:
		static const auto SIZE_X = cs::WINDOW_SIZE_X/2.0f;
		static const auto SIZE_Y = cs::WINDOW_SIZE_Y/2.0f;
		auto projection = to_glm(window.setScreen(-SIZE_X, SIZE_X , SIZE_Y, -SIZE_Y));

		// Background
		auto bgPos = glm::scale(glm::mat4(1.0f), glm::vec3(1920,1080,0));
		window.drawSprite(to_mat(bgPos), state.background.get());

		// Tilemap
		glm::mat4 mat = glm::mat4(1);
		glm::vec3 camPos = state.cameraPos;
		auto mapPos = camPos;
		mapPos.x *= cs::SCALE_X;
		mapPos.y *= cs::SCALE_Y;
		mat = glm::translate(mat, mapPos);
		state.mapLayers.render(to_vec(projection*glm::inverse(mat)));

		// Player
		mat = glm::mat4(1);
		auto texture = state.objectTextures[0].get();
		auto playerPos = state.playerPos - state.cameraPos;
		playerPos.x += 0.5f;
		playerPos.y += 0.5f;
		playerPos.x *= cs::SCALE_X;
		playerPos.y *= cs::SCALE_Y;
		mat = glm::translate(mat, playerPos);
		mat = glm::scale(mat, glm::vec3(texture->getWidth(),texture->getHeight(),1));
		window.drawSprite(to_mat(mat), texture);

#if 0
		// sprite 2
		mikroplot::Grid pixels =Sprite("");
		transform[3][0] = -1;
		transform[3][1] = -2;
		//pixels = { {14,15}, {16,17}};
		window.drawSprite(transform,pixels, {
			mikroplot::Constant("t",{time})
		},
			"color *= vec4(abs(sin(t)), abs(sin(t)), abs(sin(t)), 1.0f);"
		);

		// sprite 3
		transform[3][0] = -3;
		transform[3][1] = -1;
		//pixels = { {18,19}, {20,20}};
		window.drawSprite(transform,pixels);

		// sprite 4
		transform[3][0] = -3;
		transform[3][1] = 2;
		window.drawSprite(transform,pixels,{
			mikroplot::Constant("t",{time})
		},
			"color = vec4(max(color.r,abs(sin(10*t))), color.g, color.b, color.a);"
		);
#endif
	}

	struct Functor {
		Entity spawn(const std::string& type) {
			if(type=="Background") {
				return Entity{};
			}
			return Entity{};
		}

		std::function<std::shared_ptr<mikroplot::Texture>(const std::string&)> loadTexture;

	}; // end - struct Functor

} // end - namespace app


// Main function
int main() {
	// Create application window and run it.
	mikroplot::Window window(cs::WINDOW_SIZE_X/2, cs::WINDOW_SIZE_Y/2, "");
	app::Functor f;
	typedef cs::world::World<app::Entity> World;
	f.loadTexture = [&window](const std::string& fileName) {
		return window.loadTexture(fileName);
	};

	//auto state = cs::world::reset<app::Functor,app::Entity>(f, cs::GAME_LONG_NAME, "world1");
	auto state = cs::world::reset<app::Functor,app::Entity>(f, cs::GAME_LONG_NAME, "world1");
	return 	window.run([&](mikroplot::Window& window, float dt, float time) {
		app::render(window, cs::world::update(window, state, f, dt), time);
	});
}
