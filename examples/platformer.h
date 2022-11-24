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
		world.tileMap = hungerland::map::load<hungerland::TileMap>(f, cfg.mapFiles[index], false);

		// Load background texture
/*		world.background = f.loadTexture(cfg.backgroundFiles[index]);
		if(world.background == 0) {
			util::ERROR("Failed to load background image (index=" + std::to_string(index)+ ") file: \"" + cfg.backgroundFiles[index] + "\"!");
		}
		util::INFO("Loaded background texture: " + cfg.backgroundFiles[index]);
*/
		// Load object textures
		for(const auto& filename : cfg.characterTextureFiles) {
			auto texture = f.loadTexture(filename, false);
			if(texture== 0) {
				util::ERROR("Failed to load object texture file: \"" + filename + "\"!");
			}
			util::INFO("Loaded object texture: " + filename);
			world.characterTextures.push_back(texture);
		}

		// Load item textures
		for(const auto& filename : cfg.itemTextureFiles) {
			auto texture = f.loadTexture(filename, false);
			if(texture== 0) {
				util::ERROR("Failed to load item textures: \"" + filename + "\"!");
			}
			util::INFO("Loaded item texture: " + filename);
			world.itemTextures.push_back(texture);
		}

		// Get map x and y sizes
		auto mapSize = world.tileMap.getMapSize();
		// and adjust camera and to center y and left of map.
		//world.camera.position = math::vec3(0, mapSize.y/2, 0);
		world.player.position = math::vec3(0, 0, 0);
		world.camera.position = math::vec3(0, 0, 0);
		//world.player.position = math::vec3(0, mapSize.y, 0);
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

	template<typename TileMap, typename Object>
	glm::vec3 checkMapCollisions(const TileMap& tileMap, const Object& obj) {
		auto mapSize = tileMap.getMapSize();
		mapSize.x -= 1;
		mapSize.y -= 1;
		float posX = obj.position.x;
		float posY = obj.position.y;

		glm::vec3 normal(0);
		if(posX < 0) normal.x = 1;
		if(posX > mapSize.x) normal.x = -1;
		if(posY < 0) normal.y = 1;
		if(posY > mapSize.y) normal.y = -1;
		if(glm::dot(normal,normal) > 0){
			return glm::length(obj.velocity)*glm::normalize(normal);
		}
		return glm::vec3(0);
	}

	namespace phys {

		template<typename World, typename Player>
		auto integrateBody(const World& world, const Player& body, const glm::vec3& F, const glm::vec3& I, float dt) {
			auto euler = [&I,&F,&world](Player body, float dt) {
				// Integrate velocity from forces and position from velocity:
				auto i = dt * (F + body.acceleration);
				body.velocity += I + i;
				body.position += body.velocity * dt;
				return std::make_tuple(body, checkMapCollisions(world.tileMap, body));
			};
			auto resI = glm::vec3(0);
			float deltaTime = dt;
			Player resBody = body;
			for(size_t i=0; i<5; ++i){
				const float step = 0.5*deltaTime;
				auto [newBody, collision] = euler(body, deltaTime);
				if(glm::dot(collision,collision) > 0) {
					// If collides at first step, set impulse:
					if(glm::dot(resI,resI)==0 ) resI = collision;
					// Adjust delta time to be smaller:
					deltaTime -= step;
				} else {
					// Not collides, adjust delta time to be bigger:
					deltaTime += step;
					resBody = newBody;
				}
				if(deltaTime>dt || dt < 0.001f) {
					break;
				}
			}
			auto resN = glm::dot(resI,resI) > 0.0f ? glm::normalize(resI) : resI;
			return std::make_tuple(resBody, resN, resI);
		}

		template<typename Player>
		auto limitVelocityX(Player body, float maxX) {
			// Clamp to VX:
			auto speedX = body.velocity.x;
			if(fabsf(speedX) > maxX) {
				body.velocity.x = (speedX/fabsf(speedX))*maxX;
			}
			return body;
		};

		template<typename Player>
		auto constraintCharacter(Player character, const glm::vec3& N) {
			// Collision response after integrate:
			character.grounded |= N.y > 0;
			character.topped |= N.y < 0;
			character.hitWall  |= (N.x != 0);
			if(character.hitWall) {
				character.velocity.x = 0;
				character.velocity.y = 0;
				character.acceleration.x = 0;
				character.acceleration.y = 0;
			}
			if(character.topped) {
				character.velocity.y = 0;
				character.acceleration.x = 0;
				character.acceleration.y = 0;
			}
			return character;
		};

		template<typename World, typename Player>
		auto updateCharacter(const World& world, Player character, float vMax, float dx, bool accelerate, bool groundJump, bool wallJump, float dt) {
			const float PY = 15;
			const float VX_ACC = 100;
			const float VX_BREAK = 100;
			const float S = 2.0f;

			glm::vec3 totalForce(0,0,0);
			glm::vec3 totalImpulse(0,0,0);
			float accX = 0;
			if(groundJump) {
				totalImpulse.y = PY;
				character.acceleration.y = 0;
			} else if (wallJump) {
				totalImpulse.y = PY;
				totalImpulse.x = -dx*PY;
				character.acceleration.x = 0;
				character.acceleration.y = 0;
			} else if(dx != 0) {
				accX = VX_ACC * dx * ((character.grounded && accelerate) ? S : 1);
				totalForce.x = accX;
				character.acceleration.x = 0;
			} else {
				character.acceleration.x = 0;
				auto vx = character.velocity.x;
				if(fabsf(vx) > 1) {
					auto dir = vx/fabsf(vx);
					totalForce.x = -vx*VX_BREAK;
				} else if(fabsf(vx) > 0) {
					totalForce.x = -vx*VX_BREAK;
				}
			}

			auto [newBody, N, I] = integrateBody(world, character, totalForce, totalImpulse, dt);
			if(groundJump) {
				newBody.velocity.x = -totalImpulse.x*dt;
			} if (wallJump) {
			} else if(dx != 0) {
				character.acceleration.x -= accX;
			}

			return constraintCharacter(limitVelocityX(newBody, vMax),N);
		};

		template<typename Player>
		auto constraintObject(Player old, Player object, float vMax, const glm::vec3& N) {
			// If grounded, set velocity y=0:
			if(object.grounded) {
				object.velocity.y = 0;
				object.acceleration.y = 0;
			}
			if(object.hitWall) {
				object.velocity.x = 0;
				object.velocity.y = 0;
				object.acceleration.x = 0;
				object.acceleration.y = 0;
			}
			return limitVelocityX(object, vMax);
		};

		template<typename World, typename Player>
		auto updateObject(const World& world, const Player& body, float vMax, const glm::vec3& totalForce, const glm::vec3& totalImpulse, float dt) {
			auto [newBody, N, I] = integrateBody(world, body, totalForce, totalImpulse, dt);
			return constraintObject(body, constraintCharacter(newBody, N), vMax, N);
		};

	}
	namespace player {

		const float VX_MAX = 6;
		const float G = -20;
		///
		/// \brief 	world::player::update
		/// \param ui
		/// \param world
		/// \param player
		/// \param dt
		///
		template<typename UserInterface, typename World, typename Player>
		auto update(const UserInterface& ui, const World& world, Player player, float dt) {
			// Add player update by input:
			float dx		= ui.getKeyState(mikroplot::KEY_RIGHT)			- ui.getKeyState(mikroplot::KEY_LEFT);
			bool accelerate	= ui.getKeyState(mikroplot::KEY_LEFT_SHIFT)		+ ui.getKeyState(mikroplot::KEY_RIGHT_SHIFT);
			bool wantJump	= ui.getKeyPressed(mikroplot::KEY_LEFT_CONTROL) + ui.getKeyPressed(mikroplot::KEY_RIGHT_CONTROL);
			bool groundJump	= wantJump && player.grounded;
			bool wallJump	= wantJump && player.hitWall;

			// Add gravity and integrate player:
			glm::vec3 totalForce(0);
			glm::vec3 totalImpulse(0);
			// Integrate body movement by environment forces:
			{
				const glm::vec3 gravity(0, G, 0);
				totalForce += gravity;
			}
			player.grounded = false;
			player.topped = false;
			player.hitWall = false;
			player = phys::updateObject(world, player, VX_MAX, totalForce, totalImpulse, dt);
			return phys::updateCharacter(world, player, VX_MAX, dx, accelerate, groundJump, wallJump, dt);
		};
	}

	namespace camera {
		///
		/// \brief 	world::camera::update
		/// \param ui
		/// \param world
		/// \param camera
		/// \param dt
		///
		template<typename UserInterface, typename World, typename Camera>
		auto update(const UserInterface& ui, const World& world, Camera camera, float dt) {
			// Camera follows player x:
			camera.position.y = world.player.position.y;
			float dPosX = world.player.position.x-camera.position.x;
			if(dPosX > 7) {
				camera.position.x = world.player.position.x - 7;
			} else if(dPosX < -7) {
				camera.position.x = world.player.position.x + 7;
			}

			auto clamp = [](float v, float min, float max) {
				if(v<min) return min;
				if(v>max) return max;
				return v;
			};

			auto mapSize = world.tileMap.getMapSize();
			float minCamX	= 15.5f;
			float maxCamX	= mapSize.x-minCamX;
			float minCamY	= mapSize.y/2 - 2;
			float maxCamY	= mapSize.y/2;
			camera.position.x = clamp(camera.position.x, minCamX, maxCamX);
			camera.position.y = clamp(camera.position.y, minCamY, maxCamY);
			return camera;
		};
	}

	///
	/// \brief platform::world::update
	/// \param window
	/// \param state
	/// \param f
	/// \param dt
	/// \return
	///
	template<typename Window, typename World, typename Functor>
	const auto& update(const Window& window, World& world, Functor f, float dt) {
		typedef decltype(world.player) Player;
		world.player = player::update(window, world, world.player, dt);
		world.camera = camera::update(window, world, world.camera, dt);
		/*printf("Player=<%2.2f, %2.2f> Camera=<%2.2f, %2.2f> Grounded:%d, Topped:%d, Walled:%d \n",
			   world.player.position.x, world.player.position.y, world.camera.position.x, world.camera.position.y,
			   world.player.grounded, world.player.topped, world.player.hitWall);*/
		return world;
	};

	///
	/// \brief The World class
	///
	template<typename GameObject>
	struct World {
		std::string sceneName;
		//std::shared_ptr<mikroplot::Texture> background;
		std::vector<std::shared_ptr<mikroplot::Texture> > characterTextures;
		std::vector<std::shared_ptr<mikroplot::Texture> > itemTextures;
		TileMap tileMap;
		//glm::vec3 cameraPos;
		GameObject camera;
		GameObject player;
		std::vector<GameObject> actors;
	};
}


namespace app {
	struct GameObject {
		glm::vec3 position;
		glm::vec3 velocity = glm::vec3(0);
		glm::vec3 acceleration = glm::vec3(0);
		bool grounded = false;
		bool topped = false;
		bool hitWall = false;
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
		static const auto MAP_OFFSET = glm::vec3(0.5, 0.5, 0);

		using namespace platformer;
		window.setTitle(state.sceneName);
		window.setClearColor();



		// Aseta origo ruudun vasempaan alareunaan:
		const auto SIZE_X = float(WINDOW_SIZE_X/2);
		const auto SIZE_Y = float(WINDOW_SIZE_Y/2);
		window.setScreen(-SIZE_X, SIZE_X , SIZE_Y, -SIZE_Y);
		glm::mat4 projection = glm::ortho(-SIZE_X, SIZE_X , SIZE_Y, -SIZE_Y);


		auto tileSize = state.tileMap.getTileSize();
		// Offset of half tiles to look at centers of tiles.
		auto renderMap = [](const TileMap& mapLayers, glm::mat4 matProj, const size2d_t& sizeInPixels, const glm::vec3& cameraPosition) {
			const auto SCALE_X = mapLayers.getTileSize().x;
			const auto SCALE_Y = mapLayers.getTileSize().y;
			const auto PROJ_OFFSET = glm::vec3(SCALE_X/2,SCALE_Y/2,0);

			auto camPos = cameraPosition;
			//camPos.x = -camPos.; // Flip y and offset
			camPos.y =  mapLayers.getMapSize().y-camPos.y-1; // Flip y and offset
			auto mapPos = camPos + MAP_OFFSET;  // And offset y
			mapPos.x *= SCALE_X;
			mapPos.y *= SCALE_Y;

			glm::vec2 cameraDelta(0);
			cameraDelta.x = camPos.x * SCALE_X;
			cameraDelta.y = camPos.y * SCALE_Y;

			glm::mat4 mat = glm::mat4(1);
			mat = glm::translate(mat, mapPos);
			matProj = glm::translate(matProj, PROJ_OFFSET);
			mapLayers.render(to_vec(matProj*glm::inverse(mat)), cameraDelta);
			return matProj;
		};

		auto renderObject = [](mikroplot::Window& window, const glm::mat4& matProj, const size2d_t& sizeInPixels, const glm::vec3& cameraPosition, glm::vec3 position, const mikroplot::Texture* texture) {
			auto camPos = cameraPosition;
			position.x = position.x - camPos.x + MAP_OFFSET.x; // Flip x and offset.
			position.y = camPos.y - position.y + MAP_OFFSET.y; // Flip y and offset.
			position.x *= sizeInPixels.x;
			position.y *= sizeInPixels.y;
			glm::mat4 mat = glm::mat4(1);
			mat = glm::translate(mat, position);
			mat = glm::scale(mat, glm::vec3(sizeInPixels.x,sizeInPixels.y, 1));
			window.drawSprite(to_mat(mat), texture);
		};

		// Render Tilemap
		projection = renderMap(state.tileMap, projection, state.tileMap.getTileSize(),
					state.camera.position);
		// Render Player
		renderObject(window, projection, state.tileMap.getTileSize(), state.camera.position,
					state.player.position, state.characterTextures[0].get());
	}

	struct Functor {
		GameObject spawn(const std::string& type) {
			if(type=="Background") {
				return GameObject{};
			}
			return GameObject{};
		}

		std::function<std::shared_ptr<mikroplot::Texture>(const std::string&,bool)> loadTexture;

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
