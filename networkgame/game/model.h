#pragma once
#include <vector>
#include <string>
#include <functional>
#include <memory>

namespace game {

template<typename StateType, typename ActionType>
struct TrajectoryNode {
	//
	StateType state;
	ActionType action;
	// Linkki edelliseen solmuun
	std::shared_ptr<TrajectoryNode> prevNode = 0;
};


template<typename Game, typename Event>
struct ObjectClass {
	typedef std::function<void(Game&, int, float)>			UpdateFunc;
	typedef std::function<int(ObjectClass&, const Game&)>	PolicyFunc;
	typedef std::function<void(ObjectClass&, const Event&)>	EventFunc;
	int			visualId	= -1;
	UpdateFunc	update		= 0;
	PolicyFunc	policy		= 0;
	EventFunc	event		= 0;
	bool		alive		= true;
	bool		isReady		= false;
	int			actionId	= -1;
	int			id			= -1;
	std::string playerName;
	std::vector<float> inputs;
};

template<typename ClassType, typename StateType, typename AppData>
struct ObjectGroup {
	std::vector<ClassType>	objects;
	std::vector<StateType>	states;
	std::vector<AppData>	appDatas;
	void clear() {
		objects.clear();
		states.clear();
		appDatas.clear();
	}
	auto size() const {
		return objects.size();
	}
};

template<typename ClassType, typename StateType, typename AppData, typename NodeType>
struct ActorGroup {
	std::vector<ClassType>	objects;
	std::vector<StateType>	states;
	std::vector<AppData>	appDatas;
	std::vector< std::shared_ptr<NodeType> >	historys;
	void clear() {
		objects.clear();
		states.clear();
		appDatas.clear();
		historys.clear();
	}
	auto size() const {
		return objects.size();
	}
};

// Pelin tila (+ actionit)
template<typename AppData>
struct Game {
	typedef std::vector<float>									ObjectStateType;
	typedef int													ActionId;
	typedef std::string											Event;
	typedef std::function<Event(Game&, int, float)>				ActionFunc;
	typedef TrajectoryNode<ObjectStateType,ActionId>			NodeType;
	typedef ObjectClass<Game,Event>								ObjectClassType;
	typedef ObjectGroup<ObjectClassType,ObjectStateType,AppData>		Objects;
	typedef ActorGroup<ObjectClassType,ObjectStateType,AppData,NodeType>Agents;


	const int				NUM_PLAYERS = 2;
	const std::vector<ActionFunc> actions = {
		// Kävely oikealle
		[](Game& game, auto agentId, auto value) {
			game.agents.states[agentId][0] += value;
			return Event();
		},
		// Kävely vasemmalle
		[](Game& game, auto agentId, auto value) {
			game.agents.states[agentId][0] -= value;
			return Event();
		},
		// Kävely ylös
		[](Game& game, auto agentId, auto value) {
			game.agents.states[agentId][1] += value;
			return Event();
		},
		// Kävely alas
		[](Game& game, auto agentId, auto value) {
			game.agents.states[agentId][1] -= value;
			return Event();
		}
	};
	std::vector< std::vector<int> >		map;
	ObjectClassType			ClassAgent;
	ObjectClassType			ClassGoal;
	Agents					agents;
	Objects					objects;
	float					cameraX = 0;
	float					cameraY = 0;
	int						n = 0;

};

} // End - namespace game


namespace app {

enum State {
	WAITING_PLAYERS,
	SPAWNING,
	RUNNING,
	ENDED,
};

template<typename GameType, typename ConnectionType=int>
struct AppData {
	GameType		game;
	ConnectionType	conn;
	State			state = WAITING_PLAYERS;
};

} // End - namespace app

