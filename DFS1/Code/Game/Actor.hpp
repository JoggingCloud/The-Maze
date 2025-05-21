#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Game/ActorUID.hpp"
#include "Game/ActorType.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include <vector>

class Clock;
class Map;
class Game;
class Controller;
class AIActor;
class Item;
class Weapon;
class Timer;
class Shader;
class SpriteSheet;
class SpriteDefinition;
class Camera;
class VertexBuffer;
class IndexBuffer;
struct SpawnInfo;
struct ActorDefinition;

class Actor
{
public:
	Actor(Map* owner, SpawnInfo spawnInfo, ActorUID actorUID);
	Actor() = default;
	~Actor();

public:
	void Update();
	void CreateZAlignedAgent();
	void CreateItemBox();
	void CreateBuffers();
	void Render();
	Mat44 GetModelMatrix();

	ActorType GetType();
	ActorUID GetUID() const;
	void SetUID(ActorUID uid);
	Controller* GetController() const;
	AIActor* GetAiController() const;

	void UpdatePhysiscs();
	void AddForce(const Vec3& force);
	void AddImpulse(const Vec3& impulse);
	void MoveInDirection(Vec3 direction, float speed);
	void TurnInDirection(float goalDegree, float maxAngle);

	void OnCollide();

	void OnPossessed(Controller* controller);
	void OnUnPossessed(Controller* controller);

	std::string GetEquippedWeaponByName() const;
	bool HasWeaponAtIndex(int weaponIndex);
	void EquipWeapon(int weaponIndex);
	void IncrementEquippedWeaponIndex();
	void DecrementEquippedWeaponIndex();
	void Attack();

	void Damage(float amount, Controller* source);
	Vec3 GetActorPosition();
	bool IsItem() const;
	bool IsEnemy() const;
	bool IsPlayer() const;
	bool IsSpawnPoint() const;
	bool IsAlive();
	void DestroyDeadActor(Controller* controller);

public:
	Map* m_map = nullptr;
	Clock* m_animClock = nullptr;
	Timer* m_animTimer = nullptr;
	ActorDefinition* m_actorDef = nullptr;
	ActorType m_type = ActorType::UNKNOWN;
	std::string m_actorDefName;
	ActorUID m_uid;
	ActorUID m_ownerUID;

	Vec3 m_acceleration = Vec3::ZERO;
	Vec3 m_velocity = Vec3::ZERO;
	Vec3 m_preferredVelocity = Vec3::ZERO;
	Vec3 m_position = Vec3::ZERO;
	
	EulerAngles m_orientation = EulerAngles::ZERO;

	Rgba8 m_playerColor = Rgba8::WHITE;
	Rgba8 m_playerEyeColor = Rgba8::WHITE;
	Rgba8 m_enemyColor = Rgba8::WHITE;
	Rgba8 m_itemColor = Rgba8::WHITE;

	std::string m_actorName = "";
	std::string m_actorFaction = "";
	std::vector<std::string> m_actorInventory;
	Weapon* m_currentWeapon = nullptr;
	int m_equippedWeaponIndex = 0;

	float m_walkSpeed = 0.f;
	float m_runSpeed = 0.f;
	float m_turnSpeed = 0.f;
	float m_physicsRadius = 0.f;
	float m_physicsHeight = 0.f;
	float m_eyeHeight = 0.f;
	float m_cameraFOV = 0.f;
	float m_damage = 0.f;
	float m_impulseOnCollide = 0.f;
	int m_health = 0;
	bool m_isMoveable = false;
	bool m_canBePossessed = false;
	bool m_canCollideWithActors = false;
	bool m_canCollideWithWorld = false;
	bool m_isVisible = false;
	bool m_isAI = false;
	bool m_isItem = false;
	bool m_isFlying = false;
	float m_sightRadii = 0.f;
	float m_sightAngle = 0.f;
	float m_dragForce = 0.f;
	float m_corpseLifeTime = 0.f;
	bool m_dieOnCollide = false;
	bool m_isTakingDamage = false;
	bool m_isCorspe = false;
	bool m_isDead = false;
	bool m_isDestroyed = false;

	int m_numDeaths = 0;
	int m_numKills = 0;

	// Visuals 
	bool m_isRenderedLit = false;
	bool m_isRenderedRounded = false;

public:
	VertexBuffer* m_bodyVertexBuffer = nullptr;
	IndexBuffer* m_bodyIndexBuffer = nullptr;

	std::vector<unsigned int> m_actorBodyIndicies;
	std::vector<Vertex_PCU> m_actorBodyVertices;

	VertexBuffer* m_playerEyeVertexBuffer = nullptr;
	IndexBuffer* m_playerEyeIndexBuffer = nullptr;

	std::vector<unsigned int> m_playerActorEyeIndicies;
	std::vector<Vertex_PCU> m_playerActorEyeVertices;

	VertexBuffer* m_itemVertexBuffer = nullptr;
	IndexBuffer* m_itemIndexBuffer = nullptr;

	std::vector<unsigned int> m_itemIndicies;
	std::vector<Vertex_PCU> m_itemVertices;

public:
	Controller* m_owningController = nullptr;
	AIActor* m_aiController = nullptr;
	Item* m_item = nullptr;
};