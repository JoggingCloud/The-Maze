#include "Game/Controller.hpp"
#include "Game/Actor.hpp"
#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/Player.hpp"

Controller::Controller()
{
}

Controller::~Controller()
{
}

void Controller::Possess(Actor* newActor)
{
	Actor* currentActor = GetActor();
	if (currentActor != nullptr)
	{
		currentActor->OnUnPossessed(this);
	}

	if (newActor == nullptr || !newActor->m_canBePossessed)
	{
		return;
	}

	// Set the new actor
	m_actorUID = newActor->GetUID();
	newActor->OnPossessed(this);
}

Actor* Controller::GetActor() const
{
	if (m_actorUID == ActorUID::INVALID)
	{
		return nullptr;
	}

	Actor* currentPossessedActor = g_theApp->m_game->m_currentMap->GetActorByUID(m_actorUID);
	return currentPossessedActor;
}

bool Controller::IsPlayer()
{
	return GetActor() == g_theApp->m_game->m_currentMap->GetPlayerActor();
}

void Controller::DamagedBy(Actor* actor)
{
	actor = nullptr;
}