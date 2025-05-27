#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Game/Map.hpp"

class Texture;
class Clock;
class Player;
class Prop;

enum class GameState
{
	NONE,
	ATTRACT,
	LOBBY,
	PLAYING,
	COUNT,
};

class Game
{
	MapDefinition m_mapDef;

public:
	Game() = default; 
	~Game() = default;
	void Startup();
	void CreateMap(MapDefinition definition);
	void GameandHotKeys();

	void AttractModeTextRender();
	void LobbyTextRender();
	void HowToPlayTextRender();

	void CleanupCurrentMapAndPlayer();

	void UpdateTimeRemaining();
	void UpdateCircleRender();
	void CircleRender();
	void Shutdown();
	void RunFrame();
	void Render();
	void UpdateGameMode();

public:
	void EnterAttract();
	void EnterLobby();
	void EnterPlaying();

	void ExitAttract();
	void ExitPlaying();

	void UpdateLobby();
	void UpdateAttractMode();
	void UpdatePlaying();

	void RenderWinScreenUI();
	void RenderLoseScreenUI();
	void RenderLobby();
	void RenderAttract();
	void RenderPlaying();

public:
	GameState m_currentState = GameState::ATTRACT;

	Map* m_currentMap = nullptr;
	std::vector<Map*> m_maps;

	Player *m_player = nullptr;
	
	Clock  *m_clock = nullptr;
	
	Camera *m_otherUIView = nullptr;
	Camera *m_uiScreenView = nullptr;
	Camera *m_attractModeCamera = nullptr;
	Camera* m_lobbyModeCamera = nullptr;

private:
	Rgba8 m_startColor;
	Rgba8 m_endColor;

	float m_startThickness;
	float m_endThickness;

	float m_startRadius;
	float m_endRadius;

	bool m_isLerping = true;
	float m_lerpFactor = 0.0f;
	const float m_lerpSpeed = 1.25f;

	bool m_isRadiusLerping = true;
	float m_radiusLerpFactor = 1.0f;
	const float m_radiusLerpSpeed = 0.15f;

public:
	Texture* m_hourglassTexture = nullptr;
	Texture* m_loseScreenTexture = nullptr;
	Texture* m_winScreenTexture = nullptr;

	Rgba8 m_timerStartColor = Rgba8::WHITE;
	Rgba8 m_timerEndColor = Rgba8::RED;

	bool m_isTimerLerping = true;
	float m_timerLerpFactor = 0.0f;
	const float m_timerLerpSpeed = 1.25f;

	int m_randomMapSelection = 0;
	std::string m_maze;
};
