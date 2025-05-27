#include "Game/Weapon.hpp"
#include "Game/ActorDefinitions.hpp"
#include "Game/Actor.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/Clock.hpp"
#include "Game/Map.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Game/App.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/EngineCommon.hpp"

Weapon::Weapon(const WeaponDefinition& weaponInfo, Actor* actor)
{
	m_actor = actor;
	m_weaponDefName = weaponInfo.m_name;
	m_refireTime = weaponInfo.m_refireTime;

	// Enemy Melee
	m_enemyMeleeCount = weaponInfo.m_enemyMeleeCount;
	m_enemyMeleeArc = weaponInfo.m_enemyMeleeArc;
	m_enemyMeleeRange = weaponInfo.m_enemyMeleeRange;
	m_enemyMeleeDamage = weaponInfo.m_enemyMeleeDamage;
	m_enemyMeleeImpulse = weaponInfo.m_enemyMeleeImpulse;

	m_weaponDefinition = weaponInfo.GetWeaponDefByName(m_weaponDefName);
}

Weapon::~Weapon()
{
}

std::string Weapon::GetCurrentWeaponDefName() const
{
	return m_weaponDefName.c_str();
}

void Weapon::Fire()
{
	if (m_refireTime <= 0.f)
	{
		if (m_weaponDefName == "EnemyMelee")
		{
			SpawnInfo info;
			info.m_actorType = m_weaponDefName;
			info.m_actorPosition = Vec3(m_actor->m_position.x, m_actor->m_position.y, m_actor->m_position.z + m_actor->m_eyeHeight);
			info.m_actorOrientation = m_actor->m_orientation;
			for (int i = 0; i < m_enemyMeleeCount; i++)
			{
				Actor* actor = g_theApp->m_game->m_currentMap->GetPlayerActor();
				if (actor)
				{
					actor->Damage(m_enemyMeleeDamage, m_actor->GetController());
				}
			}
		}
		m_refireTime = m_weaponDefinition->m_refireTime;
	}
}

void Weapon::Update()
{
	m_refireTime -= g_theApp->m_game->m_clock->GetDeltaSeconds();
}

Vec3 Weapon::GetRandomDirectionInCone(float spreadDegrees) const
{
	EulerAngles result;
	result = m_actor->m_orientation;
	float xAngle = g_rng.RollRandomFloatInRange(0.f, 360.f);
	float yAngle = g_rng.RollRandomFloatInRange(0.f, 360.f);

	float x = SinDegrees(xAngle) * CosDegrees(xAngle);
	float y = SinDegrees(yAngle) * CosDegrees(yAngle);

	result += EulerAngles(x * spreadDegrees, y * spreadDegrees, 0.f);
	return result.GetForwardVector();
}