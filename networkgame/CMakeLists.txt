
set(GAME_FILES game/game.h game/model.h game/find_goal_game.h game/init_game.h)
set(NETGAME_FILES netgame/netcode.h netgame/app_protocol.h netgame/serialize.h)

include_directories("./")

add_executable(shard app/shard_app.h app/main_shard_server.cpp ${NETGAME_FILES})
target_link_libraries(shard enet)

add_executable(client app/client_app.h app/main_client.cpp)
target_link_libraries(client sfml-graphics enet)

add_executable(random_agent app/main_random_agent.cpp)
target_link_libraries(random_agent sfml-graphics enet)

add_executable(standalone app/standalone_app.h app/main_standalone.cpp ${GAME_FILES})
target_link_libraries(standalone sfml-graphics enet)

add_custom_command(TARGET standalone POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	"${CMAKE_CURRENT_SOURCE_DIR}/assets"
	"${PROJECT_BINARY_DIR}/networkgame/assets"
	COMMENT "Copying asset files to binary directory")
