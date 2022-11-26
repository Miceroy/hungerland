#include <hungerland/util.h>
#include <hungerland/map.h>
#include <hungerland/window.h>
#include <hungerland/math.h>

namespace math = glm;
using namespace hungerland;

namespace platformer {
	/// CONFIG:
	static const size_t	SCREEN_SIZE_X = 1920;
	static const size_t	SCREEN_SIZE_Y = 1080;

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
		world.player.position = math::vec3(5, mapSize.y/2, 0);
		world.camera.position = math::vec3(0, mapSize.y/2, 0);
	}

	///
	/// \brief checkMapLimits checks object collision against map limits.
	/// \param tileMap
	/// \param obj
	/// \return Return 0 if no collision or normal and velocity coded in single vector.
	///
	template<typename TileMap, typename Object>
	glm::vec3 checkMapLimits(const TileMap& tileMap, const Object& obj) {
		auto mapSize = tileMap.getMapSize();
		mapSize.x -= 1;
		mapSize.y -= 1;
		auto posX = obj.position.x;
		auto posY = obj.position.y;

		// Compute collision normal from overlapping objects.
		glm::vec3 normal(0);
		if(posX < 0)			normal.x =  1;
		if(posX > mapSize.x)	normal.x = -1;
		if(posY < 0)			normal.y =  1;
		if(posY > mapSize.y)	normal.y = -1;

		if(glm::dot(normal,normal) > 0) {
			return glm::length(obj.velocity)*glm::normalize(normal);
		}
		return glm::vec3(0);
	}

	namespace phys {

		template<typename Body, typename CollisionFunc>
		auto integrateBody(const CollisionFunc& getCollision, const Body& body, const glm::vec3& F, const glm::vec3& I, float dt) {
			auto euler = [&](Body body, float dt) {
				// Integrate velocity from forces and position from velocity:
				auto i = dt * (F + body.acceleration);
				body.velocity += I + i;
				body.position += body.velocity * dt;
				return std::make_tuple(body, getCollision(body));
			};
			auto resI = glm::vec3(0);
			auto deltaTime = dt;
			auto resBody = body;
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

		template<typename Character>
		auto constraintCharacter(Character character, const glm::vec3& N) {
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

		template<typename World, typename Character>
		auto updateCharacter(const World& world, Character character, float vMax, float dx, bool accelerate, bool groundJump, bool wallJump, float dt) {
			const float PY = 15;
			const float VX_ACC = 100;
			const float VX_BREAK = 100;
			const float S = 2.0f;
			auto checkCollisions = [&world](const Character& body){
				return checkMapLimits<TileMap,Character>(world.tileMap, body);
			};

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

			auto [newBody, N, I] = integrateBody(checkCollisions , character, totalForce, totalImpulse, dt);
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
			auto checkCollisions = [&world](const Player& body){
				return checkMapLimits<TileMap,Player>(world.tileMap, body);
			};
			auto [newBody, N, I] = integrateBody(checkCollisions, body, totalForce, totalImpulse, dt);
			return constraintObject(body, constraintCharacter(newBody, N), vMax, N);
		};

	}
	namespace player {
		struct Input {
			float dx;
			bool accelerate;
			bool wantJump;
		};

		const float VX_MAX = 6;
		const float G = -20;
		///
		/// \brief 	world::player::update
		/// \param ui
		/// \param world
		/// \param player
		/// \param dt
		///
		template<typename Input, typename World, typename Player>
		auto update(const Input& ui, const World& world, Player player, float dt) {
			// Add player update by input:

			bool groundJump	= ui.wantJump && player.grounded;
			bool wallJump	= ui.wantJump && player.hitWall;

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
			return phys::updateCharacter(world, player, VX_MAX, ui.dx, ui.accelerate, groundJump, wallJump, dt);
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
		template<typename World, typename Camera>
		auto update(const World& world, Camera camera, float dt) {
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


	template<typename World, typename Functor, typename Input>
	const auto& update(World& world, Functor f, Input input, float dt) {
		typedef decltype(world.player) Player;
		world.player = player::update(input, world, world.player, dt);
		world.camera = camera::update(world, world.camera, dt);
		/*printf("Player=<%2.2f, %2.2f> Camera=<%2.2f, %2.2f> Grounded:%d, Topped:%d, Walled:%d \n",
			   world.player.position.x, world.player.position.y, world.camera.position.x, world.camera.position.y,
			   world.player.grounded, world.player.topped, world.player.hitWall);*/
		return world;
	};

}


namespace app {

	struct Config {
		std::vector<std::string> backgroundFiles;
		std::vector<std::string> mapFiles;
		std::vector<std::string> characterTextureFiles;
		std::vector<std::string> itemTextureFiles;
	};

	///
	/// \brief The World class
	///
	template<typename GameObject>
	struct World {
		std::string sceneName;
		std::vector<std::shared_ptr<texture::Texture> > characterTextures;
		std::vector<std::shared_ptr<texture::Texture> > itemTextures;
		TileMap tileMap;
		GameObject camera;
		GameObject player;
		std::vector<GameObject> actors;
	};

	struct GameObject {
		glm::vec3 position;
		glm::vec3 velocity = glm::vec3(0);
		glm::vec3 acceleration = glm::vec3(0);
		bool grounded = false;
		bool topped = false;
		bool hitWall = false;
	};

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
	/// \brief app::reset
	/// \param window
	/// \param name
	/// \param cfg
	///
	template<typename World, typename Functor, typename Config>
	auto reset(Functor f, const std::string& name, const Config& cfg) {
		auto world = World();
		world.sceneName = name;
		platformer::loadScene(f, world, 0, cfg);
		return world;
	};


	///
	/// \brief app::render
	/// \param window
	/// \param state
	/// \param time
	///
	void render(screen::Screen& screen, const app::World<GameObject>& state) {
		static const auto MAP_OFFSET = glm::vec3(0.5, 0.5, 0);

		using namespace platformer;

		// Aseta origo ruudun vasempaan alareunaan:
		const auto SIZE_X = float(SCREEN_SIZE_X/2);
		const auto SIZE_Y = float(SCREEN_SIZE_Y/2);
		screen.setScreen(-SIZE_X, SIZE_X , SIZE_Y, -SIZE_Y);
		screen.clear(0, 0, 0, 0);
		glm::mat4 projection = glm::ortho(-SIZE_X, SIZE_X , SIZE_Y, -SIZE_Y);


		// Offset of half tiles to look at centers of tiles.
		auto renderMap = [](const TileMap& mapLayers, glm::mat4 matProj, const size2d_t& sizeInPixels, const glm::vec3& cameraPosition) {
			const auto SCALE_X = mapLayers.getTileSize().x;
			const auto SCALE_Y = mapLayers.getTileSize().y;
			const auto PROJ_OFFSET = glm::vec3(SCALE_X/2,SCALE_Y/2,0);

			auto camPos = cameraPosition;
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

		auto renderObject = [](screen::Screen& screen, const glm::mat4& matProj, const size2d_t& sizeInPixels, const glm::vec3& cameraPosition, glm::vec3 position, const texture::Texture* texture) {
			auto camPos = cameraPosition;
			position.x = position.x - camPos.x + MAP_OFFSET.x; // Flip x and offset.
			position.y = camPos.y - position.y + MAP_OFFSET.y; // Flip y and offset.
			position.x *= sizeInPixels.x;
			position.y *= sizeInPixels.y;
			glm::mat4 mat = glm::mat4(1);
			mat = glm::translate(mat, position);
			mat = glm::scale(mat, glm::vec3(sizeInPixels.x,sizeInPixels.y, 1));
			screen.drawSprite(to_mat(mat), texture);
		};

		// Render Tilemap
		projection = renderMap(state.tileMap, projection, state.tileMap.getTileSize(),
					state.camera.position);
		// Render Player
		renderObject(screen, projection, state.tileMap.getTileSize(), state.camera.position,
					state.player.position, state.characterTextures[0].get());
	}

	///
	/// \brief The Functor class
	///
	struct Functor {
		GameObject spawn(const std::string& type) {
			if(type=="Background") {
				return GameObject{};
			}
			return GameObject{};
		}

		std::function<std::shared_ptr<texture::Texture>(const std::string&,bool)> loadTexture;

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
