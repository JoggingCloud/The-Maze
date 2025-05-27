#include "Game/Tile.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

std::vector<TileDefinition> TileDefinition::s_definitions;

Tile::Tile()
{
}

Tile::~Tile()
{
}

const TileDefinition* Tile::GetTileDefinition() const
{
	return m_tileDef;
}

AABB3 Tile::GetTileBounds() const
{
	float tileHeight = 1.f;
	Vec3 mins = Vec3(static_cast<float>(m_tileCoords.x), static_cast<float>(m_tileCoords.y), 0.f);
	Vec3 maxs = Vec3(static_cast<float>(m_tileCoords.x) + 1.f, static_cast<float>(m_tileCoords.y) + 1.f, tileHeight);
	return AABB3(mins, maxs);
}

IntVec2 Tile::GetTileCoords() const
{
	return m_tileCoords;
}

IntVec2 Tile::SetTileCoords(const IntVec2& tileCoords)
{
	m_tileCoords = tileCoords;
	return m_tileCoords;
}

void Tile::SetTileType(TileDefinition const* tileDef)
{
	m_tileDef = tileDef;
}

const TileDefinition* Tile::GetTileType() const
{
	return m_tileDef;
}

Rgba8 Tile::GetTileColor() const
{
	if (m_tileDef)
	{
		return m_tileDef->m_tintColor;
	}
	else
	{
		return Rgba8::WHITE;
	}
}

std::string Tile::GetTileName() const
{
	if (m_tileDef)
	{
		return m_tileDef->m_name;
	}
	else
	{
		return "";
	}
}

Vec3 Tile::GetTilePosition() const
{
	if (m_tileDef)
	{
		return Vec3((float)m_tileCoords.x + 0.5f, (float)m_tileCoords.y + 0.5f, 0.f);
	}
	else
	{
		return Vec3::ZERO;
	}
}

TileDefinition::TileDefinition(const tinyxml2::XMLElement* element)
{
	// Parse attributes from XML element
	m_name = ParseXmlAttribute(*element, "name", std::string());
	m_isSolid = ParseXmlAttribute(*element, "isSolid", m_isSolid);
	m_floorSpriteCoords = ParseXmlAttribute(*element, "floorSpriteCoords", IntVec2::ZERO);
	m_ceilingSpriteCoords = ParseXmlAttribute(*element, "ceilingSpriteCoords", IntVec2::ZERO);
	m_wallSpriteCoords = ParseXmlAttribute(*element, "wallSpriteCoords", IntVec2::ZERO);
	m_tintColor = ParseXmlAttribute(*element, "mapImagePixelColor", m_tintColor);
}

TileDefinition* TileDefinition::GetTileDefByName(const std::string& name)
{
	for (TileDefinition& tileDef : s_definitions)
	{
		if (tileDef.m_name == name)
		{
			return &tileDef;
		}
	}
	return nullptr; // Tile definition with the given name was not found
}

TileDefinition* TileDefinition::GetTileDefinitionByColor(const Rgba8& color)
{
	for (TileDefinition& tileDef : s_definitions)
	{
		if (tileDef.m_tintColor == color)
		{
			return &tileDef;
		}
	}
	return nullptr;
}

void TileDefinition::InitializeTileDefs()
{
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("Data/Definitions/TileDefinitions.xml") != tinyxml2::XML_SUCCESS)
	{
		// Handle error loading XML file
		ERROR_AND_DIE("Failed to load TIleDefinitions.xml file");
		return;
	}

	const tinyxml2::XMLElement* root = doc.FirstChildElement("TileDefinitions");
	if (!root)
	{
		// Handle missing root element
		ERROR_AND_DIE("Failed to get the root in the tile definition xml file");
		return;
	}

	// Only initialize if it hasn't been initialized already
	if (s_definitions.empty())
	{
		// Load & iterate over TileDefinition elements
		for (const tinyxml2::XMLElement* element = root->FirstChildElement("TileDefinition"); element; element = element->NextSiblingElement("TileDefinition"))
		{
			s_definitions.push_back(TileDefinition(element));
		}
	}
}