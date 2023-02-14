///
/// Hungerland architecture for platformer game:
///
/// Controller(s):
///		- Game			= namespace platformer
///			- game::isPlayerWin(world,player) -> bool
///			- game::isPlayerLose(world,player) -> bool
///
///		- Environment	= namespace platformer
///			- env::reset(f,cfg) -> World
///			- env::update(f,world,action,dt) -> const World&
///			- env::loadScene(f,world,index,cfg)
///
///		- Env Actions	= namespace platformer
///			- f(character, map, dS, dt) -> character
///			- action::applyEnv(character,map,dt) -> character
///         - action::applyForce(character,map,totalForce,dt) -> character
///			- action::applyImpulse(character,map,totalImpulse,dt) -> character
///
///		- Agent			= namespace: platformer
///			- character::update(map,character,action,dt)
///			- player::update(action,world,player,dt)
///			- camera::update(world,camera,dt)
///
/// Model(s):
///		- World Model	= namespace model:
///			- Character
///			- World<Character>
///
/// View(s):
///		- View			= namespace app
///			- render(screen, world)
///		- Main			= cpp
///
///



///
/// \ingroup platformer
///
namespace platformer {

///=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
/// Platformer game configuration:
///=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
	/// Pelin piirtoalueen x ja y koko pikseleinä:
	static const unsigned long	SCREEN_SIZE_X = 1920;
	static const unsigned long	SCREEN_SIZE_Y = 1080;

	/// Pelaajahahmon asetukset (suluissa suureen mittayksikkö):
	namespace config {

		/// X-suuntainen liikkuminen, eli kävely/juoksu:

		/// VX_MAX = Hahmon maksimi x-suuntainen nopeus (tileä sekunnissa).
		/// Isompi luku -> korkeampi hahmon maksiminopeus kävellessä.
		const float VX_MAX = 20;

		/// SX = Juoksun maksiminopeuskerroin VX_MAX muuttujan suhteen. Maksimi juoksunopeus=SX*VX_MAX.
		/// Isompi luku -> korkeampi hahmon maksiminopeus juostessa.
		const float SX = 2.0f;

		/// VX_ACC_GROUND = x-suuntainen kiihtyvyys maassa (tileä/sekunnissa^2).
		/// KontrWorldolloi sitä, kuinka nopeasti hahmo saavuttaa maksinopeuden maassa.
		const float VX_ACC_GROUND = 20;

		/// VX_ACC_AIR = x-suuntainen kiihtyvyys ilmassa (tileä/sekunnissa^2).
		/// Kontrolloi sitä, kuinka nopeasti hahmo saavuttaa maksinopeuden ilmassa.
		const float VX_ACC_AIR = 5;

		/// VX_BREAK = x-suuntainen jarrutus kiihtyvyys (tileä/sekunnissa^2).
		/// Kontrolloi sitä, kuinka nopeasti hahmo pysähtyy.
		/// Isompi luku -> nopeampi pysähtyminen.
		const float VX_BREAK = 20;

		/// Y-suuntainen liikkuminen, eli hyppy/putoaminen:

		/// IY = Hypyn voimakkuus (impulssin suuruus).
		/// Isompi luku -> korkeampi hyppy
		const float IY = 10;

		/// GY = Maan vetovoiman voimakkuus (kiihtyvyys=tileä/sekunnissa^2).
		/// Isompio luku -> Nopeampi putoaminen
		const float GY = -20;

		/// VY_MAX = Maksimi y-suuntainen nopeus (tileä sekunnissa).
		/// Rajoittaa maksimiputoamisvauhtia.
		const float VY_MAX = 20;
	}
///=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=



///
/// \ingroup platformer::game
///
namespace game {

	///
	/// \brief isPlayerWin
	/// \param world
	/// \param player
	/// \return
	///
	template<typename World, typename Player>
	bool isPlayerWin(const World& world, const Player& player) {
		return false; // This is toy for now...
	};

	///
	/// \brief isPlayerLose
	/// \param world
	/// \param player
	/// \return
	///
	template<typename World, typename Player>
	bool isPlayerLose(const World& world, const Player& player) {
		return false; // This is toy for now...
	};
} // End - namespace platformer::game
}


#include <hungerland/math.h>
namespace math = glm;
#include <tuple>
#include <hungerland/util.h>

namespace  platformer {


///
/// \ingroup platformer::env
///
namespace env {
	template<typename MapCollision, typename Map, typename Body, typename PenetrateFunc, typename ReactFunc>
	auto integrateBody(const Body& oldBody, const Map& map, PenetrateFunc isPenetrating, ReactFunc reactFunc, glm::vec3 F, glm::vec3 I, float dt) {
		assert(false == isPenetrating(map.checkCollision(oldBody.position,glm::vec3(0.5f))));

		auto stepEuler = [&](Body body, float dt) {
			// Integrate velocity from forces and position from velocity:
			auto i = dt * F;
			body.velocity += I + i;
			body.position += body.velocity * dt;
			return std::make_tuple(body, map.checkCollision(body.position,glm::vec3(0.5f)));
		};

		auto b = oldBody;
		const float TIME_TOL = 0.0001f;
		while(dt > TIME_TOL) {
			MapCollision resCollision;
			auto deltaTime = dt;
			float usedTime = 0;
			auto resBody = b;
			for(size_t i=0; i<4; ++i){
				const float step = 0.5f*deltaTime;
				auto [newBody, collision] = stepEuler(b, deltaTime);
				if(isPenetrating(collision)) {
					// If collides at first step, set impulse:
					if(!isPenetrating(resCollision)) {
						resCollision = collision;
					}
					// Adjust delta time to be smaller:
					deltaTime -= step;
				} else {
					// Not collides, adjust delta time to be bigger:
					usedTime = deltaTime;
					resBody = newBody;
					deltaTime += step;
				}
				if(deltaTime<=TIME_TOL || deltaTime >= dt ) {
					break;
				}
			}
			assert(false == isPenetrating(map.checkCollision(b.position,glm::vec3(0.5f))));
			b = reactFunc(b, resBody, resCollision);
			assert(false == isPenetrating(map.checkCollision(b.position,glm::vec3(0.5f))));
			if(usedTime<=TIME_TOL) {
				break;
			}
			I = glm::vec3(0);
			F = glm::vec3(0);
			dt -= usedTime;
		}
		return b;
	}

} // End - namespace platformer::env


///
/// \ingroup platformer::action
///
namespace action {
namespace react {
	template<typename MapCollision>
	static inline auto getRowSum(const MapCollision& col, size_t y) {
		float res = -1;
		if(y<col.size()) {
			for(size_t x=0; x<col[y].size(); ++x){
				if(col[y][x].y >= 0.0f) {
					if(res<0){
						res = col[y][x].y;
					} else {
						res += col[y][x].y;
					}
				}
			}
		}
		return res;
	}

	template<typename MapCollision>
	static inline auto getColSum(const MapCollision& col, size_t x) {
		float res = -1;
		for(size_t y=0; y<col.size(); ++y) {
			if(col[y][x].x >= 0.0f) {
				if(res<0){
					res = col[y][x].x;
				} else {
					res += col[y][x].x;
				}
			}
		}
		return res;
	}


	template<typename MapCollision>
	void print(MapCollision collisions) {
		using namespace hungerland;
		if(collisions.size()==0){
			collisions = util::gridN<glm::vec3>(3,glm::vec3(-1));
		}
		auto to_str = [](float v) {
			if(v<0.0f){
				v -= 0.05; // rounding
				//return std::to_string(v).substr(0,4);
				return std::string("    ");
			}
			v += 0.05; // rounding
			return " " + std::to_string(v).substr(0,3);
		};
		for(size_t i=0; i<collisions.size(); ++i) {
			auto y = collisions.size()-1-i;
			std::string row = "{";
			for(size_t x=0; x<collisions[y].size(); ++x) {
				row += "<";
				row += to_str(collisions[y][x].x);
				row += ", ";
				row += to_str(collisions[y][x].y);
				row += ">";
				if((x+1)<collisions[y].size()){
					row += ", ";
				}
			}
			util::INFO(row+"}");
		}
		float bottom = getRowSum(collisions, 0);
		float top = getRowSum(collisions, 2);
		float left = getColSum(collisions, 0);
		float right = getColSum(collisions, 2);

		util::INFO("bottom=" + std::to_string(std::signbit(bottom))
				   + " top=" + std::to_string(std::signbit(top))
				   + " left=" + std::to_string(std::signbit(left))
				   + " right=" + std::to_string(std::signbit(right))
				   + "");
	}

	template<typename MapCollision, typename Character>
	auto character(const Character& old, Character character, const MapCollision& collisions) {
		using namespace hungerland;
		util::INFO("Character reaction:");
		print(collisions);
		float bottom = getRowSum(collisions, 0);
		float top = getRowSum(collisions, 2);
		float left = getColSum(collisions, 0);
		float right = getColSum(collisions, 2);
		character.isGrounded = bottom > 0.0f;
		character.isTopped	 = top > 0.0f;
		character.canMoveL	 = true;
		character.canMoveR	 = true;
		if(collisions.size()>0) {
			character.canMoveL	 = collisions[1][0].x <= 0.0f;
			character.canMoveR	 = collisions[1][2].x <= 0.0f;
		}
		character.canJump	 = character.isGrounded;
		float wallLeft = getColSum(collisions, 0);
		float wallRight = getColSum(collisions, 2);
		character.wallJump	 = 0;
		character.wallJump	 *= !character.canJump;
		util::INFO(std::string("Character:")
				   + " pos="+std::to_string(character.position.x)+","+std::to_string(character.position.y)
				   + " isGrounded="+std::to_string(character.isGrounded)
				   + " isTopped="+std::to_string(character.isTopped)
				   + " canMoveL="+std::to_string(character.canMoveL)
				   + " canMoveR="+std::to_string(character.canMoveR)
				   + " canJump="+std::to_string(character.canJump)
				   + " wallJump="+std::to_string(character.wallJump)
				   + "");
		return character;
	};

	template<typename MapCollision, typename Character>
	auto env(const Character& old, Character character, const MapCollision& collisions) {
		character = react::character(old, character, collisions);
		if(character.isTopped) {
			character.velocity.y = -character.velocity.y;
		}
		if(character.isGrounded) {
			character.velocity.y = 0;
		}
		if(!character.canMoveL && character.velocity.x < 0) {
			character.velocity.x = 0;
		}
		if(!character.canMoveR && character.velocity.x > 0) {
			character.velocity.x = 0;
		}
		auto max = glm::vec3(config::VX_MAX,config::VY_MAX,0);
		auto min = -glm::vec3(config::VX_MAX,config::VY_MAX,0);
		character.velocity = glm::clamp(character.velocity, min, max);
		return character;
	};
} // End - namespace action::react

	template<typename MapCollision>
	bool isPenetrating(const MapCollision& col) {
		for(size_t i=0; i<col.size(); ++i) {
			for(size_t j=0; j<col[i].size(); ++j) {
				if(col[i][j].x > 0.0f || col[i][j].y > 0.0f) {
					return true;
				}
			}
		}
		return false;
	}

	///
	/// \brief applyEnv
	/// \param character
	/// \param map
	/// \param dt
	///
	template<typename MapCollision, typename Map, typename Character>
	auto applyEnv(const Character& character, const Map& map, float dt) {
		const auto GRAVITY = glm::vec3(0, config::GY, 0);
		return env::integrateBody<MapCollision>(character, map, isPenetrating<MapCollision>, react::env<MapCollision,Character>, GRAVITY, glm::vec3(0), dt);
	};

	///
	/// \brief applyForce
	/// \param character
	/// \param map
	/// \param totalForce
	/// \param dt
	///
	template<typename MapCollision, typename Map, typename Character>
	auto applyForce(const Character& character, const Map& map, const glm::vec3& totalForce, float dt){
		return env::integrateBody<MapCollision>(character, map, isPenetrating<MapCollision>, react::character<MapCollision,Character>, totalForce, glm::vec3(0), dt);
	};

	///
	/// \brief applyImpulse
	/// \param character
	/// \param map
	/// \param totalImpulse
	/// \param dt
	///
	template<typename MapCollision, typename Map, typename Character>
	auto applyImpulse(const Character& character, const Map& map, const glm::vec3& totalImpulse, float dt){
		return env::integrateBody<MapCollision>(character, map, isPenetrating<MapCollision>, react::character<MapCollision,Character>, glm::vec3(0), totalImpulse, dt);
	};
} // End namespace platformer::action

///
/// \ingroup platformer::agent
///
namespace agent {
	///
	/// \brief The Action class
	///
	struct Action {
		int dx;
		int dy;
		bool accelerate;
		bool wantJump;
	};


	///
	/// \brief update
	/// \param character
	/// \param map
	/// \param input
	/// \param dt
	///
	template<typename MapCollision, typename Map, typename Character>
	auto update(Character character, const Map& map, const Action& action, float dt) {
		using namespace hungerland;
		const auto V_MAX = glm::vec3(config::SX*config::VX_MAX, config::VY_MAX, 0);
		const auto A_MAX = glm::vec3(config::SX*config::VX_ACC_GROUND, config::GY, 0);

		// Integrate character movement by GRAVITY:
		character =  action::applyEnv<MapCollision>(character, map, dt);

		// Integrate character movement by totalForce and totalImpulse:

		// Apply character movement:
		{
			if(action.wantJump && character.canJump) {
				// Gound jump list/right
				util::INFO("Jump");
				auto I = glm::vec3(0, config::IY, 0);
				character = action::applyImpulse<MapCollision>(character, map, I, dt);
			} else if(action.wantJump && !character.canJump && character.wallJump) {
				// Gound jump list/right
				util::INFO("Wall Jump");
				auto I = glm::vec3(character.wallJump*config::IY, config::IY, 0);
				character = action::applyImpulse<MapCollision>(character, map, I, dt);
			} else {
				auto F = glm::vec3(0);
				if((action.dx < 0 && character.canMoveL)
				   || (action.dx > 0 && character.canMoveR)) {
					// Move left:
					if(character.isGrounded) {
						util::INFO("Ground move x");
						F.x += config::VX_ACC_GROUND * action.dx * (action.accelerate ? config::SX : 1);
					} else {
						util::INFO("Air move x");
						F.x += config::VX_ACC_AIR * action.dx * (action.accelerate ? config::SX : 1);
					}

				} else {
					// Friction x:
					util::INFO("Friction x");
					F.x -= character.velocity.x * config::VX_BREAK;
				}
				// Integrate:
				character = action::applyForce<MapCollision>(character, map, F, dt);
			}
		}

		//util::INFO("Endframe");
		return character;
	};
} // End - namespace platformer::agent


///
/// \ingroup platformer::camera
///
namespace camera {
	///
	/// \brief update
	/// \param camera
	/// \param world
	/// \param dt
	///
	template<typename Map, typename Vec, typename Camera>
	auto update(Camera camera,  Map& map, const Vec& targetPos, float dt) {
		// Camera follows player y:
		camera.position.y = targetPos.y;

		// When player near map border, then move camera.
		float dPosX = targetPos.x-camera.position.x;
		if(dPosX > 7) {
			camera.position.x = targetPos.x - 7;
		} else if(dPosX < -7) {
			camera.position.x = targetPos.x + 7;
		}

		auto clamp = [](float v, float min, float max) {
			if(v<min) return min;
			if(v>max) return max;
			return v;
		};

		auto mapSize = map->getMapSize();
		auto tileSize = map->getTileSize();
		float minCamX	= 15.0f; // Hmm... Riipuu ruudun leveydestä... Nyt toimii tuolla ihan ok.
		float maxCamX	= mapSize.x - minCamX;
		assert(minCamX <= maxCamX);
		float minCamY	= 7.5f;
		float maxCamY	= mapSize.y - minCamY;
		assert(minCamY <= maxCamY);
		camera.position.x = clamp(camera.position.x, minCamX, maxCamX);
		camera.position.y = clamp(camera.position.y, minCamY, maxCamY);
		hungerland::util::INFO("Camera: pos=<"+std::to_string(camera.position.x)+","+std::to_string(camera.position.y)+">");
		return camera;
	};
} // End - namespace platformer::camera

}


#include <hungerland/map.h>

namespace platformer {
///
/// \ingroup platformer::env
///
namespace  env {
	///
	/// \brief loadScene
	/// \param ctx
	/// \param world
	/// \param index
	/// \param cfg
	///
	template<typename World, typename Ctx, typename Config>
	void loadScene(Ctx* ctx, World& world, size_t index, const Config& cfg) {
		using namespace hungerland;
		// Create map layers by map and tileset.
		world.tileMap = hungerland::map::load<hungerland::map::Map>(ctx, cfg.mapFiles[index], false);

		// Load object textures
		for(const auto& filename : cfg.characterTextureFiles) {
			auto texture = ctx->loadTexture(filename);
			if(texture== 0) {
				util::ERR("Failed to load object texture file: \"" + filename + "\"!");
			}
			util::INFO("Loaded object texture: " + filename);
			world.characterTextures.push_back(texture);
		}

		// Load item textures
		for(const auto& filename : cfg.itemTextureFiles) {
			auto texture = ctx->loadTexture(filename);
			if(texture== 0) {
				util::ERR("Failed to load item textures: \"" + filename + "\"!");
			}
			util::INFO("Loaded item texture: " + filename);
			world.itemTextures.push_back(texture);
		}

		// Get map x and y sizes
		auto mapSize = world.tileMap->getMapSize();
		// and adjust camera and to center y and left of map.
	#if 1
		world.players.push_back({math::vec3(5, mapSize.y/2, 0)});
		world.observer.position = math::vec3(0, mapSize.y/2, 0);
	#else
		world.player.position = math::vec3(0, 0, 0);
		world.camera.position = math::vec3(0, 0, 0);
	#endif
	}


	///
	/// \brief reset
	/// \param ctx
	/// \param name
	/// \param cfg
	///
	template<typename World, typename Ctx, typename Config>
	auto reset(Ctx* ctx, const std::string& name, const Config& cfg) {
		auto world = World();
		world.sceneName = name;
		loadScene(ctx, world, 0, cfg);
		return world;
	};

	///
	/// \brief update
	/// \param ctx
	/// \param world
	/// \param input
	/// \param dt
	/// \return
	///
	template<typename World, typename Ctx, typename Input>
	const auto& update(Ctx* ctx, World& world, Input input, float dt) {
		typedef decltype(world.players) Players;
#if defined(_WIN32)
		system("cls");
#else
		system("clear");
#endif
		hungerland::util::INFO("Platformer Frame: " + std::to_string(world.frameNum));
		for(auto& player : world.players){
			player = agent::update<hungerland::map::Map::MapCollision>(player, *world.tileMap, input, dt);
		}
		world.observer = camera::update(world.observer, world.tileMap, world.players[0].position, dt);
		/*printf("Player=<%2.2f, %2.2f> Camera=<%2.2f, %2.2f> Grounded:%d, Topped:%d, Walled:%d \n",
			   world.player.position.x, world.player.position.y, world.camera.position.x, world.camera.position.y,
			   world.player.grounded, world.player.topped, world.player.hitWall);*/
		++world.frameNum;
		return world;
	};
} // End - namespace platformer::env

} // End - namespace platformer


///
/// \ingroup view
///
namespace view {
	auto to_vec(glm::mat4 mat){
		std::vector<float> v;
		for(auto i=0; i<4; ++i) {
			for(auto j=0; j<4; ++j) {
				v.push_back(mat[i][j]);
			}
		}
		return v;
	}

	///
	/// \brief render
	/// \param screen
	/// \param state
	///
	template<typename Screen, typename World>
	void render(Screen& screen, const World& state) {
		static const auto MAP_OFFSET = glm::vec3(0.5, 0.5, 0);
		using namespace platformer;

		// Aseta origo ruudun vasempaan alareunaan:
		const auto SIZE_X = float(SCREEN_SIZE_X/2);
		const auto SIZE_Y = float(SCREEN_SIZE_Y/2);
		screen.setScreen(-SIZE_X, SIZE_X , SIZE_Y, -SIZE_Y);
		screen.clear(0, 0, 0, 0);
		auto projection = glm::ortho(-SIZE_X, SIZE_X , SIZE_Y, -SIZE_Y);


		// Offset of half tiles to look at centers of tiles.
		auto renderMapLayers = [](const hungerland::map::Map& mapLayers, glm::mat4 matProj, const hungerland::size2d_t& sizeInPixels, glm::vec3 cameraPosition) {
			// Flip camera y and offset
			cameraPosition.y =  mapLayers.getMapSize().y-cameraPosition.y-1;
			// And offset
			const auto scale = glm::vec3{mapLayers.getTileSize().x, mapLayers.getTileSize().y, 1};
			const auto mapScreenPos = scale * (cameraPosition + MAP_OFFSET);
			auto mat = glm::translate(glm::mat4(1), mapScreenPos);
			matProj = glm::translate(matProj, 0.5f * scale);
			auto cameraDelta = cameraPosition * scale;
			hungerland::map::draw(mapLayers, to_vec(matProj*glm::inverse(mat)), cameraDelta);
			return matProj;
		};

		auto renderSprite = [](Screen& screen, const glm::mat4& matProj, const hungerland::size2d_t& sizeInPixels, const glm::vec3& cameraPosition, glm::vec3 position, const hungerland::texture::Texture& texture) {
			// Flip x and y
			position.x = position.x - cameraPosition.x;
			position.y = cameraPosition.y - position.y;
			// And offset
			position += MAP_OFFSET;
			position *= glm::vec3(sizeInPixels.x, sizeInPixels.y, 1);

			auto mat = glm::mat4(1);
			mat = glm::translate(mat, position);
			mat = glm::scale(mat, glm::vec3(sizeInPixels.x,sizeInPixels.y, 1));
			screen.drawSprite(mat, texture);
		};

		// Render Tilemap
		projection = renderMapLayers(*state.tileMap, projection, state.tileMap->getTileSize(),
					state.observer.position);
		// Render Player
		renderSprite(screen, projection, state.tileMap->getTileSize(),
					state.observer.position,
					state.players[0].position, *state.characterTextures[0]);
	}


	struct Config {
		std::vector<std::string> backgroundFiles;
		std::vector<std::string> mapFiles;
		std::vector<std::string> characterTextureFiles;
		std::vector<std::string> itemTextureFiles;
	};
} // End - namespace view

///
/// \ingroup model
///
namespace model {
	///
	/// \brief The World class
	///
	template<typename GameObject>
	struct World {
		std::string sceneName;
		std::vector<std::shared_ptr<hungerland::texture::Texture> > characterTextures;
		std::vector<std::shared_ptr<hungerland::texture::Texture> > itemTextures;
		std::shared_ptr<hungerland::map::Map> tileMap;
		GameObject observer;
		std::vector<GameObject> players;
		std::vector<GameObject> nonPlayers;
		size_t frameNum = 0;
	};

	///
	/// \brief The GameObject class
	///
	struct GameObject {
		glm::vec3 position;
		glm::vec3 velocity = glm::vec3(0);
	};

	///
	/// \brief The Character class
	///
	struct Character : public GameObject {
		bool isGrounded = false;
		bool isTopped = false;
		bool canMoveL = false;
		bool canMoveR = false;
		bool canJump = false;
		float wallJump = 0.0f;
	};

} // End - namespace model

/*
#include <complex>
#include <vector>
// esitellään FFT ja käänteismuunnoksen prototyypit
void fft(std::vector<std::complex<float>> &data);
void ifft(std::vector<std::complex<float>> &data);
// toteutetaan normaalisti O(N^2) kompleksinen (yksiulotteinen) konvoluutio
// FFT:llä (O(N*log(N))
size_t template_matching_with_fft_based_convolution(
 std::vector<std::complex<float>> &input,
 std::vector<std::complex<float>> const &pattern_fft) {
	 // pattern on jo aiemmin muunnettu fft-tasoon
	 fft(input);
	 // konvoluutio aikatasossa on elementittäinen kertolasku taajuustasossa
	 for (auto i = 0u; i < input.size(); i++) {
		 input[i] *= pattern_fft[i];  // <- kompleksinen kertolasku
	 }
	 // palautus taajuustasosta aikatasoon
	 ifft(input);
	 // etsitään maksimimagnituudin sisältämä argumentti, jolloin saadaan
	 // selville, kuinka monta elementtiä dataa on rotatoitava syklisesti
	 // jotta se mätsää eniten patternin kanssa
	 auto result = std::max_element(input.begin(), input.end(),
		 [](auto const &a, auto const &b) -> bool {
			 return std::abs(a) < std::abs(b);
		 });
	 return std::distance(input.begin(), result);
}

Fast inverse square root:
float Q_rsqrt( float number )
{
	long i;
	float x2, y;
	const float threehalfs = 1.5F;

	x2 = number * 0.5F;
	y  = number;
	i  = *(long *) &y;
	i  = 0x5f3759df - ( i >> 1 );
	y  = *(float *) &i;
	y  = y * ( threehalfs - ( x2 * y * y ) );
	return y;
}
*/

/*
EXAMPLE CPP FILE
#include <examples/platformer.h>

namespace my_game_app {
	// Resource files:
	static const std::vector<std::string> MAP_FILES = {
		"assets/scene1.tmx",
	};

	static const std::vector<std::string> BACKGROUND_FILES = {
	};

	static const std::vector<std::string> CHARACTER_TEXTURE_FILES = {
		"assets/images/Player.png",
	};

	static const std::vector<std::string> ITEM_TEXTURE_FILES = {
	};

	// Application config:
	static const size_t	WINDOW_SIZE_X = platformer::SCREEN_SIZE_X/2;
	static const size_t	WINDOW_SIZE_Y = platformer::SCREEN_SIZE_Y/2;
	static const std::string GAME_NAME = "My game";
	static const std::string GAME_VERSION = "v0.0.1";
	static const std::string GAME_LONG_NAME = GAME_NAME + " " + GAME_VERSION;

	static const view::Config CONFIG = {
		BACKGROUND_FILES,
		MAP_FILES,
		CHARACTER_TEXTURE_FILES,
		ITEM_TEXTURE_FILES
	};
}

// Main function
int main() {
	using namespace my_game_app;
	using namespace platformer;

	typedef model::World<model::Character> Model;
	typedef window::Window View;

	// Create application window and run it.
	View window({WINDOW_SIZE_X, WINDOW_SIZE_Y}, "");
	auto state = env::reset<Model>(&window, GAME_LONG_NAME, CONFIG);
	float totalTime = 0;
	int lastFrame = -1;
	return window.run([&](View& window, float dt) {
		totalTime += dt;
		auto& input = window.getInput();
		if(int(totalTime) > lastFrame){
			window.setTitle(GAME_LONG_NAME + "    FPS="+std::to_string(1.0f/dt).substr(0,5));
			lastFrame = int(totalTime);
		}
		if(input.getKeyPressed(window::KEY_F5)) {
			state = env::reset<Model>(&window, GAME_LONG_NAME, CONFIG);
		}
		// Configure input buttons:
		agent::Input playerInput;
		playerInput.dy			= input.getKeyState(window::KEY_UP)				- input.getKeyState(window::KEY_DOWN);
		playerInput.dx			= input.getKeyState(window::KEY_RIGHT)			- input.getKeyState(window::KEY_LEFT);
		playerInput.accelerate	= input.getKeyState(window::KEY_LEFT_SHIFT)		+ input.getKeyState(window::KEY_RIGHT_SHIFT);
		playerInput.wantJump	= input.getKeyPressed(window::KEY_LEFT_CONTROL)	+ input.getKeyPressed(window::KEY_RIGHT_CONTROL);
		state = env::update<Model>(&window, state, playerInput, dt);
		return true;
	}, [&state,&window](screen::Screen& screen) {
		view::render(screen, state);
	});
}
*/
