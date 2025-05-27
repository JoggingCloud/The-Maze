#pragma once
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Audio/AudioSystem.hpp"

struct AnimationDefinition;
struct WeaponDefinition;
class Shader;
class Actor;
class Game;
class Image;

class Weapon
{
public:
	Weapon(const WeaponDefinition& weaponInfo, Actor* actor);
	Weapon() = default;
	~Weapon();

public:
	std::string GetCurrentWeaponDefName() const;
	void Fire();
	void Update();
	Vec3 GetRandomDirectionInCone(float spreadDegrees) const;

public:
	WeaponDefinition* m_weaponDefinition = nullptr;
	AnimationDefinition* m_weaponAnimation = nullptr;
	Actor* m_actor = nullptr;

	std::string m_weaponDefName = "";
	float m_refireTime = 0.f;

	// Enemy Melee
	int m_enemyMeleeCount = 0;
	float m_enemyMeleeArc = 0.f;
	float m_enemyMeleeRange = 0.f;
	float m_enemyMeleeDamage = 0.f;
	float m_enemyMeleeImpulse = 0.f;
};