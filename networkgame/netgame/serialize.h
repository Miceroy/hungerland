#pragma once
#include <memory>
#include <vector>
#include <game/game.h>

///
/// Tämä tiedosto sisältää serialisointifunktiot, joiden avulla voi
/// serialisoida ja deserialisoida viestejä.
///
/// Serialisointifunktiot ottavat tyypillisesti parametrinä tietorakenteen (struct, luokka)
/// ja palauttavat taulukollisen dataa (joka sitten voidaan laittaa viestin payload dataksi).
///
/// Deserialisointifunktiot ottavat tyypillisesti parametrinä taulukollisen
/// dataa (tulee viestissä payloadina) ja palauttavat/asettavat johonkin
/// tietorakenteeseen.
///
///


namespace serialize {
inline void getObjectGroup(auto& msg, const auto& objectGroup) {
	msg.push_back(std::to_string(objectGroup.size()));
	for(size_t i=0; i<objectGroup.size(); ++i){
		const auto& object = objectGroup.objects[i];
		const auto& state = objectGroup.states[i];
		if(object.alive && state.size()>0) {
			msg.push_back(game::to_str(state));
		} else {
			msg.push_back(" ");
		}
	}
}

inline void setObjectGroup(auto& objectGroup, const auto& msg) {
	// Agents
	size_t cursor = 0;
	int agentCount = std::atoi(msg.rxData[cursor++].c_str());
	for(size_t agentId=0; agentId<agentCount; ++agentId) {
		const auto& agentStateStr = msg.rxData[cursor++];
		if(agentStateStr.size() > 0 && agentStateStr != " ") {
			objectGroup.objects[agentId].alive = true;
			objectGroup.states[agentId] = game::to_vecf(agentStateStr);
		} else {
			objectGroup.objects[agentId].alive = false;
		}
	}
}

inline void getMap(auto& msg, const auto& gameState) {
	const auto& map = gameState.map;
	msg.push_back(std::to_string(map[0].size()));
	msg.push_back(std::to_string(map.size()));
	for(size_t y=0; y<map.size(); ++y){
		for(size_t x=0; x<map[y].size(); ++x){
			msg.push_back(std::to_string(map[y][x]));
		}
	}
}



inline void getLoadData(auto& msg, const auto& gameState) {
	// Map:
	getMap(msg, gameState);

	// Agents:
	getObjectGroup(msg, gameState.agents);

	// Objects:
	getObjectGroup(msg, gameState.objects);
}


inline int setMap(auto& gameState, const auto& msg, int cursor) {
	int mapSizeX = std::atoi(msg.rxData[cursor++].c_str());
	int mapSizeY = std::atoi(msg.rxData[cursor++].c_str());
	printf("CLIENT_RX: msg::LOAD: mapSizeX=%d mapSizeY=%d \n  ", mapSizeX, mapSizeY);
	for(size_t y=0; y<mapSizeY; ++y) {
		gameState.map.emplace_back();
		for(size_t x=0; x<mapSizeX; ++x) {
			gameState.map[y].push_back(std::atoi(msg.rxData[cursor++].c_str()));
		}
	}
	game::printMap(gameState);
	return cursor;
}

inline void setLoadData(auto& gameState, const auto& msg, auto spawnAgent, auto spawnObject) {
	// Map:
	size_t cursor = setMap(gameState, msg, 1);

	// Agents
	int agentCount = std::atoi(msg.rxData[cursor++].c_str());
	printf("Agents(%d):\n", agentCount);
	for(size_t agentId=0; agentId<agentCount; ++agentId) {
		const auto str = msg.rxData[cursor++];
		if(str.size()>0 && str != " ") {
			spawnAgent(gameState, str);
		}
		game::printAgent(gameState, agentId);
	}
	// Objects
	int objectCount = std::atoi(msg.rxData[cursor++].c_str());
	printf("Objects(%d):\n", objectCount);
	for(size_t objectId=0; objectId<objectCount; ++objectId) {
		const auto str = msg.rxData[cursor++];
		if(str.size()>0 && str != " ") {
			spawnObject(gameState, str);
		}
		game::printObject(gameState, objectId);
	}
}

}
