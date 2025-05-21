#include "Item.hpp"
#include "Game/Map.hpp"
#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Game/ActorType.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include <cmath>

Item::Item(ActorUID uid, Map* map)
	: m_actorUID(uid), m_map(map)
{
}

void Item::Update()
{
	m_actor = m_map->GetActorByUID(m_actorUID);

	float rotationSpeed = 50.f;
	float rotation = rotationSpeed * g_theApp->m_game->m_clock->GetDeltaSeconds();

	m_actor->m_orientation.m_yawDegrees += rotation;
	m_actor->m_orientation.m_rollDegrees += rotation;
	m_actor->m_orientation.m_pitchDegrees += rotation;

 	// Up-and-down oscillation using sine function
	float amplitude = 0.25f; // Amplitude of the oscillation
	float frequency = 2.f; // Frequency of the oscillation (oscillations per second)
	float baseHeight = 1.1f; // Base height for the oscillation
	float time = g_theApp->m_game->m_clock->GetTotalSeconds();

	m_actor->m_position.z = baseHeight + amplitude * std::sin(frequency * time);
}
