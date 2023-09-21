#pragma once
#include <assert.h>
#include <thread>
#include <stdexcept>
#include <game/init_game.h>
#include <game/find_goal_game.h>
#include <app/client_app.h>
#include <game/model.h>
#include <view/sfml_application.h>

namespace standalone_app {
typedef int AppData; // texture id.

/// \brief Steps server game according to deltaTime
inline bool stepFindGoalGame(auto& gameState, float deltaTime) {
	find_goal_game::update(gameState, deltaTime);
	return find_goal_game::isEnd(gameState);
}

///
/// \brief createGameApp
/// \param argc
/// \param argv
///
auto createGameApp(int argc, const char* argv[]) {
	// Create server:
	typedef game::Game<AppData> GameType;
	app::AppData<GameType,netcode::Connection> app = {
		game::createGame<GameType>(2),
	};

	bool initOk = game::initDefaultGame(app.game);

	// Game Behavioural functor for local player:
	app.game.ClassAgent.policy = [](auto& agent, const auto& gameState) {
		return game::inputToActionId(agent.inputs);
	};

	game::spawnAgent(app.game);
	return app;
}


///
/// \brief run
/// \param client
/// \param textures
/// \param playerName
/// \return
///
int run(auto& app, auto readInputFunc, const auto& textures, const std::string& playerName = "Player") {
	// Suorita sovellus ikkunan kanssa:
	bool isEnd = false;
	int status = sfml_application::runGame("Netgame standalone", [&](float deltaTime) {
		game::setPlayerInput(app.game, 0, readInputFunc());
		isEnd = stepFindGoalGame(app.game, deltaTime);
		return app.game;
	}, [&](auto& window) {
		client_app::renderGame(window, app.game, textures);
		return !isEnd;
	});
	game::printGameResults(app.game);
	return status;
}
}

