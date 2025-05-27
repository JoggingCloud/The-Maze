#pragma once
#include "Game/Controller.hpp"
#include "Engine/Renderer/Camera.hpp"
#include <vector>

class Camera;
class Clock;
class Game;
class Actor;

enum CameraMode
{
	FREEFLY,
	MAP,
	NUM_CAM_MODES,
};

class Player : public Controller
{
public:
	Player(Game* owner, Vec3 const& startPos, EulerAngles const& startOrientation);
	Player() = default;
	virtual ~Player();
	virtual void Render();
	virtual void Update();
	
	void FreeFlyMouseMovementUpdate();
	void FreeFlyKeyInputUpdate(float& deltaSeconds);

	void ControlledActorMovement();

	void ToggleCamera();

public:
	Game* m_game = nullptr;
	Vec3 m_position = Vec3::ZERO;
	EulerAngles m_orientation = EulerAngles::ZERO;
	Camera m_playerWorldView;
	Camera m_playerUICamera;
	float m_movementSpeed = 2.f;
	float m_controlledMovementSpeed = 0.f;
	float m_camerAspectRatio = 2.f;
	bool m_isShowingDebugOptions = false;
	CameraMode m_currentCameraMode = CameraMode::NUM_CAM_MODES;

	ActorUID m_targetActorUID = ActorUID::INVALID;
};