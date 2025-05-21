#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/Player.hpp"
#include "Game/Prop.hpp"
#include "Game/ActorDefinitions.hpp"
#include "Game/Actor.hpp"

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Input/XboxController.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/DebugDraw.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/SimpleTriangleFont.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Audio/AudioSystem.hpp"

#include <vector>
#include <ctime>

extern DevConsole* g_theConsole;

void Game::Startup()
{
	m_clock = new Clock();
	
	// Set up cameras
	m_uiScreenView = new Camera();
	m_otherUIView = new Camera();
	m_attractModeCamera = new Camera();
	m_lobbyModeCamera = new Camera();

	m_attractModeCamera->SetOrthoView(Vec2(0.f, 0.f), Vec2((float)g_theWindow->GetClientDimensions().x, (float)g_theWindow->GetClientDimensions().y));
	m_lobbyModeCamera->SetOrthoView(Vec2(0.f, 0.f), Vec2((float)g_theWindow->GetClientDimensions().x, (float)g_theWindow->GetClientDimensions().y));
	m_uiScreenView->SetOrthoView(Vec2(0.f, 0.f), Vec2((float)g_theWindow->GetClientDimensions().x, (float)g_theWindow->GetClientDimensions().y));
	m_otherUIView->SetOrthoView(Vec2(0.f, 0.f), Vec2((float)g_theWindow->GetClientDimensions().x, (float)g_theWindow->GetClientDimensions().y));
	
	m_hourglassTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Hourglass2.png");
	m_loseScreenTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Meme.jpg");
	m_winScreenTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Meme2.jpg");
	g_bitmapFont = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont.png");


	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetRasterizerState(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);

	GameandHotKeys();
}


void Game::CreateMap(MapDefinition definition)
{
	m_maps.emplace_back(new Map(this, definition));
}

void Game::GameandHotKeys()
{
	g_theConsole->AddLine(Rgba8::GREEN, "------------------------------");
	g_theConsole->AddLine(Rgba8::LIGHT_BLUE, "Game Controls");
	g_theConsole->AddLine(Rgba8::YELLOW, "Press & hold down the 'W' to move forward. + shift increase move speed");
	g_theConsole->AddLine(Rgba8::YELLOW, "Press & hold down the 'A' or 'D' to move lateral");
	g_theConsole->AddLine(Rgba8::YELLOW, "Press & hold down the 'S' to move backward");
	g_theConsole->AddLine(Rgba8::YELLOW, "Press & hold down the 'Z' to move up");
	g_theConsole->AddLine(Rgba8::YELLOW, "Press & hold down the 'C' to move down");
	g_theConsole->AddLine(Rgba8::YELLOW, "Press & hold down the 'Q' to roll to the left");
	g_theConsole->AddLine(Rgba8::YELLOW, "Press & hold down the 'E' to roll to the right");
	g_theConsole->AddLine(Rgba8::YELLOW, "Use mouse to update orientation");
	g_theConsole->AddLine(Rgba8::GREEN, "------------------------------");
	g_theConsole->AddLine(Rgba8::YELLOW, "Press 'Esc' to go back to the Attract Screen if in game");
	g_theConsole->AddLine(Rgba8::YELLOW, "Press 'Esc' to close the application if at the attract screen");
	g_theConsole->AddLine(Rgba8::GREEN, "------------------------------");
}

void Game::UpdateAttractMode()
{
	UpdateCircleRender();

	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		ExitAttract();
		g_theApp->HandleQuitRequested();
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE) || g_theInput->WasKeyJustPressed(KEYCODE_ENTER))
	{
		EnterLobby();
		m_currentState = GameState::LOBBY;
	}
}

void Game::UpdateLobby()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		EnterAttract();
		m_currentState = GameState::ATTRACT;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE) || g_theInput->WasKeyJustPressed(KEYCODE_ENTER))
	{
		// Set game to playing state
		EnterPlaying();
		m_currentState = GameState::PLAYING;
	}
}

void Game::UpdateTimeRemaining()
{
	float deltaSeconds = m_clock->GetDeltaSeconds();

	// Update the lerp factor
	if (m_isTimerLerping)
	{
		m_timerLerpFactor += m_timerLerpSpeed * deltaSeconds; // Lerp towards 1.0f

		if (m_timerLerpFactor >= 1.0f)
		{
			m_timerLerpFactor = 1.0f; // Clamp it
			m_isTimerLerping = false; // Start lerping in the opposite direction
		}
	}
	else
	{
		m_timerLerpFactor -= m_timerLerpSpeed * deltaSeconds; // Lerp towards 0.0f

		if (m_timerLerpFactor <= 0.0f)
		{
			m_timerLerpFactor = 0.0f; // Clamp it
			m_isTimerLerping = true; // Start lerping in the opposite direction
		}
	}

	// Determine the current color based on the lerp factor
	Rgba8 timeRemainingColor;
	if (m_currentMap->m_gameTime <= 10.f)
	{
		float currentR = Interpolate(m_timerStartColor.r, m_timerEndColor.r, m_timerLerpFactor);
		float currentG = Interpolate(m_timerStartColor.g, m_timerEndColor.g, m_timerLerpFactor);
		float currentB = Interpolate(m_timerStartColor.b, m_timerEndColor.b, m_timerLerpFactor);
		float currentA = Interpolate(m_timerStartColor.a, m_timerEndColor.a, m_timerLerpFactor);
		Rgba8 currentColor(static_cast<unsigned char>(currentR), static_cast<unsigned char>(currentG), static_cast<unsigned char>(currentB), static_cast<unsigned char>(currentA));
		timeRemainingColor = currentColor;
	}

	if (m_currentMap->m_didAddTime)
	{
		std::string addedTimeText = Stringf("+5.00");
		DebugAddScreenText(addedTimeText, Vec2((float)g_theWindow->GetClientDimensions().x - 880.f, (float)g_theWindow->GetClientDimensions().y - 30.f), 20.f, Vec2(0.5f, 0.5f), 0.f, Rgba8::GREEN, Rgba8::GREEN);

		m_currentMap->m_addTimeShow -= deltaSeconds;
		if (m_currentMap->m_addTimeShow <= 0.f)
		{
			m_currentMap->m_didAddTime = false;
			m_currentMap->m_addTimeShow = 1.f;
		}
	}

	std::string timerPosText = Stringf("Time Remaining: %.2f", m_currentMap->m_gameTime);
	DebugAddScreenText(timerPosText, Vec2((float)g_theWindow->GetClientDimensions().x - 1200.f, (float)g_theWindow->GetClientDimensions().y - 50.f), 20.f, Vec2(0.5f, 0.5f), 0.f, timeRemainingColor, timeRemainingColor);
}

void Game::UpdateCircleRender()
{
	float deltaSeconds = m_clock->GetDeltaSeconds();
	// Lerp
	if (m_isLerping)
	{
		m_lerpFactor += m_lerpSpeed * deltaSeconds; // Go right 

		if (m_lerpFactor >= 1.0f)
		{
			m_startColor = Rgba8::YELLOW;
			m_startThickness = 10.f;
			m_lerpFactor = 1.f; // Clamp it 
			m_isLerping = false;
		}
	}
	else
	{
		m_lerpFactor -= m_lerpSpeed * deltaSeconds; // Go left

		if (m_lerpFactor <= 0.0f)
		{
			m_endColor = Rgba8::LIGHT_BLUE;
			m_endThickness = 5.f;
			m_lerpFactor = 0.0f; // Clamp it 
			m_isLerping = true;
		}
	}

	if (m_isRadiusLerping)
	{
		m_radiusLerpFactor += m_radiusLerpSpeed * deltaSeconds;
		if (m_radiusLerpFactor >= 1.0f)
		{
			m_startRadius = 200.f;
			m_isRadiusLerping = false;
		}
	}
	else
	{
		m_radiusLerpFactor -= m_radiusLerpSpeed * deltaSeconds;
		if (m_radiusLerpFactor <= 0.0f)
		{
			m_endRadius = 50.f;
			m_isRadiusLerping = true;
		}
	}
}

void Game::UpdatePlaying()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		// Set game to attract state
		ExitPlaying();
		EnterAttract();
		m_currentState = GameState::ATTRACT;
	}
	else
	{
		m_currentMap->MapUpdate();
		UpdateTimeRemaining();
	}
}

void Game::AttractModeTextRender()
{
	std::vector<Vertex_PCU> textVerts;
	g_bitmapFont->AddVertsForText2D(textVerts, Vec2((float)g_theWindow->GetClientDimensions().x - 1400.f, (float)g_theWindow->GetClientDimensions().y - 200.f), 50.f, "3D Maze Game", Rgba8::LIGHT_GRAY, 1.f);
	g_theRenderer->BindTexture(0, &g_bitmapFont->GetTexture());
	g_theRenderer->DrawVertexArray(static_cast<int>(textVerts.size()), textVerts.data());
}

void Game::CircleRender()
{
	// Use the Interpolate function to calculate the current color for each component (R, G, B, A)
	float currentR = Interpolate(m_startColor.r, m_endColor.r, m_lerpFactor);
	float currentG = Interpolate(m_startColor.g, m_endColor.g, m_lerpFactor);
	float currentB = Interpolate(m_startColor.b, m_endColor.b, m_lerpFactor);
	float currentA = Interpolate(m_startColor.a, m_endColor.a, m_lerpFactor);

	float currentThickness = Interpolate(m_startThickness, m_endThickness, m_lerpFactor);
	float currentRadius = Interpolate(m_startRadius, m_endRadius, m_radiusLerpFactor);

	// Create the current color using the interpolated values
	Rgba8 currentColor(static_cast<unsigned char>(currentR), static_cast<unsigned char>(currentG), static_cast<unsigned char>(currentB), static_cast<unsigned char>(currentA));
	DebugDraw2DRing(Vec2(800.f, 400.f), currentRadius, currentThickness, Rgba8(currentColor)); // Blue for avoidance range
}

void Game::LobbyTextRender()
{
	std::vector<Vertex_PCU> textVerts;

	g_bitmapFont->AddVertsForText2D(textVerts, Vec2(500.f, 600.f), 40.f, "Player 1", Rgba8::LIGHT_GRAY, 1.f);
	g_bitmapFont->AddVertsForText2D(textVerts, Vec2(500.f, 570.f), 25.f, "Mouse and keyboard", Rgba8::WHITE, 1.f);
	g_bitmapFont->AddVertsForText2D(textVerts, Vec2(500.f, 540.f), 25.f, "Press the Spacebar to continue", Rgba8::GREEN, 1.f);
	g_bitmapFont->AddVertsForText2D(textVerts, Vec2(500.f, 510.f), 25.f, "Press ESCAPE to quit", Rgba8::RED, 1.f);

	g_theRenderer->BindTexture(0, &g_bitmapFont->GetTexture());
	g_theRenderer->DrawVertexArray(static_cast<int>(textVerts.size()), textVerts.data());
}

void Game::HowToPlayTextRender()
{
	std::vector<Vertex_PCU> verts;

	g_bitmapFont->AddVertsForText2D(verts, Vec2(550.f, 700.f), 40.f, "How To Play", Rgba8::LIGHT_GRAY, 1.f);
	g_bitmapFont->AddVertsForText2D(verts, Vec2(100.f, 600.f), 25.f, "- WASD to move in the cardinal directions", Rgba8::WHITE, 1.f);
	g_bitmapFont->AddVertsForText2D(verts, Vec2(100.f, 570.f), 25.f, "- Press any two move keys together to move diagonally", Rgba8::WHITE, 1.f);
	g_bitmapFont->AddVertsForText2D(verts, Vec2(100.f, 540.f), 25.f, "- Press P to pause and unpause the game", Rgba8::WHITE, 1.f);
	
	g_bitmapFont->AddVertsForText2D(verts, Vec2(50.f, 500.f), 25.f, "The Gist of the game: ", Rgba8::LIGHT_GRAY, 1.f);
	g_bitmapFont->AddVertsForText2D(verts, Vec2(50.f, 470.f), 13.f, "- The Goal of this game is to reach the end of the maze without getting touched by an AI and before the time runs out", Rgba8::YELLOW, 1.f);
	g_bitmapFont->AddVertsForText2D(verts, Vec2(50.f, 450.f), 15.f, "- Collect floating items (time boxes) to add 5 more seconds to the decrementing timer", Rgba8::YELLOW, 1.f);
	g_bitmapFont->AddVertsForText2D(verts, Vec2(450.f, 370.f), 25.f, "Press the Spacebar to enter the game", Rgba8::GREEN, 1.f);
	g_bitmapFont->AddVertsForText2D(verts, Vec2(450.f, 340.f), 25.f, "Press ESCAPE to leave game", Rgba8::RED, 1.f);

	g_theRenderer->BindTexture(0, &g_bitmapFont->GetTexture());
	g_theRenderer->DrawVertexArray(static_cast<int>(verts.size()), verts.data());
}

void Game::RunFrame()
{
	Render();
/*	EndFrame();*/
}

void Game::RenderWinScreenUI()
{
	g_theRenderer->BeginCamera(*m_otherUIView);

	std::vector<Vertex_PCU> verts;
	AddVertsForAABB2D(verts, AABB2(Vec2::ZERO, Vec2((float)g_theWindow->GetClientDimensions().x, (float)g_theWindow->GetClientDimensions().y)));
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture(0, m_winScreenTexture);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());

	std::vector<Vertex_PCU> textVerts;
	g_bitmapFont->AddVertsForText2D(textVerts, Vec2(400.f, 600.f), 25.f, "You Won! Press Esc to Quit", Rgba8::LIGHT_BLUE, 1.f);
	g_theRenderer->BindTexture(0, &g_bitmapFont->GetTexture());
	g_theRenderer->DrawVertexArray(static_cast<int>(textVerts.size()), textVerts.data());

	g_theRenderer->EndCamera(*m_otherUIView);
}

void Game::RenderLoseScreenUI()
{
	g_theRenderer->BeginCamera(*m_otherUIView);
	
	std::vector<Vertex_PCU> verts;
	AddVertsForAABB2D(verts, AABB2(Vec2::ZERO, Vec2((float)g_theWindow->GetClientDimensions().x, (float)g_theWindow->GetClientDimensions().y)));
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture(0, m_loseScreenTexture);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());

	std::vector<Vertex_PCU> textVerts;
	g_bitmapFont->AddVertsForText2D(textVerts, Vec2(400.f, 600.f), 25.f, "You Lost! Press Esc to Quit", Rgba8::LIGHT_BLUE, 1.f);
	g_theRenderer->BindTexture(0, &g_bitmapFont->GetTexture());
	g_theRenderer->DrawVertexArray(static_cast<int>(textVerts.size()), textVerts.data());

	g_theRenderer->EndCamera(*m_otherUIView);
}

void Game::RenderLobby()
{
	g_theRenderer->BeginCamera(*m_lobbyModeCamera);

	HowToPlayTextRender();

	g_theRenderer->EndCamera(*m_lobbyModeCamera);
}

void Game::RenderAttract()
{
	g_theRenderer->BeginCamera(*m_attractModeCamera);

	CircleRender();
	AttractModeTextRender();
	LobbyTextRender();

	g_theRenderer->EndCamera(*m_attractModeCamera);
}


void Game::RenderPlaying()
{
	std::vector<Vertex_PCU> verts;

	if (m_player)
	{
		g_theRenderer->BeginCamera(m_player->m_playerWorldView);
		m_currentMap->MapRender();
		DebugRenderWorld(m_player->m_playerWorldView);
		g_theRenderer->EndCamera(m_player->m_playerWorldView);

		m_player->Render();

		g_theRenderer->BeginCamera(*m_uiScreenView);

		std::string numEnemiesText = Stringf("Num Enemies: %d", m_currentMap->m_numEnemyActors.size());
		DebugAddScreenText(numEnemiesText, Vec2((float)g_theWindow->GetClientDimensions().x - 1900.f, (float)g_theWindow->GetClientDimensions().y - 45.f), 15.f, Vec2(0.5f, 0.5f), 0.f, Rgba8::LIGHT_RED, Rgba8::LIGHT_RED);

		if (m_player->m_currentCameraMode == CameraMode::MAP)
		{
			std::string debugText = Stringf("%s", m_player->m_isShowingDebugOptions ? "Press F9 to disable debug keys" : "Press F9 to enable debug keys");
			DebugAddScreenText(debugText, Vec2((float)g_theWindow->GetClientDimensions().x - 1900.f, (float)g_theWindow->GetClientDimensions().y - 60.f), 15.f, Vec2(0.5f, 0.5f), 0.f, Rgba8::WHITE, Rgba8::WHITE);
			
			if (m_player->GetActor() != nullptr)
			{
				std::string playerHealthInfo = Stringf("Player actor health value: %d", m_player->GetActor()->m_health);
				DebugAddScreenText(playerHealthInfo, Vec2((float)g_theWindow->GetClientDimensions().x - 1900.f, (float)g_theWindow->GetClientDimensions().y - 30.f), 15.f, Vec2(0.5f, 0.5f), 0.f, Rgba8::LIGHT_ORANGE, Rgba8::LIGHT_ORANGE);
			}

			if (m_player->m_isShowingDebugOptions)
			{
				std::string aiPathViewEnabledText = Stringf("%s", m_currentMap->m_canSeeAiPath ? "Press F1 to disable AI current path view" : "Press F1 to enable AI current path view");
				DebugAddScreenText(aiPathViewEnabledText, Vec2((float)g_theWindow->GetClientDimensions().x - 1900.f, (float)g_theWindow->GetClientDimensions().y - 75.f), 15.f, Vec2(0.5f, 0.5f), 0.f, Rgba8::WHITE, Rgba8::WHITE);

				std::string aiGoalPositionEnabledText = Stringf("%s", m_currentMap->m_canSeeAiGoalPosition ? "Press F3 to disable AI Goal Position view" : "Press F3 to enable AI Goal Position view");
				DebugAddScreenText(aiGoalPositionEnabledText, Vec2((float)g_theWindow->GetClientDimensions().x - 1900.f, (float)g_theWindow->GetClientDimensions().y - 105.f), 15.f, Vec2(0.5f, 0.5f), 0.f, Rgba8::WHITE, Rgba8::WHITE);
			}
		}

		std::string timeFpsScaleText = Stringf("[Game Clock] Time: %.2f, FPS: %.2f", m_clock->GetTotalSeconds(), 1.f / m_clock->GetDeltaSeconds());
		DebugAddScreenText(timeFpsScaleText, Vec2((float)g_theWindow->GetClientDimensions().x - 600.f, (float)g_theWindow->GetClientDimensions().y - 30.f), 15.f, Vec2(0.5f, 0.5f), 0.f, Rgba8::WHITE, Rgba8::WHITE);
	}

	DebugRenderScreen(*m_uiScreenView);

	g_theRenderer->EndCamera(*m_uiScreenView);

	if (m_currentMap->m_hasPlayerReachedGoal && m_currentMap->m_gameTime > 0.f)
	{
		RenderWinScreenUI();
	}

	if (m_currentMap->m_gameTime <= 0.f || m_currentMap->GetPlayerActor() == nullptr)
	{
		RenderLoseScreenUI();
	}
}

void Game::Render()
{
	switch (m_currentState)
	{
	case GameState::ATTRACT:
		RenderAttract();
		break;
	case GameState::LOBBY:
		RenderLobby();
		break;
	case GameState::PLAYING:
		RenderPlaying();
		break;
	}
}

void Game::UpdateGameMode()
{
	switch (m_currentState)
	{
	case GameState::ATTRACT:
		UpdateAttractMode();
		g_theInput->SetCursorMode(false, false);
		break;
	case GameState::LOBBY:
		UpdateLobby();
		g_theInput->SetCursorMode(false, false);
		break;
	case GameState::PLAYING:
		UpdatePlaying();
		break;
	}
}

void Game::EnterAttract()
{
	CleanupCurrentMapAndPlayer();
}

void Game::EnterLobby()
{
	// For animations, music or any setup for attract mode
}

void Game::EnterPlaying()
{
	WeaponDefinition::InitializeWeaponDef();
	ActorDefinition::InitializeActorDef();
	TileDefinition::InitializeTileDefs();
	MapDefinition::InitializeMapDef();

	g_rng.SetSeed(static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
	m_randomMapSelection = g_rng.SRollRandomIntInRange(1, 3);
	if (m_randomMapSelection == 1)
	{
		m_maze = "MazeOne";
	}
	else if (m_randomMapSelection == 2)
	{
		m_maze = "MazeTwo";
	}
	else if (m_randomMapSelection == 3)
	{
		m_maze = "MazeThree";
	}
	m_mapDef = *MapDefinition::GetMapDefByName(m_maze);

	m_player = new Player(this, Vec3(2.5f, 8.5f, 0.5f), EulerAngles::ZERO);

	CreateMap(m_mapDef);
 	m_currentMap = m_maps[0];
}

void Game::ExitAttract()
{

}

void Game::ExitPlaying()
{
	// Future will be used to save, clear or reset things.
	CleanupCurrentMapAndPlayer();
}

void Game::CleanupCurrentMapAndPlayer()
{
	SafeDelete(m_player);

	m_maps.clear();
	SafeDelete(m_currentMap);
}

void Game::Shutdown()
{   
	CleanupCurrentMapAndPlayer();
}
