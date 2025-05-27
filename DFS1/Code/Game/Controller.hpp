#pragma once
#include "Game/GameCommon.hpp"
#include "Game/ActorUID.hpp"

//------------------------------------------------------------------------------------------------
class Actor;
class AI;
class Game;

//------------------------------------------------------------------------------------------------
class Controller
{
	friend class Actor;

public:
	Controller();
	virtual ~Controller();

	virtual void Update() = 0;

	void Possess(Actor* newActor);
	Actor* GetActor() const;

	virtual bool IsPlayer();
	virtual void DamagedBy(Actor* actor);

public:
	ActorUID m_actorUID = ActorUID::INVALID;
	Game* m_game = nullptr;
};