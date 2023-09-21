#pragma once
#include <netgame/netcode.h>
#include <game/game.h>
#include <netgame/serialize.h>

///
/// Tässäpä hieman applikaation protokollakoodia, josta voi ottaa mallia.
///  app_msg-nimiavaruus:
///		Sisältää viestien lähettämisfunktioita, jotka puolestaan käyttävät
///		netcode-nimiavaruuden funktioita viestien lähettämiseen eri
///		Message id:illä. Kukin viesti laitetaan numeron osoittamaan
///		"enet-kanavaan" ja vastaanottaja voi sillä perusteella tietää
///		viestin id:n.
///
/// shard- ja client-nimiavaruudet:
///		RX-loppuiset funktiot ovat clientin tai serverin viestinkäsittelijöitä,
///		jonne tulevat kaikki enet-yhteyden kautta tulevat viestit. RX-funktio
///		tyypillisesti sitältää valintarakenteen, jossa eri Message id:llä tulevat
///		viestit käsitellään kukin sen mukaan, miten pitääkin. Esin load-viesti
///		lataa pelin koko tilan.
///
///		shard::STM funktio on peli shardin pää update funktio, joka hoitaa pelisession
///		tilan hallinnan. Shardi voi olla jossakin näistä tiloista:
/// 	WAITING_PLAYERS, SPAWNING, RUNNING tai ENDED. (määritetty model.h tiedostossa).
///		oikeasti, myös clientillä olisi olla hyvä jonkinlainen tilakone, joka pitäisi huolen
///		siitä, että ei lähetellä viestejä väärään aikaan, mutta yksinkertaisuuden vuoksi
///		sellaista nyt ei ole tehty.
///
/// Application message sending functions (in "order of appearance"):
///		Client -> Host: sendJoinRq(auto& conn, const std::string& playerName)
///		Host -> Client: sendJoinRs(auto& conn, int agentId, const std::string& playerName)
///		Host -> Client: sendLoadRq(auto& conn, const auto& gameState)
///		Client -> Host: sendLoadRs(auto& conn, const auto& gameState)
///		Host -> Client: sendStartInd(auto& conn)
///		Host -> Client: sendGameOverInd(auto& conn)
///		Host -> Client: sendEvents(auto& conn, const auto& eventsVec)
///		Host -> Client: sendSync(auto& conn, const auto& gameState)
///		Client -> Host: sendPlayerInput(auto& conn, auto& inputVec)
///		Client -> Host: sendPlayerAction(auto& conn, int actionId)
///
/// Game Shard server "send" functions (Host->Client). These functions are
/// wrappers to some message sending functions to make shard_app::run more clean:
///		bool spawnPlayer(auto& server, int peerId)
///		void destroyPlayer(auto& server, int peerId)
///
/// Game Shard protocol functions:
///		bool serverRX(auto& server, const auto& msg)
///
/// Client protocol functions:
///		bool clientRX(auto& client, const auto& msg)
///
///	Terveisin, MikkoR
///
namespace app_msg {
	enum MsgIDs {
		JOIN, LOAD, STATE, EVENTS, SYNC, PLAYER_ACTION, PLAYER_INPUT, NUM_MESSAGES
	};

	inline void sendJoinRq(auto& conn, const std::string& playerName) {
		printf("CLIENT_TX: msg::JOIN: playerName=%s\n", playerName.c_str());
		netcode::sendReliable(conn, JOIN, {playerName});
	}

	inline void sendJoinRs(auto& conn, int agentId, const std::string& playerName) {
		printf("SERVER_TX: msg::JOIN: agentId=%d, playerName=%s\n", agentId, playerName.c_str());
		netcode::sendReliableTo(conn, agentId, JOIN, {std::to_string(agentId), playerName});
	}

	inline void sendLoadRq(auto& conn, const auto& gameState) {
		// Map:
		printf("SERVER_TX: msg::LOAD: LOAD_GAME\n");
		std::vector<std::string> msg = {"LOAD_GAME"};
		serialize::getLoadData(msg, gameState);
		game::printMap(gameState);
		game::printAgents(gameState);
		netcode::sendReliable(conn, LOAD, msg);
	}

	inline void sendLoadRs(auto& conn, const auto& gameState) {
		printf("CLIENT_TX: msg::LOAD: LOAD_DONE\n");
		netcode::sendReliable(conn, LOAD, {"LOAD_DONE"});
	}

	inline void sendStartInd(auto& conn) {
		printf("SERVER_TX: msg::STATE: START_GAME\n");
		netcode::sendReliable(conn, STATE, {"START_GAME"});
	}

	inline void sendGameOverInd(auto& conn) {
		printf("SERVER_TX: msg::STATE: GAME_OVER\n");
		netcode::sendReliable(conn, STATE, {"GAME_OVER"});
	}

	inline void sendEvents(auto& conn, const auto& eventsVec) {
		printf("SERVER_TX: msg::EVENTS: count=%d\n", int(eventsVec.size()));
		netcode::sendReliable(conn, EVENTS, eventsVec);
	}

	inline void sendSync(auto& conn, const auto& gameState) {
		//printf("SERVER_TX: msg::SYNC:\n");
		std::vector<std::string> msg;
		serialize::getObjectGroup(msg, gameState.agents);
		netcode::sendReliable(conn, SYNC, msg);
	}

	inline void sendPlayerInput(auto& conn, const std::vector<float>& inputVec) {
		//printf("CLIENT_TX: msg::PLAYER_INPUT:");
		std::vector<std::string> msg;
		for(auto val : inputVec){
		//	printf("%f ", val);
			msg.push_back(std::to_string(val));
		}
		//printf("\n");
		netcode::sendReliable(conn, PLAYER_INPUT, msg);
	}

	inline void sendPlayerAction(auto& conn, int actionId) {
		//printf("CLIENT_TX: msg::PLAYER_ACTION: actionId=%d\n", actionId);
		std::vector<std::string> msg = {std::to_string(actionId)};
		netcode::sendReliable(conn, PLAYER_ACTION, msg);
	}
}

///
/// Shard server (runs the game).
///
namespace shard {
	/// \brief Deletes agent from game

	template<typename ActorType>
	inline bool spawnPlayer(auto& server, int peerId) {
		printf("SHARD: New player: %d\n", peerId);
		game::spawnAgent(server.game);
		return true;
	}

	/// \brief Deletes agent from game
	inline void destroyPlayer(auto& server, int peerId) {
		printf("SHARD: Player destroy: %d\n", peerId);
		game::destroyAgent(server.game, peerId);
	}

	/// \brief Server message handler.
	template<typename AppState>
	inline bool serverRX(auto& server, const auto& msg) {
		int agentId =  msg.peerId;
		if(msg.msgId == app_msg::LOAD && msg.rxData.size() == 1 && msg.rxData[0] == "LOAD_DONE") {
			printf("SERVER_RX: msg::LOAD_DONE: %d\n", agentId);
			server.game.agents.objects[agentId].isReady = true;
		} else if(msg.msgId == app_msg::JOIN && msg.rxData.size() == 1) {
			printf("SERVER_RX: msg::JOIN: %s\n", msg.rxData[0].c_str());
			if(server.state != AppState::WAITING_PLAYERS) {
				return false; // Bad behaving clent..
			}
			game::joinPlayer(server.game, agentId, msg.rxData[0]);
			app_msg::sendJoinRs(server.conn, agentId, server.game.agents.objects[agentId].playerName);
		} else if(msg.msgId == app_msg::PLAYER_INPUT && msg.rxData.size() > 0) {
			std::vector<float> values;
			for(const auto& str : msg.rxData){
				values.push_back(std::stof(str.c_str()));
			}
			//printf("SERVER_RX: msg::PLAYER_INPUT: %s\n", game::to_str(values).c_str());
			game::setPlayerInput(server.game, agentId, values);
		} else {
			printf("SERVER_RX: Unhandled Message: peer=%d, msgId=%d: ", msg.peerId, msg.msgId);
			for(const auto&  str : msg.rxData) {
				printf("%s ", str.c_str());
			}
			printf("\n");
			return false; // Bad behaving clent..
		}
		return true;
	}

	///
	/// \brief STM Setver state machine.
	/// \param server
	/// \param n
	/// \param stepGame
	/// \return
	///
	template<typename AppState>
	inline bool STM(auto& server, int& n, auto stepGame) {
		if(server.state != AppState::WAITING_PLAYERS && server.conn.peers.size() == 0) {
			/// GAME_ENDED:
			printf("\nSHARD_STM: AppState::ENDED: All players disconnected!\n");
			server.state = AppState::ENDED;
		} else if(server.state == AppState::WAITING_PLAYERS) {
			/// GAME_WAITING_PLAYERS:
			if(server.conn.peers.size() == server.game.NUM_PLAYERS) {
				// Send Load game:
				printf("SHARD: Game LOAD!\n");
				app_msg::sendLoadRq(server.conn, server.game);
				printf("\nSHARD_STM: AppState::SPAWNING:\n");
				server.state = AppState::SPAWNING;
			} else {
				// Not enought peers yet...
			}
		} else if(server.state == AppState::SPAWNING) {
			/// GAME_SPAWNING:
			if(game::isAllPlayersReady(server.game, server.game.NUM_PLAYERS)) {
				// Send Start game:
				printf("SHARD: Game START:\n");
				game::printMap(server.game);
				printf("\n  Agents:\n");
				for(size_t agentId=0; agentId<server.game.agents.size(); ++agentId) {
					printf("    %d: %s\n", int(agentId), game::to_str(server.game.agents.states[agentId]).c_str());
				}
				app_msg::sendStartInd(server.conn);
				printf("\nSHARD_STM: AppState::RUNNING:\n");
				server.state = AppState::RUNNING;
			}
		} else if(server.state == AppState::RUNNING) {
			/// GAME_RUNNING:
			if(stepGame()) {
				// Send Game over:
				printf("SHARD: Game OVER!\n");
				app_msg::sendGameOverInd(server.conn);
				printf("\nSHARD_STM: AppState::ENDED:\n");
				server.state = AppState::ENDED;
			}
			++n;
		}
		return server.state == AppState::ENDED;
	}
}

///
/// Client (client for shard server).
///
namespace client {
	/// \brief Client message handler.
	template<typename AppState>
	inline bool clientRX(auto& client, const auto& msg) {
		// Switch message id and act accordingly:
		if(msg.msgId == app_msg::LOAD && msg.rxData.size() > 0 && msg.rxData[0] == "LOAD_GAME") {
			printf("CLIENT_RX: msg::LOAD_GAME\n");
			game::reset(client.game);
			serialize::setLoadData(client.game, msg,[&](auto& gameState, const auto& agentStateStr)  {
				game::spawnAgentToState(gameState, game::to_vecf(agentStateStr));
			},[](auto& gameState, const auto& objectStateStr) {
				game::spawnObject(gameState.objects, gameState.ClassGoal, game::to_vecf(objectStateStr));
			});
			app_msg::sendLoadRs(client.conn, client.game);
		} else if(msg.msgId == app_msg::JOIN && msg.rxData.size() == 2) {
			printf("CLIENT_RX: msg::JOIN: agentId=%s playerName=%s\n", msg.rxData[0].c_str(), msg.rxData[1].c_str());
			client.state = AppState::SPAWNING;
		} else if(msg.msgId == app_msg::STATE && msg.rxData.size() == 1 && msg.rxData[0] == "START_GAME") {
			printf("CLIENT_RX: msg::STATE: START_GAME\n");
			game::setReady(client.game);
			game::printAgents(client.game);
			client.state = AppState::RUNNING;
		} else if(msg.msgId == app_msg::STATE && msg.rxData.size() == 1 && msg.rxData[0] == "GAME_OVER") {
			printf("CLIENT_RX: msg::STATE: GAME_OVER\n");
			client.state = AppState::ENDED;
		} else if(msg.msgId == app_msg::SYNC && msg.rxData.size() > 0) {
			//printf("CLIENT_RX: msg::SYNC:\n");
			serialize::setObjectGroup(client.game.agents, msg);
		} else {
			printf("CLIENT_RX: Unhandled Message: peerId=%d, msgId=%d: ", msg.peerId, msg.msgId);
			for(const auto&  str : msg.rxData) {
				printf("%s ", str.c_str());
			}
			printf("\n");
			return false; // Bad behaving clent..
		}
		return true;
	}
}




