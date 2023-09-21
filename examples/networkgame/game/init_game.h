#pragma once
#include "game.h"

namespace game {

	template<typename GameType>
	bool initDefaultGame(GameType& gameState) {
		srand(time(0));
		// Game Behavioural functors (goal + server player):
		gameState.ClassGoal = typename GameType::ObjectClassType {
			1 //visualId
		};
		gameState.ClassAgent = typename GameType::ObjectClassType {
			3, // visualId
		};

		// Set map:
		gameState.map = {
			{2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
			{2, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 2},
			{2, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 2},
			{2, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 2},
			{2, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 2},
			{2, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 2},
			{2, 0, 0, 0, 2, 0, 0, 2, 0, 0, 0, 0, 2},
			{2, 0, 0, 0, 2, 0, 0, 2, 2, 2, 0, 0, 2},
			{2, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 2},
			{2, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 2},
			{2, 0, 0, 0, 2, 0, 0, 2, 2, 2, 0, 0, 2},
			{2, 0, 0, 0, 2, 0, 0, 2, 0, 0, 0, 0, 2},
			{2, 0, 0, 0, 2, 0, 0, 2, 0, 0, 0, 0, 2},
			{2, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 2},
			{2, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 2},
			{2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
		};

		gameState.cameraX = 0.5f * float(gameState.map[0].size());
		gameState.cameraY = 0.5f * float(gameState.map.size());

		// By default, the agent has a policy func, which gets agent.input
		// vector of float values for input axix & button states comming from player.
		gameState.ClassAgent.policy = [](auto& agent, const auto& gameState) {
			if(agent.inputs.size() == 0) {
				return -1;
			}
			// Convert input key states to action id:s:
			return game::inputToActionId(agent.inputs);
		};

		// Spawn goal:
		typename GameType::ObjectStateType goalPos({gameState.map[0].size()-2.0f, gameState.map.size()-2.0f});
		game::spawnObject(gameState.objects, gameState.ClassGoal, goalPos);
		return true;
	}
}
