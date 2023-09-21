#pragma once
#include <view/sfml_application.h>
#include <netgame/app_protocol.h>
#include <game/model.h>
#include <assert.h>
#include <thread>
#include <stdexcept>
#include <game/find_goal_game.h>
#include <game/init_game.h>

namespace client_app {
typedef int AppData; // texture id.

/// \brief Steps server game according to deltaTime
inline void stepFindGoalGame(auto& server, float deltaTime) {
	auto events = find_goal_game::update(server.game, deltaTime);
	if(events.size() > 0) {
		app_msg::sendEvents(server.conn, events);
	}

}


///
/// \brief renderGame
/// \param window
/// \param game
/// \param textures
///
void renderGame(auto& window, const auto& game, const auto& textures) {
	// Aseta origo keskelle:
	window.setView(sf::View(
		sf::Vector2f(game.cameraX*32.0f, game.cameraY*32.0f),
		sf::Vector2f(window.getSize().x, window.getSize().y)
	));

	// Render map:
	sfml_application::render(window, game.map, textures);

	// Render goals:
	bool hasInvalidVisuals = false;
	game::forEachAliveObject(game.objects, [&](const auto& object, const auto& state) {
		if(object.visualId < 0 || object.visualId >= textures.size()) {
			hasInvalidVisuals = true;
			return;
		}
		sfml_application::render(window, sf::Vector2f(state[0], state[1]), 0.0f, *textures[object.visualId]);
	});

	// Render players:
	game::forEachAliveAgent(game.agents, [&](const auto& agent, const auto& state, const auto& history) {
		if(agent.visualId < 0 || agent.visualId >= textures.size()) {
			hasInvalidVisuals = true;
			return;
		}
		sfml_application::render(window, sf::Vector2f(state[0], state[1]), 0.0f, *textures[agent.visualId]);
	});

	if(hasInvalidVisuals) {
		printf("Object has some invalid visual ID in object or agent\n");
	}
}

///
/// \brief createClient
/// \param argc
/// \param argv
///
auto createClient(int argc, const char* argv[]) {
	if(argc != 2) {
		throw std::runtime_error("Usage: client ip:port\n");
	}
	// Parse args:
	std::string address = argv[1];
	auto loc = address.find(":");
	std::string ip = address.substr(0,loc);
	uint16_t port = std::atoi(address.substr(loc+1).c_str());

	// Create client:
	app::AppData<game::Game<AppData>,netcode::Connection> client = {
		game::createGame<game::Game<AppData> >(2), // 2 palyers
		netcode::createClient(ip.c_str(), port, app_msg::NUM_MESSAGES),
	};

	// Init empty game with player from keyboard arrows:
	bool initOk = game::initDefaultGame(client.game);
	game::reset(client.game); // Reset
	return client;
}


///
/// \brief run
/// \param client
/// \param readInputFunc
/// \param textures
/// \param playerName
/// \return
///
int run(auto& client, auto readInputFunc, const auto& textures, const std::string& playerName = "Player") {
	bool disconnected = false;

	//const float INPUT_SEND_PERIOD = 0.1s = 100 ms = 10 times/second
	game::PeriodicTimer inputSendTimer(0.1f);

	// Suorita sovellus ikkunan kanssa:
	int status = sfml_application::runGame("Netgame client", [&](float deltaTime) {
		netcode::update(client.conn, [&](int peerId) {
			printf("CLIENT: Connected to server. Joining game as: \"%s\".\n", playerName.c_str());
			app_msg::sendJoinRq(client.conn, playerName);
			return true;
		},
		[&](int peerId) {
			printf("Disconnected from server (peerId=%d)\n", peerId);
			disconnected = true;
		}, [&](const auto& msg) {
			return client::clientRX<app::State>(client, msg);
		});

		if(client.state == app::State::RUNNING) {
			//printf("Game running\n");
			stepFindGoalGame(client, deltaTime);
			// Send player inputs to server.
			inputSendTimer.update(deltaTime, [&](float) {
				//printf("Send update\n");
				app_msg::sendPlayerInput(client.conn, readInputFunc());
			});
		}
		//printf("deltaTime: %f, appState: %d\n", deltaTime, client.state);
		return client.game;
	}, [&](auto& window) {
		renderGame(window, client.game, textures);
		return !disconnected;
	});
	netcode::close(client.conn);
	game::printGameResults(client.game);
	return status;
}
}

