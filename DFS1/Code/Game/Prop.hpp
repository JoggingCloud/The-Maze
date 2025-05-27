#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include <vector>

class Game;

class Prop
{
public:
	Prop(Game* owner, Vec3 const& startPos);
	Prop();
	virtual ~Prop();
	void RenderGrid();
	void AddLine(const Vec3& bottomLeft, const Vec3& topRight, const Rgba8& color, float thickness);
	void Update();
	void Render() const;

public:
	std::vector<Vertex_PCU> m_vertexes;
	Rgba8                   m_color = Rgba8::WHITE;
	Game* m_game = nullptr;
	Vec3 m_position = Vec3::ZERO;
	EulerAngles m_orientation = EulerAngles::ZERO;
	float m_timer = 0.f;
	bool m_isWhite = false;
};