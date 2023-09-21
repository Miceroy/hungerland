#pragma once
#include <view/console_application.h>
#include <netgame/app_protocol.h>
#include <game/find_goal_game.h>
#include <game/init_game.h>
#include <game/model.h>
#include <thread>



namespace shard_app {
	typedef int AppData;

	/// \brief Steps server game according to deltaTime
	inline bool stepFindGoalGame(auto& server, float deltaTime, int syncIntervalInFrames) {
		auto events = find_goal_game::update(server.game, deltaTime);
		if(events.size() > 0) {
			app_msg::sendEvents(server.conn, events);
		}

		// Lähetä SYUNC vain joka "syncIntervalInFrames" -frame
		if((server.game.n % syncIntervalInFrames)==0) {
			//printf("Send SYNC\n");
			app_msg::sendSync(server.conn, server.game);
		}
		return find_goal_game::isEnd(server.game);
	}

	///
	/// \brief createShardApplication
	/// \param argc
	/// \param argv
	///
	auto createShardApp(int argc, const char* argv[]) {
		// Handle args:
		if(argc != 2) {
			throw std::runtime_error("No command line arguments!\nUsage: shard <port>\n");
		}
		uint16_t port = std::atoi(argv[1]);
		if(port == 0) {
			throw std::runtime_error("Invalid port!\nUsage: shard <port>\n");
		}

		// Create server:
		typedef game::Game<AppData> GameType;
		app::AppData<GameType,netcode::Connection> server = {
			game::createGame<GameType>(2),
			netcode::createHost(port, app_msg::NUM_MESSAGES),
		};

		bool initOk = game::initDefaultGame(server.game);
		return server;
	}

	///
	/// \brief run
	/// \param server
	/// \return
	///
	int run(auto& server) {
		// Suorita serveri sovellus:
		// 0.008f s = 8 ms = 125 times/second
		game::PeriodicTimer updateTimer(0.008f);

		// Sync vain joka 5:s frame ali noin 125/5 = 25 times/second
		int syncIntervalInFrames = 5;

		int n = 0;
		bool isEnd = false;
		int status = console_application::runGame([&](float dt) {
			netcode::update(server.conn, [&](int peerId) {
				printf("Players: %d/%d\n", peerId+1, server.game.NUM_PLAYERS);
				return shard::spawnPlayer<game::Game<AppData>::ObjectClassType>(server, peerId);
			}, [&](int peerId) {
				shard::destroyPlayer(server, peerId);
			}, [&](const auto& msg) {
				return shard::serverRX<app::State>(server, msg);
			});
			return shard::STM<app::State>(server, n, [&]() {
				updateTimer.update (dt, [&](float deltaTime) {
					//printf("deltaTime: %f\n", dt);
					isEnd = shard_app::stepFindGoalGame(server, deltaTime, syncIntervalInFrames);
				});
				return isEnd;
			});
		});
		netcode::close(server.conn);

		printf("Game over after %d steps. Results:\n", n);

		game::printAgents(server.game);
		return status;
	}
}



