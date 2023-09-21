#pragma once
#include <memory>
#include <vector>

namespace game {

	///
	/// \brief The PeriodicTimer class for timed events according to delta time.
	///
	class PeriodicTimer {
	public:
		PeriodicTimer(float timePeriod)
			: PERIOD(timePeriod),m_timer(0) {}

		template<typename CallbackFunc>
		void update(float deltaTime, CallbackFunc cb) {
			m_timer -= deltaTime;
			if(m_timer < 0.0f){
				m_timer = PERIOD;
				cb(PERIOD);
			}
		}

	private:
		const float		PERIOD;
		float			m_timer		= 0.0f;
	};

	inline std::string to_str(const auto& agentStates) {
		std::string res;
		for(auto value : agentStates) {
			res += std::to_string(value) + " ";
		}
		if(res.size()>0){
			res.pop_back();
		}
		return res;
	}

	inline std::vector<float> to_vecf(std::string data) {
		std::vector<float> res;
		auto i = data.find_first_of(' ');
		while(i != std::string::npos) {
			res.push_back(std::atof(data.substr(0,i).c_str()));
			data = data.substr(i+1, data.size());
			i = data.find_first_of(' ');
		}
		res.push_back(std::atof(data.c_str()));
		return res;
	}

	inline std::vector<int> to_veci(std::string data) {
		std::vector<int> res;
		auto i = data.find_first_of(' ');
		while(i != std::string::npos) {
			res.push_back(std::atoi(data.substr(0,i).c_str()));
			data = data.substr(i+1, data.size());
			i = data.find_first_of(' ');
		}
		res.push_back(std::atoi(data.c_str()));
		return res;
	}

	inline void traverseList(auto node, auto visitFunc) {
		// Rekursion loppuehto
		if (node == 0) { return; }
		// Vieraile loput edelliset solmut rekursiivisesti.
		traverseList(node->prevNode, visitFunc);
		// Vieraile t�m� solmu
		visitFunc(node);
	}

	template<typename GameType>
	inline GameType createGame(int numberOfPlayers) {
		auto game = GameType{numberOfPlayers};
		return game;
	}

	auto getMapTile(const auto& map, const auto& state) {
		int stateX = int(state[0]+0.5f);
		int stateY = int(state[1]+0.5f);
		if(stateX < 0 || stateY < 0 || stateY >= map.size() || stateX >= map[stateY].size()) {
			return -1;
		}
		return map[stateY][stateX];
	}

	template<typename GameType>
	inline auto spawnAgentToState(GameType& game, const auto& initialState) {
		// Lisää alkutila agenttien historiaan:
		auto& group = game.agents;
		auto node = std::make_shared<typename GameType::NodeType>();
		node->state = initialState;
		group.historys.push_back(node);
		// Tee agentti peliin:
		group.states.push_back(initialState);
		auto agentId = group.objects.size();
		group.objects.push_back(game.ClassAgent);
		group.objects.back().isReady	= true;
		group.objects.back().alive		= true;
		group.objects.back().id			= agentId;
		return agentId;
	}

	template<typename GameType>
	inline auto spawnAgent(GameType& game) {
		// Randomize initial state:
		typename GameType::ObjectStateType initialState = {0,0};
		auto tile = game::getMapTile(game.map, initialState);
		float mapSizeX = game.map[0].size();
		float mapSizeY = game.map.size();
		while(tile != 0) {
			initialState = {
				mapSizeX*float(rand())/float(RAND_MAX),
				mapSizeY*float(rand())/float(RAND_MAX)
			};
			tile = game::getMapTile(game.map, initialState);
		}
		return spawnAgentToState(game, initialState);
	}

	inline auto spawnObject(auto& group, const auto& objectClass, const auto& initialState) {
		// Tee objecti peliin:
		//auto& group = gameState.objects;
		group.states.push_back(initialState);
		auto objectId = group.objects.size();
		group.objects.push_back(objectClass);
		group.objects.back().isReady	= true;
		group.objects.back().alive		= true;
		group.objects.back().id			= objectId;
		return objectId;
	}

	inline void forEachAgent(const auto& agents, auto func) {
		//const auto& agents = gameState.agents;
		for(size_t agentId=0; agentId<agents.size(); ++agentId) {
			func(agents.objects[agentId], agents.states[agentId], agents.historys[agentId]);
		}
	}

	inline void forEachObject(const auto& group, auto func) {
		//const auto& group = gameState.objects;
		for(size_t objectId=0; objectId<group.size(); ++objectId) {
			func(group.objects[objectId], group.states[objectId]);
		}
	}

	inline void forEachAliveAgent(const auto& group, auto func) {
		forEachAgent(group,[func](const auto& agent, const auto& state, const auto& history) {
			if(agent.alive && state.size() > 0){
				func(agent, state, history);
			}
		});
	}

	inline void forEachAliveObject(const auto& group, auto func) {
		forEachObject(group,[func](const auto& object, const auto& state) {
			if(object.alive && state.size() > 0){
				func(object, state);
			}
		});
	}

	/// \brief Joind player to game (alive==true, isReady==false).
	inline auto joinPlayer(auto& gameState, int agentId, const std::string& playerName) {
		gameState.agents.objects[agentId].isReady	= false;
		gameState.agents.objects[agentId].alive		= true;
		gameState.agents.objects[agentId].playerName= playerName;
	}

	/// \brief setPlayerInput sets player input values for agentId.
	inline void setPlayerInput(auto& gameState, int agentId, const std::vector<float>& inputValues) {
		gameState.agents.objects[agentId].inputs = inputValues;
	}

	/// \brief Sets all alive agents to be ready.
	inline auto setReady(auto& gameState) {
		for(auto& agent : gameState.agents.objects) {
			if(agent.playerName.length() > 0 && agent.alive) {
				agent.isReady = true;
			}
		}
	}

	/// \brief Returns true if all joined players are ready
	inline bool isAllPlayersReady(auto& gameState, int numPlayers) {
		if(gameState.agents.size() != numPlayers) {
			return false;
		}
		bool allReady = true;
		for(const auto& agent : gameState.agents.objects) {
			if(agent.playerName.length() > 0 && agent.alive && !agent.isReady) {
				allReady = false;
			}
		}
		return allReady;
	}

	inline void destroyAgent(auto& gameState, int agentId) {
		gameState.agents.objects[agentId].alive	= false;
		gameState.agents.objects[agentId].isReady	= false;
	}

	inline void destroyObject(auto& gameState, int objectId) {
		gameState.objectStates.erase(gameState.objectStates.begin()+objectId);
		gameState.objects.erase(gameState.objects.begin()+objectId);
	}

	inline void reset(auto& gameState) {
		gameState.objects.clear();
		gameState.agents.clear();
		gameState.map.clear();
	}

	inline bool isReadyToStart(const auto& gameState) {
		if(gameState.agents.size() == 0) {
			return false; // Not started
		}
		bool allReady = true;
		for(const auto& agent : gameState.agents) {
			if(!agent.isReady && allReady) {
				allReady = false;
			}
		}
		return allReady;
	}

	inline void printMap(const auto& gameState) {
		printf("Map %dx%d:\n", int(gameState.map[0].size()), int(gameState.map.size()));
		for(size_t y=0; y<gameState.map.size(); ++y){
			printf("  %s\n", game::to_str(gameState.map[y]).c_str());
		}
	}

	inline void printAgent(const auto& gameState, auto agentId) {
		printf("Player %d:\n", int(agentId));
		const auto& agent = gameState.agents.objects[agentId];
		const auto& state = gameState.agents.states[agentId];
		printf("  Name:  %s\n", agent.playerName.c_str());
		printf("  Alive: %d\n", int(agent.alive));
		printf("  Ready: %d\n", int(agent.isReady));
		printf("  State: %s\n", game::to_str(state).c_str());
		printf("  History:\n");
		game::traverseList(gameState.agents.historys[agentId], [](auto n) {
	//		printf("    Action: %d, State: %s\n", n->action, game::to_str(n->state).c_str());
		});
	}

	inline void printObject(const auto& gameState, auto objectId) {
		printf("Object %d:\n", int(objectId));
		const auto& object = gameState.objects.objects[objectId];
		const auto& state = gameState.objects.states[objectId];
		printf("  Alive: %d\n", int(object.alive));
		printf("  State: %s\n", game::to_str(state).c_str());
	}

	inline void printAgents(const auto& gameState) {
		for (size_t agentId = 0; agentId < gameState.agents.size(); ++agentId) {
			printAgent(gameState, agentId);
		}
	}

	inline void printGameResults(const auto& gameState) {
		printf("Game ended after %d steps\n", gameState.n);
		printAgents(gameState);
	}

	inline int inputToActionId(const auto& input) {
		if(input.size() == 0) {
			return -1;
		}
		float dx = input[0];
		float dy = input[1];
		if(std::abs(dx) < 0.01f && std::abs(dy) < 0.01f) {
			return -1;
		}

		if(std::abs(dx) > std::abs(dy) ) {
			return dx < 0.0f ?  1 : 0; // x
		}
		return dy < 0.0f ?  2 : 3;
	}



}

