#include "Player.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Clock.hpp"
#include "Game/Game.hpp"
#include "Game/Actor.hpp"
#include "Engine/Core/Clock.hpp"
#include "Game/ActorDefinitions.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/VertexUtils.hpp"

extern Renderer* g_theRenderer;
extern InputSystem* g_theInput;

Player::Player(Game* owner, Vec3 const& startPos, EulerAngles const& startOrientation)
	:m_game(owner), m_position(startPos), m_orientation(startOrientation)
{
	m_currentCameraMode = CameraMode::MAP;
}

Player::~Player()
{
}

void Player::Render()
{
}

void Player::Update()
{
	float deltaSeconds = m_game->m_clock->GetDeltaSeconds();
	Actor* actor = m_game->m_currentMap->GetActorByUID(m_actorUID);
	actor->m_orientation.m_pitchDegrees = GetClamped(actor->m_orientation.m_pitchDegrees, -85.f, 85.f);

	if (g_theInput->WasKeyJustPressed(KEYCODE_F9))
	{
		m_isShowingDebugOptions = !m_isShowingDebugOptions;
	}

	if (g_theInput->WasKeyJustPressed('P'))
	{
		m_game->m_clock->TogglePause();
	}

	if (m_game->m_player)
	{
		if (g_theInput->WasKeyJustPressed('F'))
		{
			ToggleCamera();
		}

		if (g_theInput->WasKeyJustPressed('N'))
		{
			m_game->m_currentMap->DebugPossessNext();
		}

		if (m_currentCameraMode == CameraMode::FREEFLY)
		{
			m_orientation.m_pitchDegrees = GetClamped(m_orientation.m_pitchDegrees, -85.f, 85.f);
			m_orientation.m_rollDegrees = GetClamped(m_orientation.m_rollDegrees, -45.f, 45.f);
			
			FreeFlyMouseMovementUpdate();
			FreeFlyKeyInputUpdate(deltaSeconds);

			m_playerWorldView.SetPerspectiveView(2.f, 60.f, 0.1f, 1000.f);
			m_playerWorldView.SetRenderBasis(Vec3(0.f, 0.f, 1.f), Vec3(-1.f, 0.f, 0.f), Vec3(0.f, 1.f, 0.f));
			m_playerWorldView.SetTransform(m_position, m_orientation);
		}
		else if (m_currentCameraMode == CameraMode::MAP)
		{
			m_position = actor->m_position + Vec3(0.f, 0.f, actor->m_eyeHeight);
			m_orientation = actor->m_orientation;
			
 			Vec3 camPosition = actor->m_position + Vec3(0.f, -10.f, 50.f);
 			EulerAngles camOrientation = EulerAngles(90.f, 60.f, 0.f);				
			ControlledActorMovement();		
 			m_playerWorldView.SetPerspectiveView(2.f, 50.f, 0.1f, 1000.f);
 			m_playerWorldView.SetRenderBasis(Vec3(0.f, 0.f, 1.f), Vec3(-1.f, 0.f, 0.f), Vec3(0.f, 1.f, 0.f));
 			m_playerWorldView.SetTransform(camPosition, camOrientation);

			if (actor->m_health <= 0)
			{
				actor->m_isDead = true;
				actor->m_isMoveable = false;
			}
		}
	}
}

void Player::FreeFlyKeyInputUpdate(float& deltaSeconds)
{
	const float turnRate = 90.f;
	Vec3 forward = m_orientation.GetForwardVector();
	Vec3 right = m_orientation.GetRightVector();
	Vec3 up = m_orientation.GetUpVector();

	if (g_theInput->IsKeyDown(KEYCODE_SHIFT))
	{
		m_movementSpeed = 25.f;
	}
	else
	{
		m_movementSpeed = 5.f;
	}

	if (g_theInput->WasKeyJustPressed('H'))
	{
		m_position = Vec3(0.f, 0.f, 0.f);
		m_orientation = EulerAngles(90.f, 0.f, 0.f);
	}

	if (g_theInput->IsKeyDown('W')) // Move forward
	{
		m_position += forward * m_movementSpeed * deltaSeconds;
	}

	if (g_theInput->IsKeyDown('S')) // Move backward
	{
		m_position -= forward * m_movementSpeed * deltaSeconds;
	}

	if (g_theInput->IsKeyDown('A')) // Move left
	{
		m_position -= right * m_movementSpeed * deltaSeconds;
	}

	if (g_theInput->IsKeyDown('D')) // Move right
	{
		m_position += right * m_movementSpeed * deltaSeconds;
	}

	if (g_theInput->IsKeyDown('Z'))
	{
		m_position += up * m_movementSpeed * deltaSeconds;
	}

	if (g_theInput->IsKeyDown('C'))
	{
		m_position -= up * m_movementSpeed * deltaSeconds;
	}

	// Handle rotation (roll)
	if (g_theInput->IsKeyDown('Q'))
	{
		m_orientation.m_rollDegrees -= turnRate * deltaSeconds;
	}

	if (g_theInput->IsKeyDown('E'))
	{
		m_orientation.m_rollDegrees += turnRate * deltaSeconds;
	}
}

void Player::FreeFlyMouseMovementUpdate()
{
	m_orientation.m_yawDegrees -= g_theInput->GetCursorClientDelta().x * 0.075f;
	m_orientation.m_pitchDegrees += GetClamped(g_theInput->GetCursorClientDelta().y * 0.075f, -85.f, 85.f);
}

void Player::ControlledActorMovement()
{
	Actor* actor = m_game->m_currentMap->GetActorByUID(m_actorUID);
	Vec3 forward = actor->m_orientation.GetForwardVector().GetNormalized();

	if (actor->m_isDead || m_game->m_currentMap->m_hasPlayerReachedGoal)
	{
		if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
		{
			m_game->ExitPlaying();
			m_game->EnterAttract();
			m_game->m_currentState = GameState::ATTRACT;
		}
		return;
	}

	if (g_theInput->IsKeyDown(KEYCODE_SHIFT))
	{
		m_controlledMovementSpeed = actor->m_runSpeed;
	}
	else
	{
		m_controlledMovementSpeed = actor->m_walkSpeed;
	}

	Vec3 directionalMovement(0.f, 0.f, 0.f);
	float desiredYaw = actor->m_orientation.m_yawDegrees;

	if (g_theInput->IsKeyDown('W')) // Move forward
	{
		desiredYaw = 90.f;
		directionalMovement += forward * m_controlledMovementSpeed;
	}

	if (g_theInput->IsKeyDown('S')) // Move backward
	{
		desiredYaw = 270.f;
		directionalMovement += forward * m_controlledMovementSpeed;
	}

	if (g_theInput->IsKeyDown('A')) // Move left
	{
		desiredYaw = 180.f;
		directionalMovement += forward * m_controlledMovementSpeed;
	}

	if (g_theInput->IsKeyDown('D')) // Move right
	{
		desiredYaw = 0.f;
		directionalMovement += forward * m_controlledMovementSpeed;
	}

	if (g_theInput->IsKeyDown('W') && g_theInput->IsKeyDown('D')) // Move forward-right
	{
		desiredYaw = 45.f;
	}

	if (g_theInput->IsKeyDown('W') && g_theInput->IsKeyDown('A')) // Move forward-left
	{
		desiredYaw = 135.f;
	}

	if (g_theInput->IsKeyDown('S') && g_theInput->IsKeyDown('A')) // Move backward-left
	{
		desiredYaw = 225.f;
	}

	if (g_theInput->IsKeyDown('S') && g_theInput->IsKeyDown('D')) // Move backward-right
	{
		desiredYaw = 315.f;
	}

	if (directionalMovement.GetLength() > 0)
	{
		directionalMovement.Normalize();
		actor->TurnInDirection(desiredYaw, 5.f);
		actor->MoveInDirection(directionalMovement, m_controlledMovementSpeed);
	}
}

void Player::ToggleCamera()
{
	if (m_currentCameraMode == CameraMode::MAP)
	{
		m_currentCameraMode = CameraMode::FREEFLY;
	}
	else if (m_currentCameraMode == CameraMode::FREEFLY)
	{
		m_currentCameraMode = CameraMode::MAP;
	}
}
