#pragma once
#include "game.h"
#include <assert.h>
#include <cmath>

namespace find_goal_game {



	// Onko peli loppu?
	inline bool isEnd(const auto& gameState) {
		if(gameState.agents.size() == 0) {
			return true; // Not started
		}
		bool ended = false;
		bool isAllDead = true;
		// Tarkista, onko joku agentti maalissa numero 0 (toleranssi 0.01f):
		for(size_t agentId=0; agentId<gameState.agents.size(); ++agentId) {
			const auto& agent = gameState.agents.objects[agentId];
			if(agent.alive || !isAllDead) {
				isAllDead = false;
			}
			if(agent.alive) {
				auto& agents = gameState.agents;
				auto& objects = gameState.objects;
				const auto& agentState = agents.states[agentId];
				const auto& objectState = objects.states[0];
				auto dx = agentState[0] - objectState[0];
				auto dy = agentState[1] - objectState[1];
				if(std::sqrt(dx*dx + dy*dy) < 0.5f) {
					printf("Agent: %d goaled!\n", int(agentId));
					ended = true;
				}
			}
		}
		return ended || isAllDead;
	}

	template<typename GameType>
	inline auto envUpdate(GameType& gameState, float deltaTime) {
		std::vector<typename GameType::Event> events;
		// TODO: Integroi peliä deltaTimen verran eteenpäin ja kerää ulkoiset eventit:
		return events;
	}

	// Game update:
	template<typename GameType>
	inline auto update(GameType& gameState, float deltaTime) {
		assert(deltaTime >= 0.0f);

		// Päivitä kaikki agentit:
		game::forEachAliveAgent(gameState.agents, [&gameState,deltaTime](auto& obj, const auto& state, const auto& history) {
			if(obj.update) {
				obj.update(gameState, obj.id, deltaTime);
			}
		});

		// Päivitä kaikki objektit:
		game::forEachAliveObject(gameState.objects,[&gameState,deltaTime](auto& obj, const auto& state) {
			if(obj.update) {
				obj.update(gameState, obj.id, deltaTime);
			}
		});

		// Tee kaikkien agenttien actionit: (HUOM! tämä on eri kuin update. Politiikkafunktiolla eri parametrit)
		game::forEachAliveAgent(gameState.agents, [&gameState, deltaTime](auto& obj, const auto& state, const auto& history) {
			// Kysy tehtävä action id politiikkafunktiolta:
			int actionId = obj.policy(gameState.agents.objects[obj.id], gameState);
			// Kutsu pelin action funktiota:
			if(actionId >= 0 && actionId < gameState.actions.size() ) {
				// Tee action kutsumalla id:n osoittamaa action funktiota:
				auto ev = gameState.actions[actionId](gameState, obj.id, 1.0f*deltaTime);
				// Jos ei tyhjä eventti, kutsu agentin event funktiota (sisäinen eventti):
				if(!ev.empty() && obj.event) {
					obj.event(gameState.agents.objects[obj.id], ev);
				}
			}
			//printf("State: %f,%f\n", state[0], state[1]);

			// Tee uusi historiasolmu agentin actionin ja uuden tilan mukaan:
			auto prevNode		= gameState.agents.historys[obj.id];
			auto newNode		= std::make_shared<typename GameType::NodeType>();
			newNode->prevNode	= prevNode;
			newNode->action		= actionId;
			newNode->state		= gameState.agents.states[obj.id];
			gameState.agents.historys[obj.id] = newNode; // Korvaa vanha solmu uudella.
		//	printf("Agent %d: State:%s, Action:%d\n", int(obj.id), game::to_str(newNode->state).c_str(), newNode->action);
		});

		// Lähetä kaikki eventit kaikille agenteille:
		auto events = envUpdate(gameState, deltaTime);
		for(const auto& ev : events) {
			if(!ev.empty()) {
				for(auto& obj : gameState.agents.objects) {
					if(false == obj.alive) {
						continue;
					}
					if(obj.event) {
						obj.event(obj, ev);
					}
				}
			}
		}
		++gameState.n;
		return events;
	}
}

