#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Game/ActorType.hpp"
#include <vector>

class SpriteSheet;
class Image;
class Shader;

struct WeaponDefinition
{
	WeaponDefinition() {}
	explicit WeaponDefinition(const tinyxml2::XMLElement* element);
	std::string m_name = "";
	float m_refireTime = 0.f;

	// Enemy Melee
	int m_enemyMeleeCount = 0;
	float m_enemyMeleeArc = 0.f;
	float m_enemyMeleeRange = 0.f;
	float m_enemyMeleeDamage = 0.f;
	float m_enemyMeleeImpulse = 0.f;

	static void InitializeWeaponDef();
	static WeaponDefinition* GetWeaponDefByName(const std::string& name);
};

static std::vector<WeaponDefinition*> s_weaponDefinition;

struct ActorDefinition
{
	ActorDefinition() {};
	explicit ActorDefinition(const tinyxml2::XMLElement* element);

	std::string m_name = "";
	std::string m_faction = "";
	ActorType m_actorTypeByFaction;
	int m_health = 0;
	bool m_canBePossed = false;
	float m_corpseLifetime = 0.f;
	bool m_visible = false;

	// Collision
	float m_radius = 0.f;
	float m_height = 0.f;
	bool m_collidesWithWorld = false;
	bool m_collidesWithActors = false;
	bool m_doesDieOnCollision = false;

	// Physics
	bool m_simulated = false;
	float m_walkSpeed = 0.f;
	float m_runSpeed = 0.f;
	float m_turnSpeed = 0.f;
	bool m_flying = false;
	float m_drag = 0.f;

	// Camera
	float m_eyeHeight = 0.f;
	float m_cameraFOV = 0.f;

	// Ai
	bool m_aiEnabled = false;
	float m_sightRadius = 0.f;
	float m_sightAngle = 0.f;

	// Item
	bool m_isItem = false;

	// Visuals 
	bool m_renderLit = false;
	bool m_renderRounded = false;

	// Inventory
	std::vector<std::string> m_inventoryWeapons;

	static void InitializeActorDef();
	static ActorDefinition* GetActorDefByName(const std::string& name);
};
static std::vector<ActorDefinition*> s_actorDefinition;

class ActorDefinitions
{
public:
	ActorDefinitions();
	~ActorDefinitions();

	void ClearAllDefinitions();
};