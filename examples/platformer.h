#include <hungerland/util.h>
#include <hungerland/map.h>
#include <hungerland/window.h>
#include <hungerland/math.h>

namespace math = glm;
using namespace hungerland;

namespace platformer {

	namespace phys {
		///
		/// \brief integrateBody
		/// \param getCollision
		/// \param body
		/// \param F
		/// \param I
		/// \param dt
		///
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
				// Done?
				if(deltaTime>dt || dt < 0.001f) {
					break;
				}
			}
			auto resN = glm::dot(resI,resI) > 0.0f ? glm::normalize(resI) : resI;
			return std::make_tuple(resBody, resN, resI);
		}

		///
		/// \brief updateBodyImpact
		/// \param body
		/// \param N
		///
		template<typename Body>
		auto updateBodyImpact(Body body, const glm::vec3& N) {
			// Collision response after integrate:
			if(glm::length(N) > 0){
				body.velocity = glm::reflect(body.velocity, glm::normalize(N));
				body.acceleration.x = 0;
				body.acceleration.y = 0;
			}
			return body;
		};

		///
		/// \brief limitVelocity
		/// \param body
		/// \param max
		///
		template<typename Body>
		auto limitVelocity(Body body, glm::vec3 max) {
			// Clamp to VX:
			auto speedX = body.velocity.x;
			if(std::abs(speedX) > max.x) {
				body.velocity.x = std::signbit(speedX) ? -max.x : max.x;
			}
			auto speedY = body.velocity.y;
			if(std::abs(speedY) > max.y) {
				body.velocity.y = std::signbit(speedY) ? -max.y : max.y;
			}
			auto speedZ = body.velocity.z;
			if(std::abs(speedZ) > max.z) {
				body.velocity.z = std::signbit(speedZ) ? -max.z : max.z;
			}
			return body;
		};
	}

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
		world.tileMap = hungerland::map::load<hungerland::map::Map>(f, cfg.mapFiles[index], false);

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
		auto mapSize = world.tileMap->getMapSize();
		// and adjust camera and to center y and left of map.
		world.player.position = math::vec3(5, mapSize.y/2, 0);
		world.camera.position = math::vec3(0, mapSize.y/2, 0);
	}

	namespace player {
		struct Input {
			float dx;
			bool accelerate;
			bool wantJump;
		};

		const float VX_MAX = 6;
		const float G = -20;



		template<typename Character>
		auto updateCharacterImpact(Character character, const glm::vec3& N) {
			// Collision response after integrate:
			character = phys::updateBodyImpact(character, N);
			character.grounded	|= N.y > 0;
			character.topped	|= N.y < 0;
			character.hitWall	|= N.x == 1;
			character.hitWall	|= N.x == -1;
			return character;
		};

		template<typename World, typename Character>
		auto updateCharacter(const World& world, Character character, float vMax, float dx, bool accelerate, bool groundJump, bool wallJump, float dt) {
			const float PY = 10;
			const float VX_ACC = 100;
			const float VX_BREAK = 100;
			const float S = 2.0f;
			auto checkCollisions = [&world](const Character& body) {
				return world.tileMap->checkCollision(body.position, glm::vec3(0.5, 0.5, 0.0), glm::length(body.velocity));
			};

			// First, Integrate body movement by environment forces:
			{
				auto totalForce = glm::vec3(0,0,0);
				auto totalImpulse = glm::vec3(0,0,0);
				const glm::vec3 gravity(0, G, 0);
				totalForce += gravity;
				// Intergate:
				auto [newBody, N, I] = phys::integrateBody(checkCollisions , character, totalForce, totalImpulse, dt);
				newBody = phys::limitVelocity(newBody, glm::vec3(vMax ,vMax, 0));
				newBody = updateCharacterImpact(newBody, N);
				character = newBody;
			}

			// Constraint Character:
			{
				if(character.grounded) {
					// If grounded, set velocity y=0:
					character.velocity.y = 0;
				}
				if(character.hitWall) {
					// If hitWall, set velocity y=0:
					character.velocity.x = 0;
				}
			}

			// Then apply character movement:
			{
				auto totalForce = glm::vec3(0,0,0);
				auto totalImpulse = glm::vec3(0,0,0);
				if(groundJump) {
					// Gound jump list/right
					totalImpulse.y = PY;
					character.acceleration.y = 0;
				} else if (wallJump) {
					// Wall jump list/right
					totalImpulse.y = PY;
					totalImpulse.x = -dx*PY;
					character.acceleration.x = 0;
					character.acceleration.y = 0;
					character.velocity.x = 0;
					character.velocity.y = 0;
				} else if(dx != 0) {
					// Move list/right:
					totalForce.x = VX_ACC * dx * ((character.grounded && accelerate) ? S : 1);
					character.acceleration.x = 0;
				} else {
					// Friction x:
					character.acceleration.x = 0;
					auto vx = character.velocity.x;
					if(fabsf(vx) > 1) {
						auto dir = vx/fabsf(vx);
						totalForce.x = -vx*VX_BREAK;
					} else if(fabsf(vx) > 0) {
						totalForce.x = -vx*VX_BREAK;
					}
				}
				// Intergate:
				auto [newBody, N, I] = phys::integrateBody(checkCollisions , character, totalForce, totalImpulse, dt);
				newBody = phys::limitVelocity(newBody, glm::vec3(vMax ,vMax, 0));
				newBody = updateCharacterImpact(newBody, N);
				return newBody;
			}
		};

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
			player.grounded = false;
			player.topped = false;
			player.hitWall = false;
			return updateCharacter(world, player, VX_MAX, ui.dx, ui.accelerate, groundJump, wallJump, dt);
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

			auto mapSize = world.tileMap->getMapSize();
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
		std::shared_ptr<map::Map> tileMap;
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
		auto renderMapLayers = [](const map::Map& mapLayers, glm::mat4 matProj, const size2d_t& sizeInPixels, glm::vec3 cameraPosition) {
			// Flip camera y and offset
			cameraPosition.y =  mapLayers.getMapSize().y-cameraPosition.y-1;
			// And offset
			glm::mat4 mat = glm::mat4(1);
			const glm::vec3 SCALE = {mapLayers.getTileSize().x, mapLayers.getTileSize().y, 1};
			auto mapScreenPos = SCALE * (cameraPosition + MAP_OFFSET);
			mat = glm::translate(mat, mapScreenPos);
			matProj = glm::translate(matProj, 0.5f * SCALE);
			glm::vec2 cameraDelta = cameraPosition * SCALE;
			map::draw(mapLayers, to_vec(matProj*glm::inverse(mat)), cameraDelta);
			return matProj;
		};

		auto renderSprite = [](screen::Screen& screen, const glm::mat4& matProj, const size2d_t& sizeInPixels, const glm::vec3& cameraPosition, glm::vec3 position, const texture::Texture* texture) {
			// Flip x and y
			position.x = position.x - cameraPosition.x;
			position.y = cameraPosition.y - position.y;
			// And offset
			position += MAP_OFFSET;
			position *= glm::vec3(sizeInPixels.x, sizeInPixels.y, 1);

			glm::mat4 mat = glm::mat4(1);
			mat = glm::translate(mat, position);
			mat = glm::scale(mat, glm::vec3(sizeInPixels.x,sizeInPixels.y, 1));
			screen.drawSprite(mat, texture);
		};

		// Render Tilemap
		projection = renderMapLayers(*state.tileMap, projection, state.tileMap->getTileSize(),
					state.camera.position);
		// Render Player
		renderSprite(screen, projection, state.tileMap->getTileSize(),
					state.camera.position,
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
