#include "Prop.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Clock.hpp"
#include "Game/Game.hpp"

extern Renderer* g_theRenderer;

Prop::Prop(Game* owner, Vec3 const& startPos)
{
	m_game = owner;
	m_position = startPos;
}

Prop::Prop()
{
}

Prop::~Prop()
{
}

void Prop::RenderGrid()
{
	const int gridSize = 100;
	const int thickLineSpacing = 5;
	const float lineWidth = 0.1f;
	int halfGridSize = gridSize / 2;

	// Iterate over Y-Axis lines 
	for (int y = -halfGridSize; y <= halfGridSize; y++)
	{
		Rgba8 color = Rgba8::LIGHT_GRAY;
		if (y % thickLineSpacing == 0)
		{
			// Thicker and brighter lines
			if (y == 0 || y == halfGridSize || y == -halfGridSize)
			{
				color = Rgba8::GREEN;
			}
			else
			{
				color = Rgba8::DARK_GREEN;
			}

			AddLine(Vec3(static_cast<float>(y), static_cast<float>(-halfGridSize), 0.f), Vec3(static_cast<float>(y), static_cast<float>(halfGridSize), 0.f), color, lineWidth);
		}
		else
		{
			// Regular lines
			AddLine(Vec3(static_cast<float>(y), static_cast<float>(-halfGridSize), 0.f), Vec3(static_cast<float>(y), static_cast<float>(halfGridSize), 0.f), Rgba8::LIGHT_GRAY, lineWidth * 0.5f);
		}
	}

	// Iterate over X-Axis lines
	for (int x = -halfGridSize; x <= halfGridSize; x++)
	{
		Rgba8 color = Rgba8::LIGHT_GRAY;
		if (x % thickLineSpacing == 0)
		{
			// Thicker and brighter lines
			if (x == 0 || x == halfGridSize || x == -halfGridSize)
			{
				color = Rgba8::RED;
			}
			else
			{
				color = Rgba8::DARK_RED;
			}

			AddLine(Vec3(static_cast<float>(-halfGridSize), static_cast<float>(x), 0.f), Vec3(static_cast<float>(halfGridSize), static_cast<float>(x), 0.f), color, lineWidth);
		}
		else
		{
			// Regular lines
			AddLine(Vec3(static_cast<float>(-halfGridSize), static_cast<float>(x), 0.f), Vec3(static_cast<float>(halfGridSize), static_cast<float>(x), 0.f), Rgba8::LIGHT_GRAY, lineWidth * 0.5f);
		}
	}
}

void Prop::AddLine(const Vec3& bottomLeft, const Vec3& topRight, const Rgba8& color, float thickness) 
{
	Vec3 center = (bottomLeft + topRight) * 0.5f;
	Vec3 scale = (topRight - bottomLeft);

	// Adjust scale to account for thickness along the x-axis and y-axis
	if (scale.x == 0.f) 
	{
		scale.x = thickness;
	}
	else if (scale.y == 0.f) 
	{
		scale.y = thickness;
	}
	scale.z = thickness;

	// Add the line cube with adjusted scale
	AddVertsForAABB3D(m_vertexes, AABB3(center + scale * 0.5f, center - scale * 0.5f), color, AABB2(Vec2(0.0f, 0.0f), Vec2(1.0f, 1.0f)));
}



void Prop::Render() const
{
	g_theRenderer->SetRasterizerState(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants();
	
	g_theRenderer->BindTexture(0, nullptr);

	g_theRenderer->DrawVertexArray(static_cast<int>(m_vertexes.size()), m_vertexes.data());
}

void Prop::Update()
{
}
