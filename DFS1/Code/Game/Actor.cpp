#include "Game/Actor.hpp"
#include "Game/ActorDefinitions.hpp"
#include "Game/Weapon.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Game/Controller.hpp"
#include "Game/AIActor.hpp"
#include "Game/Item.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Game/App.hpp"
#include "Game/Player.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"

Actor::Actor(Map* owner, SpawnInfo spawnInfo, ActorUID actorUID)
{
	m_map = owner;
	m_animClock = new Clock(*m_map->m_game->m_clock);
	m_actorDefName = spawnInfo.m_actorType;
	m_position = spawnInfo.m_actorPosition;
	m_orientation = spawnInfo.m_actorOrientation;
	m_actorDef = ActorDefinition::GetActorDefByName(m_actorDefName);

	if (m_actorDef)
	{
		m_actorName = m_actorDef->m_name;
		m_isAI = m_actorDef->m_aiEnabled;
		m_isItem = m_actorDef->m_isItem;
		m_actorFaction = m_actorDef->m_faction;
		m_type = m_actorDef->m_actorTypeByFaction;

		m_isMoveable = m_actorDef->m_simulated;
		m_walkSpeed = m_actorDef->m_walkSpeed;
		m_runSpeed = m_actorDef->m_runSpeed;
		m_turnSpeed = m_actorDef->m_turnSpeed;
		m_isFlying = m_actorDef->m_flying;

		m_physicsHeight = m_actorDef->m_height;
		m_eyeHeight = m_actorDef->m_eyeHeight;
		m_cameraFOV = m_actorDef->m_cameraFOV;
		m_physicsRadius = m_actorDef->m_radius;

		m_health = m_actorDef->m_health;
		m_corpseLifeTime = m_actorDef->m_corpseLifetime;
		m_canBePossessed = m_actorDef->m_canBePossed;
		m_canCollideWithActors = m_actorDef->m_collidesWithActors;
		m_canCollideWithWorld = m_actorDef->m_collidesWithWorld;
		m_dieOnCollide = m_actorDef->m_doesDieOnCollision;

		m_sightRadii = m_actorDef->m_sightRadius;
		m_sightAngle = m_actorDef->m_sightAngle;
		m_isVisible = m_actorDef->m_visible;
		m_dragForce = m_actorDef->m_drag;
		m_actorInventory = m_actorDef->m_inventoryWeapons;

		// Visuals
		m_isRenderedLit = m_actorDef->m_renderLit;
		m_isRenderedRounded = m_actorDef->m_renderRounded;

		if (!m_actorInventory.empty())
		{
			EquipWeapon(0);
		}

		if (m_actorDef->m_name == "Player")
		{
			CreateZAlignedAgent();
			CreateBuffers();
			m_playerColor = Rgba8::GREEN;
			m_playerEyeColor = Rgba8::MAGENTA;
		}
		else if (m_actorDef->m_name == "Enemy")
		{
			CreateZAlignedAgent();
			CreateBuffers();
			m_enemyColor = Rgba8::RED;
		}
		else if (m_actorDef->m_name == "ItemBox")
		{
			CreateItemBox();
			CreateBuffers();
			m_itemColor = Rgba8::AQUA;
		}
	}

	m_uid = actorUID;
	
	if (m_isAI)
	{
		m_aiController = new AIActor(m_map->m_game, m_map);
		m_aiController->Possess(this);
	}

	if (m_isItem)
	{
		m_item = new Item(m_uid, m_map);
	}
}

Actor::~Actor()
{
	SafeDelete(m_bodyVertexBuffer);
	SafeDelete(m_bodyIndexBuffer);
	SafeDelete(m_playerEyeVertexBuffer);
	SafeDelete(m_playerEyeIndexBuffer);
	SafeDelete(m_itemVertexBuffer);
	SafeDelete(m_itemIndexBuffer);

	m_actorBodyIndicies.clear();
	m_actorBodyVertices.clear();

	m_playerActorEyeIndicies.clear();
	m_playerActorEyeVertices.clear();

	m_itemIndicies.clear();
	m_itemVertices.clear();

	m_actorInventory.clear();
	SafeDelete(m_currentWeapon);
}

void Actor::Update()
{
	if (m_currentWeapon)
	{
		m_currentWeapon->Update();
	}

	if (m_isVisible && !m_isItem)
	{
		m_owningController->Update();
	}

	if (m_isItem)
	{
		m_item->Update();
	}

	UpdatePhysiscs();

	if (!m_isFlying)
	{
		m_position.z = 0.f;
	}

	if (m_isDead)
	{
		m_corpseLifeTime -= m_map->m_game->m_clock->GetDeltaSeconds();
		if (m_corpseLifeTime <= 0.f)
		{
			m_isDestroyed = true;
		}
	}
}

void Actor::CreateZAlignedAgent()
{
	AddVertsForZCylinder3D(m_actorBodyVertices, m_actorBodyIndicies, Vec3(0.f, 0.f, m_physicsRadius), Vec3(0.f, 0.f, m_physicsHeight), m_physicsRadius, 16, Rgba8::WHITE, AABB2::ZERO_TO_ONE);
	
	if (m_actorDef->m_name == "Player")
	{
		AddVertsForCone3D(m_playerActorEyeVertices, m_playerActorEyeIndicies, Vec3(m_physicsRadius * 0.8f, 0.f, m_eyeHeight), Vec3(m_physicsRadius, 0.f, m_eyeHeight), m_physicsRadius, m_physicsRadius * 0.5f, 32, Rgba8::WHITE);
	}
}

void Actor::CreateItemBox()
{
	OBB3 boxBounds = OBB3(Vec3::ZERO, GetModelMatrix().GetIBasis3D(), GetModelMatrix().GetJBasis3D(), GetModelMatrix().GetKBasis3D(), Vec3(0.5f, 0.5f, 0.5f));
	AddVertsForOBB3D(m_itemVertices, m_itemIndicies, boxBounds);
}

void Actor::CreateBuffers()
{
	if (m_actorDef->m_name == "Player" || m_actorDef->m_name == "Enemy")
	{
		m_bodyVertexBuffer = g_theRenderer->CreateVertexBuffer(m_actorBodyVertices.size());
		g_theRenderer->CopyCPUToGPU(m_actorBodyVertices.data(), m_actorBodyVertices.size() * sizeof(Vertex_PCU), m_bodyVertexBuffer);

		m_bodyIndexBuffer = g_theRenderer->CreateIndexBuffer(m_actorBodyIndicies.size());
		g_theRenderer->CopyCPUToGPU(m_actorBodyIndicies.data(), m_actorBodyIndicies.size() * sizeof(unsigned int), m_bodyIndexBuffer);
	}

	if (m_actorDef->m_name == "Player")
	{
		m_playerEyeVertexBuffer = g_theRenderer->CreateVertexBuffer(m_playerActorEyeVertices.size());
		g_theRenderer->CopyCPUToGPU(m_playerActorEyeVertices.data(), m_playerActorEyeVertices.size() * sizeof(Vertex_PCU), m_playerEyeVertexBuffer);

		m_playerEyeIndexBuffer = g_theRenderer->CreateIndexBuffer(m_playerActorEyeIndicies.size());
		g_theRenderer->CopyCPUToGPU(m_playerActorEyeIndicies.data(), m_playerActorEyeIndicies.size() * sizeof(unsigned int), m_playerEyeIndexBuffer);
	}

	if (m_actorDef->m_name == "ItemBox")
	{
		m_itemVertexBuffer = g_theRenderer->CreateVertexBuffer(m_itemVertices.size());
		g_theRenderer->CopyCPUToGPU(m_itemVertices.data(), m_itemVertices.size() * sizeof(Vertex_PCU), m_itemVertexBuffer);

		m_itemIndexBuffer = g_theRenderer->CreateIndexBuffer(m_itemIndicies.size());
		g_theRenderer->CopyCPUToGPU(m_itemIndicies.data(), m_itemIndicies.size() * sizeof(unsigned int), m_itemIndexBuffer);
	}
}

void Actor::Render()
{
	if (!m_isVisible) return;

	std::vector<Vertex_PCU> actorVerts;
	switch (m_type)
	{
	case ActorType::ACTOR_ENEMY:
	{
		g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
		g_theRenderer->SetDepthMode(DepthMode::ENABLED);
		g_theRenderer->SetRasterizerState(RasterizerMode::SOLID_CULL_NONE);
		g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
		g_theRenderer->SetModelConstants(GetModelMatrix(), m_enemyColor);
		g_theRenderer->BindTexture(0, nullptr);
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->DrawVertexBufferIndex(m_bodyVertexBuffer, m_bodyIndexBuffer, VertexType::Vertex_PCU, static_cast<int>(m_actorBodyIndicies.size()));
		break;
	}
	case ActorType::ACTOR_PLAYER:
	{
		g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
		g_theRenderer->SetDepthMode(DepthMode::ENABLED);
		g_theRenderer->SetRasterizerState(RasterizerMode::SOLID_CULL_NONE);
		g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
		g_theRenderer->SetModelConstants(GetModelMatrix(), m_playerColor);
		g_theRenderer->BindTexture(0, nullptr);
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->DrawVertexBufferIndex(m_bodyVertexBuffer, m_bodyIndexBuffer, VertexType::Vertex_PCU, static_cast<int>(m_actorBodyIndicies.size()));
		
		g_theRenderer->SetModelConstants(GetModelMatrix(), m_playerEyeColor);
		g_theRenderer->DrawVertexBufferIndex(m_playerEyeVertexBuffer, m_playerEyeIndexBuffer, VertexType::Vertex_PCU, static_cast<int>(m_playerActorEyeIndicies.size()));
		break;
	}
	case ActorType::ACTOR_ITEMBOX:
	{
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		g_theRenderer->SetDepthMode(DepthMode::ENABLED);
		g_theRenderer->SetRasterizerState(RasterizerMode::SOLID_CULL_BACK);
		g_theRenderer->SetModelConstants(GetModelMatrix(), m_itemColor);
		g_theRenderer->BindTexture(0, m_map->m_game->m_hourglassTexture);
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->DrawVertexBufferIndex(m_itemVertexBuffer, m_itemIndexBuffer, VertexType::Vertex_PCU, static_cast<int>(m_itemIndicies.size()));

		g_theRenderer->SetRasterizerState(RasterizerMode::WIREFRAME_CULL_BACK);
		g_theRenderer->DrawVertexBufferIndex(m_itemVertexBuffer, m_itemIndexBuffer, VertexType::Vertex_PCU, static_cast<int>(m_itemIndicies.size()));
		break;
	}
	}
}

Mat44 Actor::GetModelMatrix()
{
	Mat44 translation = Mat44::CreateTranslation3D(m_position);
	Mat44 orientation = m_orientation.GetAsMatrix_IFwd_JLeft_KUp();

	translation.Append(orientation);
	return translation;
}

ActorType Actor::GetType()
{
	return m_type;
}

ActorUID Actor::GetUID() const
{
	return m_uid;
}

void Actor::SetUID(ActorUID uid)
{
	m_uid = uid;
}

Controller* Actor::GetController() const
{
	return m_owningController;
}

AIActor* Actor::GetAiController() const
{
	return m_aiController;
}

void Actor::UpdatePhysiscs()
{
	float deltaSeconds = m_map->m_game->m_clock->GetDeltaSeconds();

	if (!m_isMoveable || m_isCorspe)
	{
		return;
	}

	Vec3 drag = -m_velocity;
	AddForce(drag);

	// Update velocity 
	m_velocity += m_acceleration * deltaSeconds * 0.5f;

	// update position
	m_position += m_velocity * deltaSeconds;

	m_velocity += m_acceleration * deltaSeconds * 0.5f;

	// Reset acceleration for next frame
	m_acceleration = Vec3::ZERO;
}

void Actor::AddForce(const Vec3& force)
{
	m_acceleration += force * m_dragForce;
}

void Actor::AddImpulse(const Vec3& impulse)
{
	m_velocity += impulse;
}

void Actor::MoveInDirection(Vec3 direction, float speed)
{
	Vec3 normalizedDirection = direction.GetNormalized();
	AddForce(normalizedDirection * speed);
}

void Actor::TurnInDirection(float goalDegree, float maxAngle)
{
	m_orientation.m_yawDegrees = GetTurnedTowardDegrees(m_orientation.m_yawDegrees, goalDegree, maxAngle);
}

void Actor::OnCollide()
{
	m_map->m_gameTime += 5.f;
	m_map->m_didAddTime = true;
	m_isDead = true;
}

void Actor::OnPossessed(Controller* controller)
{
	m_owningController = controller;
}

void Actor::OnUnPossessed(Controller* controller)
{
	// first set current controller to nullptr 
	controller = nullptr;
	if (m_isAI)
	{
		controller = m_aiController;
	}
}

std::string Actor::GetEquippedWeaponByName() const
{
	if (m_currentWeapon)
	{
		return m_currentWeapon->GetCurrentWeaponDefName();
	}
	return "";
}

bool Actor::HasWeaponAtIndex(int weaponIndex)
{
	return weaponIndex >= 0 && weaponIndex < m_actorInventory.size();
}

void Actor::EquipWeapon(int weaponIndex)
{
	std::string weaponName = m_actorInventory[weaponIndex];

	WeaponDefinition* weaponDef = WeaponDefinition::GetWeaponDefByName(weaponName);

	m_currentWeapon = new Weapon(*weaponDef, this);

	m_equippedWeaponIndex = weaponIndex;
}

void Actor::IncrementEquippedWeaponIndex()
{
	m_equippedWeaponIndex++;
	if (m_equippedWeaponIndex == 3)
	{
		m_equippedWeaponIndex = 0;
	}
}

void Actor::DecrementEquippedWeaponIndex()
{
	m_equippedWeaponIndex--;
	if (m_equippedWeaponIndex == -1)
	{
		m_equippedWeaponIndex = 2;
	}
}

void Actor::Attack()
{
	if (m_currentWeapon)
	{
		m_currentWeapon->Fire();
	}
}

void Actor::Damage(float amount, Controller* source)
{
	m_health -= static_cast<int>(amount);
	m_isTakingDamage = true;

	if (source != nullptr)
	{
		m_owningController->DamagedBy(source->GetActor());
	}

	if (m_health <= 0)
	{
		m_isTakingDamage = false;
		m_isCorspe = true;
	}
}

Vec3 Actor::GetActorPosition()
{
	return m_position;
}

bool Actor::IsItem() const
{
	return m_type == ActorType::ACTOR_ITEMBOX;
}

bool Actor::IsEnemy() const
{
	return m_type == ActorType::ACTOR_ENEMY;
}

bool Actor::IsPlayer() const
{
	return m_type == ActorType::ACTOR_PLAYER;
}

bool Actor::IsSpawnPoint() const
{
	return m_type == ActorType::ACTOR_SPAWN;
}

bool Actor::IsAlive()
{
	return !m_isDead;
}

void Actor::DestroyDeadActor(Controller* controller)
{
	if (controller == m_aiController)
	{
		m_map->DeleteDestroyedActors();
	}
}
