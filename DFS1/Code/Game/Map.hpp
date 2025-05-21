#pragma once
#include "Game/Tile.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/Timer.hpp"
#include <vector>
#include <string>

class Controller;
class Game;
class Actor;
struct ActorUID;
class Shader;
class Texture;
class Image;
class VertexBuffer;
class IndexBuffer;

struct RaycastResult
{
	// Basic ray cast result information (required)
	bool	m_didImpact = false;
	float	m_impactDist = 0.f;
	Vec3	m_impactPos;
	Vec3	m_impactNormal;
	IntVec2 m_impactTileCoord = IntVec2::ZERO;
	Actor* m_impactedActor = nullptr;
	// Original ray cast information (optional)
	Vec3	m_rayFwdNormal;
	Vec3	m_rayStartPos;
	float	m_rayMaxLength = 1.f;
};

struct SpawnInfo
{
	SpawnInfo() {}
	explicit SpawnInfo(const tinyxml2::XMLElement* element);
	std::string m_actorType = "";

	Vec3 m_actorPosition;
	EulerAngles m_actorOrientation;
};

struct MapDefinition
{
	MapDefinition() {}
	MapDefinition(const tinyxml2::XMLElement* element);
	std::string m_name = "";
	std::string m_image = "";
	std::string m_texture = "";
	MapDefinition(Image* mapType, Texture* spriteTexture, IntVec2 cellCount);

	Image* m_mapImage = nullptr;
	Texture* m_spriteTexture = nullptr;
	IntVec2 m_cellCount = IntVec2::ZERO;

	static void InitializeMapDef();
	static MapDefinition* GetMapDefByName(const std::string& name);
	std::vector<SpawnInfo> m_spawnInfos;
};

static std::vector<MapDefinition> s_mapDefinition;

class Map
{
	SpriteSheet* m_terrainSpriteSheet = nullptr;
	MapDefinition m_definition;
	std::vector<Tile> m_tiles;
	IntVec2 m_dimensions = IntVec2::ZERO;

public:
	Map() = default;
	~Map();
	Map(Game* owner, MapDefinition definition);
	void InitializeMap();
	void AddVertsForTile(int tileIndex, const SpriteSheet& spriteSheet);
	IntVec2 GetMapDimensions();
	Vec3 GetMapWorldCenterPosition();
	Vec2 IsSpawnPointValid();
	bool IsPositionInBounds(Vec3 position, const float tolerance = 0.0f) const;
	bool AreCoordsInBounds(int x, int y) const;
	const Tile* GetTile(int x, int y) const;
	int GetTileIndex(int x, int y) const;
	IntVec2 GetTileCoordsForPos(const Vec3& position);
	IntVec2 GetRandomTilewithinRange(const IntVec2& startPos, int range) const;
	bool IsSolidTile(int tileX, int tileY) const;
	bool AreAdjacentTileNonSolid(int currentTileX, int currentTileY, int neighborX, int neighborY) const;
	bool AreAdjacentTileNonSolid(const IntVec2& currentTilePos, const IntVec2& neighborCoords) const;
	bool AreActorsCloseEnough(const Actor& actor1, const Actor& actor2, float distanceThreshold);
	void PopulateMapWithEnemyActors(const std::string& tileTypeName);
	void PopulateMapWithTimerBoxActors(const std::string& tileTypeName);
	void CheckIfPlayerHasReachedAGoalTile();
	Vec3 GetPlayerActorPosition();

	void CreateSky();
	void CreateBuffers();
	void MapRender();
	void RenderSkyBox() const;
	void RenderActors();
	void MapUpdate();
	void UpdateGameLogic();
	void UpdateActors();

	void GetMaxNumberSpawnedEnemyActors();
	Actor* GetItemActor();
	Actor* GetPlayerActor();
	ActorUID GenerateActorUID(int actorIndex);
	Actor* SpawnActor(const SpawnInfo& spawnInfo);
	Actor* SpawnPlayerActorAtRandomTileType(Controller* playerController, const std::string& tileTypeName);
	Actor* SpawnPlayerActorAtRandomTileColor(Controller* playerController, const Rgba8& tileColor);
	Actor* GetActorByUID(const ActorUID uid) const;
	
	Controller* GetPlayerController() const;
	void DebugPossessNext();
	void DeleteDestroyedActors();

	void CollideActors();
	void CollideActors(Actor* actorA, Actor* actorB);
	void CollideActorsWithMap();
	bool PushActorOutOfWalls(Actor* actor, const AABB2& tileBounds) const;

	float GetAngleToActor(Actor* referenceActor, Actor* targetActor);
	RaycastResult RaycastAll(Actor* actor, const Vec3& start, const Vec3& direction, float distance) const; 
	RaycastResult RaycastWorldXY(const Vec3& start, const Vec3& direction, float distance) const; 
	RaycastResult RaycastWorldActors(Actor* actor, const Vec3& start, const Vec3& direction, float distance) const; 

	void DebugKeys();
	void AdjustLightCommands();
	void MapShutDown();

public:
	VertexBuffer* m_tileVertexBuffer = nullptr;
	IndexBuffer* m_tileIndexBuffer = nullptr;
	Game* m_game = nullptr;
	std::vector<Vertex_PCUTBN> m_tileVertexes;
	std::vector<unsigned int> m_tileIndexes;
	std::vector<int> m_controllerList;

public:
	Vec3 m_sunDirection = Vec3::ZERO;
	float m_sunIntensity;
	float m_ambientIntensity;

	Texture* m_skyBoxTexture = nullptr;

	VertexBuffer* m_skyVertexBuffer = nullptr;
	IndexBuffer* m_skyIndexBuffer = nullptr;

	std::vector<unsigned int> m_skyIndexes;
	std::vector<Vertex_PCU> m_skyVertices;

public:
	std::vector<Actor*> m_actors;
	std::vector<Actor*> m_aiActors;
	std::vector<Tile*> m_matchingEnemyTiles;
	std::vector<Tile*> m_matchingTimerBoxTiles;
	std::vector<Actor*> m_numEnemyActors;
	int m_maxNumEnemies = 200;
	int m_maxNumTimerBoxes = 100;
	static const unsigned int MAX_ACTOR_SALT = 0x0000fffeu;
	unsigned int m_actorSalt = MAX_ACTOR_SALT;
public:
	bool m_canSeeAiPath = false;
	bool m_canSeeAiGoalPosition = false;
public:
	float m_gameTime = 45.f;
	float m_addTimeShow = 1.f;
	bool m_didAddTime = false;
	bool m_hasPlayerReachedGoal = false;
};