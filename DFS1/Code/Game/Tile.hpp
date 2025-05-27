#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB3.hpp"
#include "ThirdParty/TinyXML2/tinyxml2.h"
#include "Engine/Core/XmlUtils.hpp"
#include <vector>
#include <string>

struct TileDefinition
{
	TileDefinition(const tinyxml2::XMLElement* element);
	static std::vector<TileDefinition> s_definitions;

	std::string m_name = "";
	bool m_isSolid = false;
	IntVec2 m_floorSpriteCoords = IntVec2::ZERO;
	IntVec2 m_ceilingSpriteCoords = IntVec2::ZERO;
	IntVec2 m_wallSpriteCoords = IntVec2::ZERO;
	Rgba8 m_tintColor = Rgba8::WHITE;
	static TileDefinition* GetTileDefByName(const std::string& name);
	static TileDefinition* GetTileDefinitionByColor(const Rgba8& color);
	static void InitializeTileDefs();
};

class Tile
{
	const TileDefinition* m_tileDef = nullptr;
	IntVec2 m_tileCoords;

public:
	Tile();
	~Tile();

public:
	const TileDefinition* GetTileDefinition() const;
	AABB3 GetTileBounds() const;
	IntVec2 GetTileCoords() const;
	IntVec2 SetTileCoords(const IntVec2& tileCoords);
	void SetTileType(const TileDefinition* tileDef);
	const TileDefinition* GetTileType() const;
	Rgba8 GetTileColor() const;
	std::string GetTileName() const;
	Vec3 GetTilePosition() const;
};