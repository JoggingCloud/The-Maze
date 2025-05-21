#include "Game/Game.hpp"
#include "Game/Map.hpp"
#include "Game/Player.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/Actor.hpp"
#include "Engine/Core/Clock.hpp"
#include "Game/Controller.hpp"
#include "Game/ActorDefinitions.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include <unordered_set>

struct TileDefinition;

Map::~Map()
{
	MapShutDown();
}

Map::Map(Game* owner, MapDefinition definition)
	:m_game(owner), m_definition(definition)
{
	m_sunDirection = Vec3(2.f, 1.f, -1.f);
	m_sunIntensity = 0.5f;
	m_ambientIntensity = 0.5f;
	m_dimensions = m_definition.m_mapImage->GetDimensions();
	
	InitializeMap();
	CreateSky();

	Texture* terrain_8x8 = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Terrain_8x8.png");
	m_terrainSpriteSheet = new SpriteSheet(terrain_8x8, m_definition.m_cellCount);

	for (int tileIndex = 0; tileIndex < m_tiles.size(); tileIndex++)
	{
		AddVertsForTile(tileIndex, *m_terrainSpriteSheet);
	}

	CreateBuffers();

	Actor* player;
	player = SpawnPlayerActorAtRandomTileType(m_game->m_player, "PlayerStartPoint");
	PopulateMapWithEnemyActors("EnemyStartPoint");
	PopulateMapWithTimerBoxActors("ItemSpawnPoint");
	GetMaxNumberSpawnedEnemyActors();
}

void Map::InitializeMap()
{
	int maxTiles = m_dimensions.x * m_dimensions.y;

	for (int tileIndex = 0; tileIndex < maxTiles; tileIndex++)
	{
		Rgba8 texelData = m_definition.m_mapImage->m_rgbaTexels[tileIndex];
		TileDefinition* tileDef = TileDefinition::GetTileDefinitionByColor(texelData);
		int tileY = tileIndex / m_dimensions.x;
		int tileX = tileIndex % m_dimensions.x;

		Tile newTile;
		IntVec2 newCoords(tileX, tileY);

		newTile.SetTileCoords(newCoords);
		newTile.SetTileType(tileDef);

		m_tiles.emplace_back(newTile);
	}
}

void Map::AddVertsForTile(int tileIndex, const SpriteSheet& spriteSheet)
{
	AABB3 tileBounds = m_tiles[tileIndex].GetTileBounds();

	if (m_tiles[tileIndex].GetTileDefinition() == &TileDefinition::s_definitions[0]) // Ground
	{
		int floorSpriteIndex = (m_tiles[tileIndex].GetTileDefinition()->m_floorSpriteCoords.y * m_definition.m_cellCount.x) + m_tiles[tileIndex].GetTileDefinition()->m_floorSpriteCoords.x;

		// Floor
		SpriteDefinition const& floorSpriteDef = spriteSheet.GetSpriteDef(floorSpriteIndex);

		Vec3 floorBottomLeft = tileBounds.m_mins;
		Vec3 floorBottomRight(tileBounds.m_maxs.x, tileBounds.m_mins.y, tileBounds.m_mins.z);
		Vec3 floorTopRight(tileBounds.m_maxs.x, tileBounds.m_maxs.y, tileBounds.m_mins.z);
		Vec3 floorTopLeft(tileBounds.m_mins.x, tileBounds.m_maxs.y, tileBounds.m_mins.z);

		AddVertsForQuad3D(m_tileVertexes, m_tileIndexes, floorBottomLeft, floorBottomRight, floorTopRight, floorTopLeft, Rgba8::WHITE, floorSpriteDef.GetUVs());
	}
	else if (m_tiles[tileIndex].GetTileDefinition() == &TileDefinition::s_definitions[1]) // Enemy Start Spot
	{
		int floorSpriteIndex = (m_tiles[tileIndex].GetTileDefinition()->m_floorSpriteCoords.y * m_definition.m_cellCount.x) + m_tiles[tileIndex].GetTileDefinition()->m_floorSpriteCoords.x;

		// Floor
		SpriteDefinition const& floorSpriteDef = spriteSheet.GetSpriteDef(floorSpriteIndex);

		Vec3 floorBottomLeft = tileBounds.m_mins;
		Vec3 floorBottomRight(tileBounds.m_maxs.x, tileBounds.m_mins.y, tileBounds.m_mins.z);
		Vec3 floorTopRight(tileBounds.m_maxs.x, tileBounds.m_maxs.y, tileBounds.m_mins.z);
		Vec3 floorTopLeft(tileBounds.m_mins.x, tileBounds.m_maxs.y, tileBounds.m_mins.z);

		AddVertsForQuad3D(m_tileVertexes, m_tileIndexes, floorBottomLeft, floorBottomRight, floorTopRight, floorTopLeft, Rgba8::WHITE, floorSpriteDef.GetUVs());
	}
	else if (m_tiles[tileIndex].GetTileDefinition() == &TileDefinition::s_definitions[2]) // Player Start Spot
	{
		int floorSpriteIndex = (m_tiles[tileIndex].GetTileDefinition()->m_floorSpriteCoords.y * m_definition.m_cellCount.x) + m_tiles[tileIndex].GetTileDefinition()->m_floorSpriteCoords.x;

		// Floor
		SpriteDefinition const& floorSpriteDef = spriteSheet.GetSpriteDef(floorSpriteIndex);

		Vec3 floorBottomLeft = tileBounds.m_mins;
		Vec3 floorBottomRight(tileBounds.m_maxs.x, tileBounds.m_mins.y, tileBounds.m_mins.z);
		Vec3 floorTopRight(tileBounds.m_maxs.x, tileBounds.m_maxs.y, tileBounds.m_mins.z);
		Vec3 floorTopLeft(tileBounds.m_mins.x, tileBounds.m_maxs.y, tileBounds.m_mins.z);

		AddVertsForQuad3D(m_tileVertexes, m_tileIndexes, floorBottomLeft, floorBottomRight, floorTopRight, floorTopLeft, Rgba8::WHITE, floorSpriteDef.GetUVs());
	}
	else if (m_tiles[tileIndex].GetTileDefinition() == &TileDefinition::s_definitions[3]) // Goal spot
	{
		int floorSpriteIndex = (m_tiles[tileIndex].GetTileDefinition()->m_floorSpriteCoords.y * m_definition.m_cellCount.x) + m_tiles[tileIndex].GetTileDefinition()->m_floorSpriteCoords.x;

		// Floor
		SpriteDefinition const& floorSpriteDef = spriteSheet.GetSpriteDef(floorSpriteIndex);

		Vec3 floorBottomLeft = tileBounds.m_mins;
		Vec3 floorBottomRight(tileBounds.m_maxs.x, tileBounds.m_mins.y, tileBounds.m_mins.z);
		Vec3 floorTopRight(tileBounds.m_maxs.x, tileBounds.m_maxs.y, tileBounds.m_mins.z);
		Vec3 floorTopLeft(tileBounds.m_mins.x, tileBounds.m_maxs.y, tileBounds.m_mins.z);

		AddVertsForQuad3D(m_tileVertexes, m_tileIndexes, floorBottomLeft, floorBottomRight, floorTopRight, floorTopLeft, Rgba8::WHITE, floorSpriteDef.GetUVs());
	}
	else if (m_tiles[tileIndex].GetTileDefinition() == &TileDefinition::s_definitions[4]) // Interior Walls
	{
		// Walls
		if (m_tiles[tileIndex].GetTileDefinition()->m_isSolid)
		{
			int wallSpriteIndex = (m_tiles[tileIndex].GetTileDefinition()->m_wallSpriteCoords.y * m_definition.m_cellCount.x) + m_tiles[tileIndex].GetTileDefinition()->m_wallSpriteCoords.x;

			SpriteDefinition const& wallSpriteDef = spriteSheet.GetSpriteDef(wallSpriteIndex);

			// Front Wall
			Vec3 wallBottomLeft(tileBounds.m_mins.x, tileBounds.m_mins.y, tileBounds.m_mins.z);
			Vec3 wallBottomRight(tileBounds.m_maxs.x, tileBounds.m_mins.y, tileBounds.m_mins.z);
			Vec3 wallTopRight(tileBounds.m_maxs.x, tileBounds.m_mins.y, tileBounds.m_maxs.z);
			Vec3 wallTopLeft(tileBounds.m_mins.x, tileBounds.m_mins.y, tileBounds.m_maxs.z);

			// Right Wall
			Vec3 wall2BottomLeft = wallBottomRight;
			Vec3 wall2BottomRight(tileBounds.m_maxs.x, tileBounds.m_maxs.y, tileBounds.m_mins.z);
			Vec3 wall2TopRight(tileBounds.m_maxs.x, tileBounds.m_maxs.y, tileBounds.m_maxs.z);
			Vec3 wall2TopLeft(tileBounds.m_maxs.x, tileBounds.m_mins.y, tileBounds.m_maxs.z);

			// Back Wall
			Vec3 wall3BottomLeft = wall2BottomRight;
			Vec3 wall3BottomRight(tileBounds.m_mins.x, tileBounds.m_maxs.y, tileBounds.m_mins.z); // X value should decrease
			Vec3 wall3TopRight(tileBounds.m_mins.x, tileBounds.m_maxs.y, tileBounds.m_maxs.z);
			Vec3 wall3TopLeft(tileBounds.m_maxs.x, tileBounds.m_maxs.y, tileBounds.m_maxs.z);

			// Left Wall
			Vec3 wall4BottomLeft = wall3BottomRight;
			Vec3 wall4BottomRight(tileBounds.m_mins.x, tileBounds.m_mins.y, tileBounds.m_mins.z); // This should be the same as the original bottom left
			Vec3 wall4TopRight(tileBounds.m_mins.x, tileBounds.m_mins.y, tileBounds.m_maxs.z); // This should be the same as the original top left
			Vec3 wall4TopLeft(tileBounds.m_mins.x, tileBounds.m_maxs.y, tileBounds.m_maxs.z); // Y value should decrease, moving "back" along the Y-axis

			AddVertsForQuad3D(m_tileVertexes, m_tileIndexes, wallBottomLeft, wallBottomRight, wallTopRight, wallTopLeft, m_tiles[tileIndex].GetTileColor(), wallSpriteDef.GetUVs());
			AddVertsForQuad3D(m_tileVertexes, m_tileIndexes, wall2BottomLeft, wall2BottomRight, wall2TopRight, wall2TopLeft, m_tiles[tileIndex].GetTileColor(), wallSpriteDef.GetUVs());
			AddVertsForQuad3D(m_tileVertexes, m_tileIndexes, wall3BottomLeft, wall3BottomRight, wall3TopRight, wall3TopLeft, m_tiles[tileIndex].GetTileColor(), wallSpriteDef.GetUVs());
			AddVertsForQuad3D(m_tileVertexes, m_tileIndexes, wall4BottomLeft, wall4BottomRight, wall4TopRight, wall4TopLeft, m_tiles[tileIndex].GetTileColor(), wallSpriteDef.GetUVs());

			// Second stack 

			// Front Wall
			float wallHeight = 1.f;
			Vec3 newWallBottomLeft(tileBounds.m_mins.x, tileBounds.m_mins.y, tileBounds.m_mins.z + wallHeight);
			Vec3 newWallBottomRight(tileBounds.m_maxs.x, tileBounds.m_mins.y, tileBounds.m_mins.z + wallHeight);
			Vec3 newWallTopRight(tileBounds.m_maxs.x, tileBounds.m_mins.y, tileBounds.m_maxs.z + wallHeight);
			Vec3 newWallTopLeft(tileBounds.m_mins.x, tileBounds.m_mins.y, tileBounds.m_maxs.z + wallHeight);

			// Right Wall
			Vec3 newWall2BottomLeft = newWallBottomRight;
			Vec3 newWall2BottomRight(tileBounds.m_maxs.x, tileBounds.m_maxs.y, tileBounds.m_mins.z + wallHeight);
			Vec3 newWall2TopRight(tileBounds.m_maxs.x, tileBounds.m_maxs.y, tileBounds.m_maxs.z + wallHeight);
			Vec3 newWall2TopLeft(tileBounds.m_maxs.x, tileBounds.m_mins.y, tileBounds.m_maxs.z + wallHeight);

			// Back Wall
			Vec3 newWall3BottomLeft = newWall2BottomRight;
			Vec3 newWall3BottomRight(tileBounds.m_mins.x, tileBounds.m_maxs.y, tileBounds.m_mins.z + wallHeight);
			Vec3 newWall3TopRight(tileBounds.m_mins.x, tileBounds.m_maxs.y, tileBounds.m_maxs.z + wallHeight);
			Vec3 newWall3TopLeft(tileBounds.m_maxs.x, tileBounds.m_maxs.y, tileBounds.m_maxs.z + wallHeight);

			// Left Wall
			Vec3 newWall4BottomLeft = newWall3BottomRight;
			Vec3 newWall4BottomRight(tileBounds.m_mins.x, tileBounds.m_mins.y, tileBounds.m_mins.z + wallHeight);
			Vec3 newWall4TopRight(tileBounds.m_mins.x, tileBounds.m_mins.y, tileBounds.m_maxs.z + wallHeight);
			Vec3 newWall4TopLeft(tileBounds.m_mins.x, tileBounds.m_maxs.y, tileBounds.m_maxs.z + wallHeight);

			// Top Wall
			Vec3 newWall5BottomLeft(tileBounds.m_mins.x, tileBounds.m_mins.y, tileBounds.m_maxs.z + wallHeight);
			Vec3 newWall5BottomRight(tileBounds.m_maxs.x, tileBounds.m_mins.y, tileBounds.m_maxs.z + wallHeight);
			Vec3 newWall5TopRight(tileBounds.m_maxs.x, tileBounds.m_maxs.y, tileBounds.m_maxs.z + wallHeight);
			Vec3 newWall5TopLeft(tileBounds.m_mins.x, tileBounds.m_maxs.y, tileBounds.m_maxs.z + wallHeight);

			AddVertsForQuad3D(m_tileVertexes, m_tileIndexes, newWallBottomLeft, newWallBottomRight, newWallTopRight, newWallTopLeft, m_tiles[tileIndex].GetTileColor(), wallSpriteDef.GetUVs());
			AddVertsForQuad3D(m_tileVertexes, m_tileIndexes, newWall2BottomLeft, newWall2BottomRight, newWall2TopRight, newWall2TopLeft, m_tiles[tileIndex].GetTileColor(), wallSpriteDef.GetUVs());
			AddVertsForQuad3D(m_tileVertexes, m_tileIndexes, newWall3BottomLeft, newWall3BottomRight, newWall3TopRight, newWall3TopLeft, m_tiles[tileIndex].GetTileColor(), wallSpriteDef.GetUVs());
			AddVertsForQuad3D(m_tileVertexes, m_tileIndexes, newWall4BottomLeft, newWall4BottomRight, newWall4TopRight, newWall4TopLeft, m_tiles[tileIndex].GetTileColor(), wallSpriteDef.GetUVs());
			AddVertsForQuad3D(m_tileVertexes, m_tileIndexes, newWall5BottomLeft, newWall5BottomRight, newWall5TopRight, newWall5TopLeft, m_tiles[tileIndex].GetTileColor(), wallSpriteDef.GetUVs());
		}
	}
	else if (m_tiles[tileIndex].GetTileDefinition() == &TileDefinition::s_definitions[5]) // Exterior Walls
	{
		// Walls
		if (m_tiles[tileIndex].GetTileDefinition()->m_isSolid)
		{
			int wallSpriteIndex = (m_tiles[tileIndex].GetTileDefinition()->m_wallSpriteCoords.y * m_definition.m_cellCount.x) + m_tiles[tileIndex].GetTileDefinition()->m_wallSpriteCoords.x;

			SpriteDefinition const& wallSpriteDef = spriteSheet.GetSpriteDef(wallSpriteIndex);

			// Front Wall
			Vec3 wallBottomLeft(tileBounds.m_mins.x, tileBounds.m_mins.y, tileBounds.m_mins.z);
			Vec3 wallBottomRight(tileBounds.m_maxs.x, tileBounds.m_mins.y, tileBounds.m_mins.z);
			Vec3 wallTopRight(tileBounds.m_maxs.x, tileBounds.m_mins.y, tileBounds.m_maxs.z);
			Vec3 wallTopLeft(tileBounds.m_mins.x, tileBounds.m_mins.y, tileBounds.m_maxs.z);

			// Right Wall
			Vec3 wall2BottomLeft = wallBottomRight;
			Vec3 wall2BottomRight(tileBounds.m_maxs.x, tileBounds.m_maxs.y, tileBounds.m_mins.z);
			Vec3 wall2TopRight(tileBounds.m_maxs.x, tileBounds.m_maxs.y, tileBounds.m_maxs.z);
			Vec3 wall2TopLeft(tileBounds.m_maxs.x, tileBounds.m_mins.y, tileBounds.m_maxs.z);

			// Back Wall
			Vec3 wall3BottomLeft = wall2BottomRight;
			Vec3 wall3BottomRight(tileBounds.m_mins.x, tileBounds.m_maxs.y, tileBounds.m_mins.z); // X value should decrease
			Vec3 wall3TopRight(tileBounds.m_mins.x, tileBounds.m_maxs.y, tileBounds.m_maxs.z);
			Vec3 wall3TopLeft(tileBounds.m_maxs.x, tileBounds.m_maxs.y, tileBounds.m_maxs.z);

			// Left Wall
			Vec3 wall4BottomLeft = wall3BottomRight;
			Vec3 wall4BottomRight(tileBounds.m_mins.x, tileBounds.m_mins.y, tileBounds.m_mins.z); // This should be the same as the original bottom left
			Vec3 wall4TopRight(tileBounds.m_mins.x, tileBounds.m_mins.y, tileBounds.m_maxs.z); // This should be the same as the original top left
			Vec3 wall4TopLeft(tileBounds.m_mins.x, tileBounds.m_maxs.y, tileBounds.m_maxs.z); // Y value should decrease, moving "back" along the Y-axis

			AddVertsForQuad3D(m_tileVertexes, m_tileIndexes, wallBottomLeft, wallBottomRight, wallTopRight, wallTopLeft, m_tiles[tileIndex].GetTileColor(), wallSpriteDef.GetUVs());
			AddVertsForQuad3D(m_tileVertexes, m_tileIndexes, wall2BottomLeft, wall2BottomRight, wall2TopRight, wall2TopLeft, m_tiles[tileIndex].GetTileColor(), wallSpriteDef.GetUVs());
			AddVertsForQuad3D(m_tileVertexes, m_tileIndexes, wall3BottomLeft, wall3BottomRight, wall3TopRight, wall3TopLeft, m_tiles[tileIndex].GetTileColor(), wallSpriteDef.GetUVs());
			AddVertsForQuad3D(m_tileVertexes, m_tileIndexes, wall4BottomLeft, wall4BottomRight, wall4TopRight, wall4TopLeft, m_tiles[tileIndex].GetTileColor(), wallSpriteDef.GetUVs());

			// Second stack 

			// Front Wall
			float wallHeight = 1.f;
			Vec3 newWallBottomLeft(tileBounds.m_mins.x, tileBounds.m_mins.y, tileBounds.m_mins.z + wallHeight);
			Vec3 newWallBottomRight(tileBounds.m_maxs.x, tileBounds.m_mins.y, tileBounds.m_mins.z + wallHeight);
			Vec3 newWallTopRight(tileBounds.m_maxs.x, tileBounds.m_mins.y, tileBounds.m_maxs.z + wallHeight);
			Vec3 newWallTopLeft(tileBounds.m_mins.x, tileBounds.m_mins.y, tileBounds.m_maxs.z + wallHeight);

			// Right Wall
			Vec3 newWall2BottomLeft = newWallBottomRight;
			Vec3 newWall2BottomRight(tileBounds.m_maxs.x, tileBounds.m_maxs.y, tileBounds.m_mins.z + wallHeight);
			Vec3 newWall2TopRight(tileBounds.m_maxs.x, tileBounds.m_maxs.y, tileBounds.m_maxs.z + wallHeight);
			Vec3 newWall2TopLeft(tileBounds.m_maxs.x, tileBounds.m_mins.y, tileBounds.m_maxs.z + wallHeight);

			// Back Wall
			Vec3 newWall3BottomLeft = newWall2BottomRight;
			Vec3 newWall3BottomRight(tileBounds.m_mins.x, tileBounds.m_maxs.y, tileBounds.m_mins.z + wallHeight);
			Vec3 newWall3TopRight(tileBounds.m_mins.x, tileBounds.m_maxs.y, tileBounds.m_maxs.z + wallHeight);
			Vec3 newWall3TopLeft(tileBounds.m_maxs.x, tileBounds.m_maxs.y, tileBounds.m_maxs.z + wallHeight);

			// Left Wall
			Vec3 newWall4BottomLeft = newWall3BottomRight;
			Vec3 newWall4BottomRight(tileBounds.m_mins.x, tileBounds.m_mins.y, tileBounds.m_mins.z + wallHeight);
			Vec3 newWall4TopRight(tileBounds.m_mins.x, tileBounds.m_mins.y, tileBounds.m_maxs.z + wallHeight);
			Vec3 newWall4TopLeft(tileBounds.m_mins.x, tileBounds.m_maxs.y, tileBounds.m_maxs.z + wallHeight);

			// Top Wall
			Vec3 newWall5BottomLeft(tileBounds.m_mins.x, tileBounds.m_mins.y, tileBounds.m_maxs.z + wallHeight);
			Vec3 newWall5BottomRight(tileBounds.m_maxs.x, tileBounds.m_mins.y, tileBounds.m_maxs.z + wallHeight);
			Vec3 newWall5TopRight(tileBounds.m_maxs.x, tileBounds.m_maxs.y, tileBounds.m_maxs.z + wallHeight);
			Vec3 newWall5TopLeft(tileBounds.m_mins.x, tileBounds.m_maxs.y, tileBounds.m_maxs.z + wallHeight);

			AddVertsForQuad3D(m_tileVertexes, m_tileIndexes, newWallBottomLeft, newWallBottomRight, newWallTopRight, newWallTopLeft, m_tiles[tileIndex].GetTileColor(), wallSpriteDef.GetUVs());
			AddVertsForQuad3D(m_tileVertexes, m_tileIndexes, newWall2BottomLeft, newWall2BottomRight, newWall2TopRight, newWall2TopLeft, m_tiles[tileIndex].GetTileColor(), wallSpriteDef.GetUVs());
			AddVertsForQuad3D(m_tileVertexes, m_tileIndexes, newWall3BottomLeft, newWall3BottomRight, newWall3TopRight, newWall3TopLeft, m_tiles[tileIndex].GetTileColor(), wallSpriteDef.GetUVs());
			AddVertsForQuad3D(m_tileVertexes, m_tileIndexes, newWall4BottomLeft, newWall4BottomRight, newWall4TopRight, newWall4TopLeft, m_tiles[tileIndex].GetTileColor(), wallSpriteDef.GetUVs());
			AddVertsForQuad3D(m_tileVertexes, m_tileIndexes, newWall5BottomLeft, newWall5BottomRight, newWall5TopRight, newWall5TopLeft, m_tiles[tileIndex].GetTileColor(), wallSpriteDef.GetUVs());
		}
	}
	else if (m_tiles[tileIndex].GetTileDefinition() == &TileDefinition::s_definitions[6]) // Item Spawn Spot
	{
		int floorSpriteIndex = (m_tiles[tileIndex].GetTileDefinition()->m_floorSpriteCoords.y * m_definition.m_cellCount.x) + m_tiles[tileIndex].GetTileDefinition()->m_floorSpriteCoords.x;

		// Floor
		SpriteDefinition const& floorSpriteDef = spriteSheet.GetSpriteDef(floorSpriteIndex);

		Vec3 floorBottomLeft = tileBounds.m_mins;
		Vec3 floorBottomRight(tileBounds.m_maxs.x, tileBounds.m_mins.y, tileBounds.m_mins.z);
		Vec3 floorTopRight(tileBounds.m_maxs.x, tileBounds.m_maxs.y, tileBounds.m_mins.z);
		Vec3 floorTopLeft(tileBounds.m_mins.x, tileBounds.m_maxs.y, tileBounds.m_mins.z);

		AddVertsForQuad3D(m_tileVertexes, m_tileIndexes, floorBottomLeft, floorBottomRight, floorTopRight, floorTopLeft, Rgba8::WHITE, floorSpriteDef.GetUVs());
	}
}

IntVec2 Map::GetMapDimensions()
{
	return m_dimensions;
}

Vec3 Map::GetMapWorldCenterPosition()
{
	Vec3 mapCenter = Vec3((float)m_dimensions.x * 0.5f, (float)m_dimensions.y * 0.5f, 0.f);
	return mapCenter;
}

Vec2 Map::IsSpawnPointValid()
{
	int x = g_rng.SRollRandomIntInRange(1, m_dimensions.x - 2);
	int y = g_rng.SRollRandomIntInRange(1, m_dimensions.y - 2);

	while (IsSolidTile(x, y))
	{
		x = g_rng.SRollRandomIntInRange(1, m_dimensions.x - 2);
		y = g_rng.SRollRandomIntInRange(1, m_dimensions.y - 2);
	}

	return Vec2(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f);
}

bool Map::IsPositionInBounds(Vec3 position, const float tolerance) const
{
	Vec2 bottomLeft = Vec2::ZERO;
	Vec2 topRight = Vec2(static_cast<float>(m_dimensions.x), static_cast<float>(m_dimensions.y));
	topRight += Vec2(tolerance, tolerance);

	return position.x >= bottomLeft.x - tolerance && position.x <= topRight.x && position.y >= bottomLeft.y - tolerance && position.y <= topRight.y;
}

bool Map::AreCoordsInBounds(int x, int y) const
{
	return x >= 0 && x < m_dimensions.x && y >= 0 && y < m_dimensions.y;
}

const Tile* Map::GetTile(int x, int y) const
{
	if (AreCoordsInBounds(x, y))
	{
		int tileIndex = x + (y * m_dimensions.x);
		return &m_tiles[tileIndex];
	}
	return nullptr;
}

int Map::GetTileIndex(int x, int y) const
{
	return x + (y * m_dimensions.x);
}

IntVec2 Map::GetTileCoordsForPos(const Vec3& position)
{
	return IntVec2(RoundDownToInt(position.x), RoundDownToInt(position.y));
}

IntVec2 Map::GetRandomTilewithinRange(const IntVec2& startPos, int range) const
{
	IntVec2 randomTile;
	IntVec2 randomIntVec2 = g_rng.SRollRandomIntVec2InRange(-range, range);
	randomTile = startPos + randomIntVec2;

	return randomTile;
}

bool Map::IsSolidTile(int tileX, int tileY) const
{
	if (!AreCoordsInBounds(tileX, tileY))
	{
		return false;
	}

	const Tile* tile = GetTile(tileX, tileY);
	return tile->GetTileDefinition()->m_isSolid;
}

bool Map::AreAdjacentTileNonSolid(const IntVec2& currentTilePos, const IntVec2& neighborCoords) const
{
	IntVec2 direction = neighborCoords - currentTilePos;

	if (direction.x != 0 && direction.y != 0) 
	{ 
		IntVec2 adjacent1 = currentTilePos + IntVec2(direction.x, 0);
		IntVec2 adjacent2 = currentTilePos + IntVec2(0, direction.y);
		return !IsSolidTile(adjacent1.x, adjacent1.y) && !IsSolidTile(adjacent2.x, adjacent2.y);
	}
	return true;
}

bool Map::AreAdjacentTileNonSolid(int currentTileX, int currentTileY, int neighborX, int neighborY) const
{
	int directionX = neighborX - currentTileX;
	int directionY = neighborY - currentTileY;

	if (directionX != 0 && directionY != 0)
	{
		int adjacent1X = currentTileX + directionX;
		int adjacent1Y = currentTileY;
		
		int adjacent2X = currentTileX;
		int adjacent2Y = currentTileY + directionY;
		
		return !IsSolidTile(adjacent1X, adjacent1Y) && !IsSolidTile(adjacent2X, adjacent2Y);
	}

	return true;
}

bool Map::AreActorsCloseEnough(const Actor& actor1, const Actor& actor2, float distanceThreshold)
{
	float distanceSq = (actor1.m_position - actor2.m_position).GetLengthSquared();
	return distanceSq <= distanceThreshold * distanceThreshold;
}

void Map::PopulateMapWithEnemyActors(const std::string& tileTypeName)
{
	for (int i = 0; i < m_maxNumEnemies; i++) 
	{
		for (int y = 0; y < m_dimensions.y; y++) 
		{
			for (int x = 0; x < m_dimensions.x; x++) 
			{
				Tile& currentTile = m_tiles[GetTileIndex(x, y)];
				if (currentTile.GetTileName() == tileTypeName) 
				{
					m_matchingEnemyTiles.emplace_back(&currentTile);
				}
			}
		}

		if (m_matchingEnemyTiles.empty()) 
		{
			return;
		}

		std::unordered_set<int> usedSpawnIndices;
		bool spawned = false;

		while (!spawned && usedSpawnIndices.size() < m_matchingEnemyTiles.size()) 
		{
			int spawnIndex = g_rng.SRollRandomIntInRange(0, (int)m_matchingEnemyTiles.size() - 1);

			if (usedSpawnIndices.find(spawnIndex) != usedSpawnIndices.end()) 
			{
				continue; // This spawn index is already occupied by an actor so try another
			}

			usedSpawnIndices.insert(spawnIndex);
			Tile* chosenTile = m_matchingEnemyTiles[spawnIndex];

			// Check if any actor is already at the chosen tile position
			bool positionOccupied = false;
			for (int actor = 0; actor < m_actors.size(); actor++) 
			{
				if (m_actors[actor] != nullptr && m_actors[actor]->GetActorPosition() == chosenTile->GetTilePosition()) 
				{
					positionOccupied = true;
					break;
				}
			}

			if (!positionOccupied) 
			{
				ActorDefinition* enemyDef = ActorDefinition::GetActorDefByName("Enemy");
				if (!enemyDef) 
				{
					ERROR_AND_DIE("Failed to get the enemy actor definition");
				}

				SpawnInfo spawnInfo;
				spawnInfo.m_actorType = enemyDef->m_name;
				spawnInfo.m_actorPosition = chosenTile->GetTilePosition();
				spawnInfo.m_actorOrientation = EulerAngles::ZERO;
				
				SpawnActor(spawnInfo);
				spawned = true;
			}
		}

		if (usedSpawnIndices.size() == m_matchingEnemyTiles.size()) 
		{
			break; // No more unique spawn points available
		}
	}
}

void Map::PopulateMapWithTimerBoxActors(const std::string& tileTypeName)
{
	for (int i = 0; i < m_maxNumTimerBoxes; i++)
	{
		for (int y = 0; y < m_dimensions.y; y++)
		{
			for (int x = 0; x < m_dimensions.x; x++)
			{
				Tile& currentTile = m_tiles[GetTileIndex(x, y)];
				if (currentTile.GetTileName() == tileTypeName)
				{
					m_matchingTimerBoxTiles.emplace_back(&currentTile);
				}
			}
		}

		if (m_matchingTimerBoxTiles.empty())
		{
			return;
		}

		std::unordered_set<int> usedSpawnIndices;
		bool spawned = false;

		while (!spawned && usedSpawnIndices.size() < m_matchingTimerBoxTiles.size())
		{
			int spawnIndex = g_rng.SRollRandomIntInRange(0, (int)m_matchingTimerBoxTiles.size() - 1);

			if (usedSpawnIndices.find(spawnIndex) != usedSpawnIndices.end())
			{
				continue; // This spawn index is already occupied by an actor so try another
			}

			usedSpawnIndices.insert(spawnIndex);
			Tile* chosenTile = m_matchingTimerBoxTiles[spawnIndex];

			// Check if any actor is already at the chosen tile position
			bool positionOccupied = false;
			for (int actor = 0; actor < m_actors.size(); actor++)
			{
				if (m_actors[actor] != nullptr && m_actors[actor]->GetActorPosition() == chosenTile->GetTilePosition())
				{
					positionOccupied = true;
					break;
				}
			}

			if (!positionOccupied)
			{
				ActorDefinition* itemDef = ActorDefinition::GetActorDefByName("ItemBox");
				if (!itemDef)
				{
					ERROR_AND_DIE("Failed to get the item actor definition");
				}

				SpawnInfo spawnInfo;
				spawnInfo.m_actorType = itemDef->m_name;
				spawnInfo.m_actorPosition = chosenTile->GetTilePosition();
				spawnInfo.m_actorOrientation = EulerAngles::ZERO;

				SpawnActor(spawnInfo);
				spawned = true;
			}
		}

		if (usedSpawnIndices.size() == m_matchingTimerBoxTiles.size())
		{
			break; // No more unique spawn points available
		}
	}
}

void Map::CheckIfPlayerHasReachedAGoalTile()
{
	std::vector<Tile*> matchingTiles;
	for (int y = 0; y < m_dimensions.y; y++)
	{
		for (int x = 0; x < m_dimensions.x; x++)
		{
			Tile& currentTile = m_tiles[GetTileIndex(x, y)];
			if (currentTile.GetTileName() == "EndPoint")
			{
				matchingTiles.emplace_back(&currentTile);
			}
		}
	}

	Vec3 playerPos = GetPlayerActorPosition();
	
	for (int i = 0; i < matchingTiles.size(); i++)
	{
		if (matchingTiles[i]->GetTileBounds().IsPointInside(playerPos))
		{
			m_hasPlayerReachedGoal = true;
		}
	}
}

void Map::MapRender()
{
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetRasterizerState(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->BindTexture(0, &m_terrainSpriteSheet->GetTexture());
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->SetLightConstants(m_sunDirection, m_sunIntensity, m_ambientIntensity, m_game->m_player->m_position, 0.f, 0.f, 0.f, 0.f, LightingDebug());
	g_theRenderer->DrawVertexBufferIndex(m_tileVertexBuffer, m_tileIndexBuffer, VertexType::Vertex_PCUTBN, static_cast<int>(m_tileIndexes.size()));

	RenderSkyBox();
	RenderActors();
}

void Map::RenderSkyBox() const
{
	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetRasterizerState(RasterizerMode::SOLID_CULL_NONE);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->BindTexture(0, m_skyBoxTexture);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->DrawVertexBufferIndex(m_skyVertexBuffer, m_skyIndexBuffer, VertexType::Vertex_PCU, static_cast<int>(m_skyIndexes.size()));
}

Vec3 Map::GetPlayerActorPosition()
{
	if (m_game->m_player->GetActor())
	{
		return m_game->m_player->GetActor()->m_position;
	}
	return Vec3::ZERO;
}

void Map::CreateSky()
{
	m_skyBoxTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/skybox.png");
	AddVertsForZSphere(m_skyVertices, m_skyIndexes, Vec3::ZERO, 800.f, Rgba8::WHITE, AABB2::ZERO_TO_ONE, 128);
}

void Map::CreateBuffers()
{
	size_t vertexBufferSize = sizeof(Vertex_PCUTBN) * m_tileVertexes.size();
	m_tileVertexBuffer = g_theRenderer->CreateVertexBuffer(vertexBufferSize);
	g_theRenderer->CopyCPUToGPU(m_tileVertexes.data(), vertexBufferSize, m_tileVertexBuffer);

	size_t indexBufferSize = sizeof(unsigned int) * m_tileIndexes.size();
	m_tileIndexBuffer = g_theRenderer->CreateIndexBuffer(indexBufferSize);
	g_theRenderer->CopyCPUToGPU(m_tileIndexes.data(), indexBufferSize, m_tileIndexBuffer);

	m_skyVertexBuffer = g_theRenderer->CreateVertexBuffer(m_skyVertices.size());
	g_theRenderer->CopyCPUToGPU(m_skyVertices.data(), m_skyVertices.size() * sizeof(Vertex_PCU), m_skyVertexBuffer);

	m_skyIndexBuffer = g_theRenderer->CreateIndexBuffer(m_skyIndexes.size());
	g_theRenderer->CopyCPUToGPU(m_skyIndexes.data(), m_skyIndexes.size() * sizeof(unsigned int), m_skyIndexBuffer);
}

void Map::RenderActors()
{
	for (int index = 0; index < m_actors.size(); index++)
	{
		if (m_actors[index] == nullptr)
		{
			continue;
		}
		m_actors[index]->Render();
	}
}

void Map::MapUpdate()
{
	UpdateGameLogic();
	UpdateActors();
	CollideActors();
	CollideActorsWithMap();
	DebugKeys();
	//AdjustLightCommands();
	DeleteDestroyedActors();
}

void Map::UpdateGameLogic()
{
	CheckIfPlayerHasReachedAGoalTile();
	if (m_hasPlayerReachedGoal)
	{
		return;
	}
	m_gameTime -= m_game->m_clock->GetDeltaSeconds();
	if (!m_hasPlayerReachedGoal && m_gameTime <= 0.f)
	{
		m_gameTime = 0.f;
	}
}

void Map::UpdateActors()
{
	for (int index = 0; index < m_actors.size(); index++)
	{
		if (m_actors[index] == nullptr)
		{
			continue;
		}
		m_actors[index]->Update();
	}
}

void Map::GetMaxNumberSpawnedEnemyActors()
{
	for (int i = 0; i < m_actors.size(); i++)
	{
		if (m_actors[i] != nullptr)
		{
			if (m_actors[i]->m_isAI)
			{
				m_numEnemyActors.emplace_back(m_actors[i]);
			}
		}
	}
}

Actor* Map::GetItemActor()
{
	for (int actor = 0; actor < m_actors.size(); actor++)
	{
		if (m_actors[actor] != nullptr)
		{
			if (m_actors[actor]->GetType() == ActorType::ACTOR_ITEMBOX)
			{
				return m_actors[actor];
			}
		}
	}
	return nullptr;
}

Actor* Map::GetPlayerActor()
{
	for (int actor = 0; actor < m_actors.size(); actor++)
	{
		if (m_actors[actor] != nullptr)
		{
			if (m_actors[actor]->GetType() == ActorType::ACTOR_PLAYER)
			{
				return m_actors[actor];
			}
		}
	}
	return nullptr;
}

ActorUID Map::GenerateActorUID(int actorIndex)
{
	ActorUID uid = ActorUID(m_actorSalt++, actorIndex);
	return uid;
}


void Map::CollideActors()
{
	for (int actorIndexA = 0; actorIndexA < m_actors.size(); actorIndexA++)
	{
		Actor* actorA = m_actors[actorIndexA];

		for (int actorIndexB = actorIndexA + 1; actorIndexB < m_actors.size(); actorIndexB++)
		{
			Actor* actorB = m_actors[actorIndexB];

			if (!actorA || !actorB) continue;

			if ((actorA->m_type == ActorType::ACTOR_ITEMBOX && actorB->m_type == ActorType::ACTOR_ENEMY) || (actorA->m_type == ActorType::ACTOR_ENEMY && actorB->m_type == ActorType::ACTOR_ITEMBOX)) continue;

			CollideActors(actorA, actorB);
		}
	}
}

void Map::CollideActors(Actor* actorA, Actor* actorB)
{
	FloatRange actorARange(actorA->m_position.z, actorA->m_position.z + actorA->m_physicsHeight);
	FloatRange actorBRange(actorB->m_position.z, actorB->m_position.z + actorB->m_physicsHeight);

	if (actorARange.IsOverlapingWith(actorBRange))
	{
		if (DoDiscsOverlap(Vec2(actorA->m_position.x, actorA->m_position.y), actorA->m_physicsRadius, Vec2(actorB->m_position.x, actorB->m_position.y), actorB->m_physicsRadius))
		{
			Vec2 actorAPosXY = Vec2(actorA->m_position.x, actorA->m_position.y);
			Vec2 actorBPosXY = Vec2(actorB->m_position.x, actorB->m_position.y);

			if (actorA->m_canCollideWithActors && actorB->m_canCollideWithActors)
			{
				PushDiscOutOfEachOther2D(actorAPosXY, actorA->m_physicsRadius, actorBPosXY, actorB->m_physicsRadius);

				if (actorA->m_ownerUID.IsValid() || actorB->m_ownerUID.IsValid())
				{
					if (actorA->m_uid == actorB->m_ownerUID || actorB->m_uid == actorA->m_ownerUID || actorA->m_ownerUID == actorB->m_ownerUID)
					{
						return;
					}
				}

				actorA->m_position = Vec3(actorAPosXY.x, actorAPosXY.y, actorA->m_position.z);
				actorB->m_position = Vec3(actorBPosXY.x, actorBPosXY.y, actorB->m_position.z);

				if (actorA->IsPlayer() && actorB->IsItem())
				{
					if (actorB->IsAlive())
					{
						actorB->OnCollide();
					}
				}
			}
		}
	}
}

void Map::CollideActorsWithMap()
{
	for (int actor = 0; actor < m_actors.size(); actor++)
	{
		if (m_actors[actor] == nullptr)
		{
			continue;
		}

		Vec2 actorPosition = Vec2(m_actors[actor]->m_position.x, m_actors[actor]->m_position.y);

		int tileX = RoundDownToInt(actorPosition.x);
		int tileY = RoundDownToInt(actorPosition.y);

		AABB2 tileBounds(Vec2(static_cast<float>(tileX), static_cast<float>(tileY)), Vec2(static_cast<float>(tileX + 1), static_cast<float>(tileY + 1)));
		bool didCollide = false;
		if (m_actors[actor]->m_canCollideWithWorld)
		{
			if (PushActorOutOfWalls(m_actors[actor], tileBounds))
			{
				didCollide |= true;
			}
		}
	}
}

bool  Map::PushActorOutOfWalls(Actor* actor, const AABB2& tileBounds) const
{
	Vec2 actorPosition2D(actor->m_position.x, actor->m_position.y);
	int actorTileX = static_cast<int>(tileBounds.m_mins.x);
	int actorTileY = static_cast<int>(tileBounds.m_mins.y);
	bool didCollide = false;

	if (IsSolidTile(actorTileX, actorTileY + 1))
	{
		AABB2 northTileBounds(Vec2(static_cast<float>(actorTileX), static_cast<float>(actorTileY + 1)), Vec2(static_cast<float>(actorTileX + 1), static_cast<float>(actorTileY + 2)));
		didCollide |= PushDiscOutOfFixedAABB2D(actorPosition2D, actor->m_physicsRadius, northTileBounds);
	}
	if (IsSolidTile(actorTileX, actorTileY - 1))
	{
		AABB2 southTileBounds(Vec2(static_cast<float>(actorTileX), static_cast<float>(actorTileY - 1)), Vec2(static_cast<float>(actorTileX + 1), static_cast<float>(actorTileY)));
		didCollide |= PushDiscOutOfFixedAABB2D(actorPosition2D, actor->m_physicsRadius, southTileBounds);
	}
	if (IsSolidTile(actorTileX + 1, actorTileY))
	{
		AABB2 eastTileBounds(Vec2(static_cast<float>(actorTileX + 1), static_cast<float>(actorTileY)), Vec2(static_cast<float>(actorTileX + 2), static_cast<float>(actorTileY + 1)));
		didCollide |= PushDiscOutOfFixedAABB2D(actorPosition2D, actor->m_physicsRadius, eastTileBounds);
	}
	if (IsSolidTile(actorTileX - 1, actorTileY))
	{
		AABB2 westTileBounds(Vec2(static_cast<float>(actorTileX - 1), static_cast<float>(actorTileY)), Vec2(static_cast<float>(actorTileX), static_cast<float>(actorTileY + 1)));
		didCollide |= PushDiscOutOfFixedAABB2D(actorPosition2D, actor->m_physicsRadius, westTileBounds);
	}
	if (didCollide)
	{
		actor->m_position.x = actorPosition2D.x;
		actor->m_position.y = actorPosition2D.y;
	}

	return didCollide;
}

float Map::GetAngleToActor(Actor* referenceActor, Actor* targetActor)
{
	Vec3 toTarget = (targetActor->m_position - referenceActor->m_position).GetNormalized();
	Vec3 forward = referenceActor->m_orientation.GetForwardVector();

	// Use Atan2Degrees to get the angle in degrees directly
	float targetAngle = Atan2Degrees(toTarget.y, toTarget.x);
	float forwardAngle = Atan2Degrees(forward.y, forward.x);

	// Get the shortest angular difference
	float angleDifference = GetShortestAngularDispDegrees(forwardAngle, targetAngle);

	return fabsf(angleDifference);
}

RaycastResult Map::RaycastAll(Actor* actor, const Vec3& start, const Vec3& direction, float distance) const
{
	RaycastResult closestResult;
	closestResult.m_impactDist = distance;

	RaycastResult xyResult = RaycastWorldXY(start, direction, distance);
	if (xyResult.m_didImpact && xyResult.m_impactDist < closestResult.m_impactDist)
	{
		closestResult = xyResult;
	}

	RaycastResult actorsResult = RaycastWorldActors(actor, start, direction, distance);
	if (actorsResult.m_didImpact && actorsResult.m_impactDist < closestResult.m_impactDist)
	{
		closestResult = actorsResult;
	}

	return closestResult;
}

RaycastResult Map::RaycastWorldXY(const Vec3& start, const Vec3& direction, float distance) const
{
	RaycastResult result;
	result.m_rayStartPos = start;
	result.m_rayFwdNormal = direction;
	result.m_rayMaxLength = distance;

	IntVec2 currentTileXY = IntVec2(RoundDownToInt(start.x), RoundDownToInt(start.y));
	if (IsSolidTile(currentTileXY.x, currentTileXY.y) && AreCoordsInBounds(currentTileXY.x, currentTileXY.y))
	{
		if (!(start.z > 1.f || start.z < 0.f))
		{
			result.m_didImpact = true;
			result.m_impactPos = start;
			result.m_impactDist = 0.f;
			result.m_impactNormal = direction * -1.f;
			return result;
		}
	}

	// X
	float fwdDistPerXCrossing = 1.f / abs(direction.x);
	int tileStepDirectionX = (direction.x > 0) ? 1 : -1;
	float xAtFirstXCrossing = currentTileXY.x + (tileStepDirectionX + 1) / 2.f;
	float xDistToFirstXCrossing = xAtFirstXCrossing - start.x;
	float fwdDistAtNextXCrossing = fabsf(xDistToFirstXCrossing) * fwdDistPerXCrossing;

	// Y
	float fwdDistPerYCrossing = 1.f / abs(direction.y);
	int tileStepDirectionY = (direction.y > 0) ? 1 : -1;
	float yAtFirstYCrossing = currentTileXY.y + (tileStepDirectionY + 1) / 2.f;
	float yDistToFirstYCrossing = yAtFirstYCrossing - start.y;
	float fwdDistAtNextYCrossing = fabsf(yDistToFirstYCrossing) * fwdDistPerYCrossing;

	for (;;)
	{
		if (fwdDistAtNextXCrossing < fwdDistAtNextYCrossing)
		{
			if (fwdDistAtNextXCrossing > distance)
			{
				result.m_didImpact = false;
				return result;
			}
			currentTileXY.x += tileStepDirectionX;
			if (IsSolidTile(currentTileXY.x, currentTileXY.y) && AreCoordsInBounds(currentTileXY.x, currentTileXY.y))
			{
				result.m_impactTileCoord = IntVec2(currentTileXY.x, currentTileXY.y);
				result.m_didImpact = true;
				result.m_impactDist = fwdDistAtNextXCrossing;
				result.m_impactPos = start + direction * result.m_impactDist;
				if (result.m_impactPos.z > 1.f || result.m_impactPos.z < 0)
				{
					result.m_didImpact = false;
					fwdDistAtNextXCrossing += fwdDistPerXCrossing;
					continue;
				}
				result.m_impactNormal = Vec3(static_cast<float>(-tileStepDirectionX), 0.f, 0.f);
				return result;
			}
			else
			{
				fwdDistAtNextXCrossing += fwdDistPerXCrossing;
			}
		}
		else
		{
			if (fwdDistAtNextYCrossing > distance)
			{
				result.m_didImpact = false;
				return result;
			}
			currentTileXY.y += tileStepDirectionY;
			if (IsSolidTile(currentTileXY.x, currentTileXY.y) && AreCoordsInBounds(currentTileXY.x, currentTileXY.y))
			{
				result.m_impactTileCoord = IntVec2(currentTileXY.x, currentTileXY.y);
				result.m_didImpact = true;
				result.m_impactDist = fwdDistAtNextYCrossing;
				result.m_impactPos = start + direction * result.m_impactDist;
				if (result.m_impactPos.z > 1.f || result.m_impactPos.z < 0)
				{
					result.m_didImpact = false;
					fwdDistAtNextYCrossing += fwdDistPerYCrossing;
					continue;
				}
				result.m_impactNormal = Vec3(0.f, static_cast<float>(-tileStepDirectionY), 0.f);
				return result;
			}
			else
			{
				fwdDistAtNextYCrossing += fwdDistPerYCrossing;
			}
		}
	}
}

RaycastResult Map::RaycastWorldActors(Actor* actor, const Vec3& start, const Vec3& direction, float distance) const
{
	RaycastResult closestResult;
	closestResult.m_impactDist = distance;

	for (int numActor = 0; numActor < m_actors.size(); numActor++)
	{
		if (m_actors[numActor] == nullptr)
		{
			continue;
		}
		if (m_actors[numActor] == actor)
		{
			continue;
		}
		Vec2 actorXYPos = Vec2(m_actors[numActor]->m_position.x, m_actors[numActor]->m_position.y);

		RaycastResult3D result3D = RaycastVsZCylinder(start, direction, distance, actorXYPos, FloatRange(m_actors[numActor]->m_position.z, m_actors[numActor]->m_position.z + m_actors[numActor]->m_physicsHeight), m_actors[numActor]->m_physicsRadius);
		if (result3D.m_didImpact && result3D.m_impactDist < closestResult.m_impactDist)
		{
			closestResult.m_didImpact = result3D.m_didImpact;
			closestResult.m_impactDist = result3D.m_impactDist;
			closestResult.m_impactPos = result3D.m_impactPos;
			closestResult.m_impactNormal = result3D.m_impactNormal;
			closestResult.m_impactedActor = m_actors[numActor];
		}
	}

	return closestResult;
}

Actor* Map::SpawnActor(const SpawnInfo& spawnInfo)
{
	for (int i = 0; i < m_actors.size(); i++)
	{
		if (m_actors[i] == nullptr)
		{
			ActorUID uid = GenerateActorUID(i);

			Actor* newActor = new Actor(this, spawnInfo, uid);
			m_actors[i] = newActor;
			return newActor;
		}
	}
	int index = static_cast<int>(m_actors.size());
	ActorUID uid = GenerateActorUID(index);

	Actor* newActor = new Actor(this, spawnInfo, uid);

	m_actors.emplace_back(newActor);
	return newActor;
}

Actor* Map::SpawnPlayerActorAtRandomTileType(Controller* playerController, const std::string& tileTypeName)
{
	ActorDefinition* playerActorDef = ActorDefinition::GetActorDefByName("Player");
	if (!playerActorDef)
	{
		ERROR_AND_DIE("Unable to get the player actor definition");
	}

	std::vector<Tile*> matchingTiles;
	for (int y = 0; y < m_dimensions.y; y++)
	{
		for (int x = 0; x < m_dimensions.x; x++)
		{
			Tile& currentTile = m_tiles[GetTileIndex(x, y)];
			if (currentTile.GetTileName() == tileTypeName)
			{
				matchingTiles.emplace_back(&currentTile);
			}
		}
	}

	if (matchingTiles.empty())
	{
		return nullptr;
	}

	int spawnIndex = g_rng.SRollRandomIntInRange(0, (int)matchingTiles.size() - 1);
	Tile* chosenTile = matchingTiles[spawnIndex];

	SpawnInfo spawnInfo;
	spawnInfo.m_actorType = playerActorDef->m_name;
	spawnInfo.m_actorPosition = chosenTile->GetTilePosition();
	spawnInfo.m_actorOrientation = EulerAngles(90.f, 0.f, 0.f);

	ActorUID uid = GenerateActorUID(GetTileIndex((int)chosenTile->GetTilePosition().x, (int)chosenTile->GetTilePosition().y));
	Actor* playerActor = new Actor(this, spawnInfo, uid);
	playerActor->m_owningController = playerController;
	playerController->m_actorUID = uid;
	m_actors.emplace_back(playerActor);
	
	return playerActor;
}

Actor* Map::SpawnPlayerActorAtRandomTileColor(Controller* playerController, const Rgba8& tileColor)
{
	ActorDefinition* playerActorDef = ActorDefinition::GetActorDefByName("Player");
	if (!playerActorDef)
	{
		ERROR_AND_DIE("Unable to get the player actor definition");
	}

	std::vector<Tile*> matchingTiles;
	for (int y = 0; y < m_dimensions.y; y++)
	{
		for (int x = 0; x < m_dimensions.x; x++)
		{
			Tile& currentTile = m_tiles[GetTileIndex(x, y)];
			if (currentTile.GetTileColor() == tileColor)
			{
				matchingTiles.emplace_back(&currentTile);
			}
		}
	}

	if (matchingTiles.empty())
	{
		return nullptr;
	}

	int spawnIndex = g_rng.SRollRandomIntInRange(0, (int)matchingTiles.size() - 1);
	Tile* chosenTile = matchingTiles[spawnIndex];

	SpawnInfo spawnInfo;
	spawnInfo.m_actorType = playerActorDef->m_name;
	spawnInfo.m_actorPosition = chosenTile->GetTilePosition();
	spawnInfo.m_actorOrientation = EulerAngles(90.f, 0.f, 0.f);

	ActorUID uid = GenerateActorUID(GetTileIndex((int)chosenTile->GetTilePosition().x, (int)chosenTile->GetTilePosition().y));
	Actor* playerActor = new Actor(this, spawnInfo, uid);
	playerActor->m_owningController = playerController;
	playerController->m_actorUID = uid;
	m_actors.emplace_back(playerActor);

	return playerActor;
}

Controller* Map::GetPlayerController() const
{
	if (m_game->m_player != nullptr)
	{
		return m_game->m_player;
	}
	return nullptr;
}

Actor* Map::GetActorByUID(const ActorUID uid) const
{
	for (Actor* actor : m_actors)
	{
		if (actor && actor->GetUID() == uid)
		{
			return actor;
		}
	}
	return nullptr;
}

void Map::DebugPossessNext()
{
	for (int i = m_game->m_player->GetActor()->GetUID().GetIndex() + 1; i < m_actors.size(); i++)
	{
		if (m_actors[i] != nullptr && m_actors[i]->m_canBePossessed)
		{
			m_game->m_player->Possess(m_actors[i]);
			return;
		}
	}
}

void Map::DeleteDestroyedActors()
{
	for (int i = 0; i < (int)m_actors.size(); i++)
	{
		if (m_actors[i] == nullptr)
		{
			continue;
		}

		if (m_actors[i]->m_isDestroyed)
		{
			delete m_actors[i];
			m_actors[i] = nullptr;
		}
	}
}

void Map::DebugKeys()
{
	if (m_game->m_player->m_isShowingDebugOptions)
	{
		if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
		{
			m_canSeeAiPath = !m_canSeeAiPath;
		}

		if (g_theInput->WasKeyJustPressed(KEYCODE_F3))
		{
			m_canSeeAiGoalPosition = !m_canSeeAiGoalPosition;
		}
	}
}

void Map::AdjustLightCommands()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_F2))
	{
		m_sunDirection.x -= 1.f;
		std::string sunDirection = Stringf("Sun Direction X: %.2f", m_sunDirection.x);
		DebugAddMessage(sunDirection, 2.f, Rgba8::WHITE, Rgba8::WHITE);
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F3))
	{
		m_sunDirection.x += 1.f;
		std::string sunDirection = Stringf("Sun Direction X: %.2f", m_sunDirection.x);
		DebugAddMessage(sunDirection, 2.f, Rgba8::WHITE, Rgba8::WHITE);
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F4))
	{
		m_sunDirection.y -= 1.f;
		std::string sunDirection = Stringf("Sun Direction Y: %.2f", m_sunDirection.y);
		DebugAddMessage(sunDirection, 2.f, Rgba8::WHITE, Rgba8::WHITE);
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F5))
	{
		m_sunDirection.y += 1.f;
		std::string sunDirection = Stringf("Sun Direction Y: %.2f", m_sunDirection.y);
		DebugAddMessage(sunDirection, 2.f, Rgba8::WHITE, Rgba8::WHITE);
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F6))
	{
		m_sunIntensity -= 0.05f;
		std::string sunIntensity = Stringf("Sun Intensity: %.2f", m_sunIntensity);
		DebugAddMessage(sunIntensity, 2.f, Rgba8::WHITE, Rgba8::WHITE);
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F7))
	{
		m_sunIntensity += 0.05f;
		std::string sunIntensity = Stringf("Sun Intensity: %.2f", m_sunIntensity);
		DebugAddMessage(sunIntensity, 2.f, Rgba8::WHITE, Rgba8::WHITE);
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
	{
		m_ambientIntensity -= 0.05f;
		std::string ambientIntensity = Stringf("Ambient Intensity: %.2f", m_ambientIntensity);
		DebugAddMessage(ambientIntensity, 2.f, Rgba8::WHITE, Rgba8::WHITE);
	}
	if (g_theInput->IsKeyDown(KEYCODE_F9))
	{
		m_ambientIntensity += 0.05f;
		std::string ambientIntensity = Stringf("Ambient Intensity: %.2f", m_ambientIntensity);
		DebugAddMessage(ambientIntensity, 2.f, Rgba8::WHITE, Rgba8::WHITE);
	}
	m_sunIntensity = GetClampedZeroToOne(m_sunIntensity);
	m_ambientIntensity = GetClampedZeroToOne(m_ambientIntensity);
}

void Map::MapShutDown()
{
	m_tileVertexes.clear();
	m_tileIndexes.clear();

	m_skyVertices.clear();
	m_skyIndexes.clear();

	SafeDelete(m_tileVertexBuffer);
	SafeDelete(m_tileIndexBuffer);
	
	SafeDelete(m_skyVertexBuffer);
	SafeDelete(m_skyIndexBuffer);

	s_mapDefinition.clear();

	SafeDelete(m_actors);

	m_actors.clear();
	m_matchingEnemyTiles.clear();
	m_matchingTimerBoxTiles.clear();
}

void MapDefinition::InitializeMapDef()
{
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("Data/Definitions/MapDefinitions.xml") != tinyxml2::XML_SUCCESS)
	{
		// Handle error loading XML file
		return;
	}

	const tinyxml2::XMLElement* root = doc.FirstChildElement("MapDefinitions");
	if (!root)
	{
		// Handle missing root element
		ERROR_AND_DIE("Failed to get the root in the map definition xml file");
		return;
	}

	// Only initialize if it hasn't been initialized already
	if (s_mapDefinition.empty())
	{
		// Load & iterate over MapDefinition elements
		for (const tinyxml2::XMLElement* mapElement = root->FirstChildElement("MapDefinition"); mapElement; mapElement = mapElement->NextSiblingElement("MapDefinition"))
		{
			MapDefinition newMapDef(mapElement);

			// Get the container element for spawn info within the map def
			const tinyxml2::XMLElement* spawnInfosElement = mapElement->FirstChildElement("SpawnInfos");
			if (spawnInfosElement)
			{
				// Iterate over SpawnInfo elements within the SpawnInfos container
				for (const tinyxml2::XMLElement* spawnElement = spawnInfosElement->FirstChildElement("SpawnInfo"); spawnElement; spawnElement = spawnElement->NextSiblingElement("SpawnInfo"))
				{
					SpawnInfo newSpawnInfo(spawnElement);
					newMapDef.m_spawnInfos.push_back(newSpawnInfo);
				}
			}
			s_mapDefinition.emplace_back(newMapDef);
		}
	}
}

MapDefinition* MapDefinition::GetMapDefByName(const std::string& name)
{
	for (MapDefinition& mapDef : s_mapDefinition)
	{
		if (mapDef.m_name == name)
		{
			return &mapDef;
		}
	}
	return nullptr; // Map definition with the given name was not found
}

MapDefinition::MapDefinition(const tinyxml2::XMLElement* element)
{
	// Parse attributes from XML element
	m_name = ParseXmlAttribute(*element, "name", std::string());
	m_image = ParseXmlAttribute(*element, "image", std::string());
	m_texture = ParseXmlAttribute(*element, "spriteSheetTexture", std::string());
	m_cellCount = ParseXmlAttribute(*element, "spriteSheetCellCount", m_cellCount);

	m_mapImage = new Image(m_image.c_str());
	m_spriteTexture = g_theRenderer->CreateOrGetTextureFromFile(m_texture.c_str());
}

MapDefinition::MapDefinition(Image* mapType, Texture* spriteTexture, IntVec2 cellCount)
	:m_mapImage(mapType), m_spriteTexture(spriteTexture), m_cellCount(cellCount)
{

}

SpawnInfo::SpawnInfo(const tinyxml2::XMLElement* element)
{
	// Parse attributes from XML element
	m_actorType = ParseXmlAttribute(*element, "actor", std::string());
	m_actorPosition = ParseXmlAttribute(*element, "position", Vec3::ZERO);
	m_actorOrientation = ParseXmlAttribute(*element, "orientation", EulerAngles::ZERO);
}
