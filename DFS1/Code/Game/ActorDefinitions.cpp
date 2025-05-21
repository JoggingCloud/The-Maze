#include "Game/ActorDefinitions.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <d3d11.h>

ActorDefinition::ActorDefinition(const tinyxml2::XMLElement* element)
{
	m_name = ParseXmlAttribute(*element, "name", std::string());
	m_canBePossed = ParseXmlAttribute(*element, "canBePossessed", false);
	m_corpseLifetime = ParseXmlAttribute(*element, "corpseLifetime", 0.f);
	m_visible = ParseXmlAttribute(*element, "visible", false);
	m_isItem = ParseXmlAttribute(*element, "item", false);
}

void ActorDefinition::InitializeActorDef()
{
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("Data/Definitions/ActorDefinitions.xml") != tinyxml2::XML_SUCCESS)
	{
		// Handle error loading XML file
		return;
	}

	const tinyxml2::XMLElement* root = doc.FirstChildElement("Definitions");
	if (!root)
	{
		// Handle missing root element
		ERROR_AND_DIE("Failed to get the root in the actor definition xml file");
		return;
	}

	if (s_actorDefinition.empty())
	{
		for (const tinyxml2::XMLElement* actorElement = root->FirstChildElement("ActorDefinition"); actorElement; actorElement = actorElement->NextSiblingElement("ActorDefinition"))
		{
			ActorDefinition* newActorDef = new ActorDefinition(actorElement);

			// Parse optional attributes
			newActorDef->m_faction = ParseXmlAttribute(*actorElement, "faction", std::string());
			if (newActorDef->m_faction == "Player")
			{
				newActorDef->m_actorTypeByFaction = ActorType::ACTOR_PLAYER;
			}
			else if (newActorDef->m_faction == "Enemy")
			{
				newActorDef->m_actorTypeByFaction = ActorType::ACTOR_ENEMY;
			}
			else if (newActorDef->m_faction == "ItemBox")
			{
				newActorDef->m_actorTypeByFaction = ActorType::ACTOR_ITEMBOX;
			}

			newActorDef->m_health = ParseXmlAttribute(*actorElement, "health", 0);

			// Parse Collision element
			const tinyxml2::XMLElement* collisionElement = actorElement->FirstChildElement("Collision");
			if (collisionElement)
			{
				newActorDef->m_radius = ParseXmlAttribute(*collisionElement, "radius", 0.f);
				newActorDef->m_height = ParseXmlAttribute(*collisionElement, "height", 0.f);
				newActorDef->m_collidesWithWorld = ParseXmlAttribute(*collisionElement, "collidesWithWorld", false);
				newActorDef->m_collidesWithActors = ParseXmlAttribute(*collisionElement, "collidesWithActors", false);
				newActorDef->m_doesDieOnCollision = ParseXmlAttribute(*collisionElement, "dieOnCollide", false);
			}

			// Parse Physics element
			const tinyxml2::XMLElement* physicsElement = actorElement->FirstChildElement("Physics");
			if (physicsElement)
			{
				newActorDef->m_simulated = ParseXmlAttribute(*physicsElement, "simulated", false);
				newActorDef->m_walkSpeed = ParseXmlAttribute(*physicsElement, "walkSpeed", 0.f);
				newActorDef->m_runSpeed = ParseXmlAttribute(*physicsElement, "runSpeed", 0.f);
				newActorDef->m_turnSpeed = ParseXmlAttribute(*physicsElement, "turnSpeed", 0.f);
				newActorDef->m_flying = ParseXmlAttribute(*physicsElement, "flying", false);
				newActorDef->m_drag = ParseXmlAttribute(*physicsElement, "drag", 0.f);
			}

			// Parse Camera element
			const tinyxml2::XMLElement* cameraElement = actorElement->FirstChildElement("Camera");
			if (cameraElement)
			{
				newActorDef->m_eyeHeight = ParseXmlAttribute(*cameraElement, "eyeHeight", 0.f);
				newActorDef->m_cameraFOV = ParseXmlAttribute(*cameraElement, "cameraFOV", 0.f);
			}

			// Parse AI element
			const tinyxml2::XMLElement* aiElement = actorElement->FirstChildElement("AI");
			if (aiElement)
			{
				newActorDef->m_aiEnabled = ParseXmlAttribute(*aiElement, "aiEnabled", false);
				newActorDef->m_sightRadius = ParseXmlAttribute(*aiElement, "sightRadius", 0.f);
				newActorDef->m_sightAngle = ParseXmlAttribute(*aiElement, "sightAngle", 0.f);
			}

			// Parse Visuals
			const tinyxml2::XMLElement* visualsElement = actorElement->FirstChildElement("Visuals");
			if (visualsElement)
			{
				newActorDef->m_renderLit = ParseXmlAttribute(*visualsElement, "renderLit", false);
				newActorDef->m_renderRounded = ParseXmlAttribute(*visualsElement, "renderRounded", false);
			}
			const tinyxml2::XMLElement* inventoryElement = actorElement->FirstChildElement("Inventory");
			if (inventoryElement)
			{
				for (const tinyxml2::XMLElement* weaponElement = inventoryElement->FirstChildElement("Weapon"); weaponElement; weaponElement = weaponElement->NextSiblingElement("Weapon"))
				{
					std::string weaponName = ParseXmlAttribute(*weaponElement, "name", std::string());
					if (!weaponName.empty())
					{
						newActorDef->m_inventoryWeapons.push_back(weaponName);
					}
				}
			}
			s_actorDefinition.push_back(newActorDef);
		}
	}
}

ActorDefinition* ActorDefinition::GetActorDefByName(const std::string& name)
{
	for (int i = 0; i < s_actorDefinition.size(); i++)
	{
		if (s_actorDefinition[i]->m_name == name)
		{
			return s_actorDefinition[i];
		}
	}
	return nullptr;
}

ActorDefinitions::ActorDefinitions()
{
}

ActorDefinitions::~ActorDefinitions()
{
	ClearAllDefinitions();
}

void ActorDefinitions::ClearAllDefinitions()
{
	s_actorDefinition.clear();
	s_weaponDefinition.clear();
}

WeaponDefinition::WeaponDefinition(const tinyxml2::XMLElement* element)
{
	m_name = ParseXmlAttribute(*element, "name", std::string());
	m_refireTime = ParseXmlAttribute(*element, "refireTime", 0.f);

	// Enemy enemy
	m_enemyMeleeCount = ParseXmlAttribute(*element, "enemyMeleeCount", 0);
	m_enemyMeleeArc = ParseXmlAttribute(*element, "enemyMeleeArc", 0.f);
	m_enemyMeleeRange = ParseXmlAttribute(*element, "enemyMeleeRange", 0.f);
	m_enemyMeleeDamage = ParseXmlAttribute(*element, "enemyMeleeDamage", 0.f);
	m_enemyMeleeImpulse = ParseXmlAttribute(*element, "meleeImpulse", 0.f);
}

void WeaponDefinition::InitializeWeaponDef()
{
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("Data/Definitions/WeaponDefinitions.xml") != tinyxml2::XML_SUCCESS)
	{
		// Handle error loading XML file
		return;
	}

	const tinyxml2::XMLElement* root = doc.FirstChildElement("Definitions");
	if (!root)
	{
		// Handle missing root element
		ERROR_AND_DIE("Failed to get the root in the weapon definition xml file");
		return;
	}

	if (s_weaponDefinition.empty())
	{
		for (const tinyxml2::XMLElement* weaponElement = root->FirstChildElement("WeaponDefinition"); weaponElement; weaponElement = weaponElement->NextSiblingElement("WeaponDefinition"))
		{
			WeaponDefinition* newWeaponDef = new WeaponDefinition(weaponElement);
			s_weaponDefinition.push_back(newWeaponDef);
		}
	}
}

WeaponDefinition* WeaponDefinition::GetWeaponDefByName(const std::string& name)
{
	for (int i = 0; i < s_weaponDefinition.size(); i++)
	{
		if (s_weaponDefinition[i]->m_name == name)
		{
			return s_weaponDefinition[i];
		}
	}
	return nullptr;
}
