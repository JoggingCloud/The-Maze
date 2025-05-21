#pragma once
#include "Game/Actor.hpp"

class Map;

class Item : public Actor
{
public:
	Item(ActorUID uid, Map* map);
	~Item() = default;

	void Update();

public:
	ActorUID m_actorUID = ActorUID::INVALID;
	Actor* m_actor = nullptr;	
	Map* m_map = nullptr;
};