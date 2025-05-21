#include "Game/App.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "ThirdParty/TinyXML2/tinyxml2.h"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"

Window*		 g_theWindow   = nullptr;
App*		 g_theApp      = nullptr;
Renderer*	 g_theRenderer = nullptr;
InputSystem* g_theInput    = nullptr;
AudioSystem* g_theAudio    = nullptr;

STATIC bool App::Event_Quit(EventArgs& args)
{
	UNUSED(args);
	g_theApp->HandleQuitRequested();
	return false;
}

STATIC bool App::Event_WindowMinimized(EventArgs& args)
{
	UNUSED(args);
	//DebuggerPrintf("Window Minimized!\n");
	g_theApp->m_isPaused = true;
	return false;
}

STATIC bool App::Event_WindowMaximized(EventArgs& args)
{
	UNUSED(args);
	//DebuggerPrintf("Window Maximized!\n");
	return false;
}

STATIC bool App::Event_WindowRestored(EventArgs& args)
{
	UNUSED(args);
	//DebuggerPrintf("Window Restored!\n");
	g_theApp->m_isPaused = false;
	return false;
}

App::~App()
{
}

void App::Startup()
{
	LoadGameData();

	JobSystemConfig jobSystemConfig;
	jobSystemConfig.m_numWorkers = -1;
	g_theJobSystem = new JobSystem(jobSystemConfig);

	EventSystemConfig eventConfig;
	g_theEventSystem = new EventSystem(eventConfig);

	InputConfig inputConfig;
	g_theInput = new InputSystem(inputConfig);

	WindowConfig windowConfig;
	windowConfig.m_inputSystem = g_theInput;
	windowConfig.m_windowTitle = g_defaultConfigBlackboard->GetValue("windowTitle", "Unnamed Application");
	windowConfig.m_aspectRatio = g_defaultConfigBlackboard->GetValue("windowAspect", 2.f);
	windowConfig.m_fullScreen = g_defaultConfigBlackboard->GetValue("windowFullscreen", false);
	windowConfig.m_size = g_defaultConfigBlackboard->GetValue("windowSize", IntVec2(1500, 750));
	windowConfig.m_pos = g_defaultConfigBlackboard->GetValue("windowPosition", IntVec2(75, 150));
	g_theWindow = new Window(windowConfig);

	RenderConfig renderConfig;
	renderConfig.m_window = g_theWindow;
	g_theRenderer = new Renderer(renderConfig);

	DevConsoleConfig devConsoleConfig;
	devConsoleConfig.m_filePath = "Data/Fonts/SquirrelFixedFont.png";
	g_theConsole = new DevConsole(devConsoleConfig);
	g_theConsole->SetMode(DevConsoleMode::HIDDEN);

	AudioConfig audioConfig;
	g_theAudio = new AudioSystem(audioConfig);

	DebugRenderConfig debugConfig;
	debugConfig.m_renderer = g_theRenderer;

	m_clock = new Clock();
	m_game = new Game();

	g_theJobSystem->Startup();
	g_theEventSystem->StartUp();
	SubscribeEventCallbackFunction("Quit", App::Event_Quit);
	SubscribeEventCallbackFunction("WindowMinimized", App::Event_WindowMinimized);
	SubscribeEventCallbackFunction("WindowMaximized", App::Event_WindowMaximized);
	SubscribeEventCallbackFunction("WindowRestored", App::Event_WindowRestored);
	g_theConsole->Startup();
	g_theInput->Startup();
	g_theWindow->Startup();
	g_theRenderer->Startup();
	g_theAudio->Startup();
	m_game->Startup();
	Clock::TickSystemClock();

	DebugRenderSystemStartup(debugConfig);
}

void App::Shutdown()
{
	m_game->Shutdown();

	DebugRenderSystemShutdown();
	g_theAudio->Shutdown();
	g_theConsole->ShutDown();
	g_theRenderer->Shutdown();
	g_theWindow->Shutdown();
	g_theInput->Shutdown();
	g_theEventSystem->ShutDown();
	g_theJobSystem->ShutDown();

	SafeDelete(g_theAudio);
	SafeDelete(g_theConsole);
	SafeDelete(g_theRenderer);
	SafeDelete(g_theWindow);
	SafeDelete(g_theInput);
	SafeDelete(g_theEventSystem);
	SafeDelete(g_theJobSystem);
}

void App::RunFrame()
{
	BeginFrame();

	if (!m_isPaused)
	{
		Update();
		Render();
	}
	
	EndFrame();
}

void App::Run()
{
	// program main loop; keep running frames until it's time to quit
	while (!m_isQuitting)
	{
		RunFrame();
	}
}

void App::LoadGameData()
{
	tinyxml2::XMLDocument document;

	g_defaultConfigBlackboard = new NamedStrings();

	// Load and parse GameConfig.xml
	if (document.LoadFile("Data/GameConfig.xml") == tinyxml2::XML_SUCCESS)
	{
		// Get the root element 
		tinyxml2::XMLElement* rootElement = document.RootElement();
		GUARANTEE_OR_DIE(rootElement, "XML couldn't be loaded"); // if files doesn't exist then print this text

		// Populate g_gameConfigBlackboard with attributes
		g_defaultConfigBlackboard->PopulateFromXmlElementAttributes(*rootElement, false);
	}
}

bool App::HandleQuitRequested()
{
	m_isQuitting = true;
	return false;
}

void App::BeginFrame()
{
	g_theJobSystem->BeginFrame();
	g_theEventSystem->BeginFrame();
	g_theInput->BeginFrame();
	g_theWindow->BeginFrame();
	g_theRenderer->BeginFrame();
	g_theConsole->BeginFrame();
	g_theAudio->BeginFrame();
	Clock::TickSystemClock();
	DebugRenderBeginFrame();
}

void App::Update()
{
	if (g_theConsole->IsOpen() || m_game->m_currentState == GameState::ATTRACT)
	{
		g_theInput->SetCursorMode(false, false);
	}
	else
	{
		g_theInput->SetCursorMode(true, true);
	}

	m_game->UpdateGameMode();
}

void App::Render()
{
	g_theRenderer->ClearScreen(Rgba8::BLACK);
	m_game->Render();
	g_theConsole->Render(AABB2(Vec2(0.25f, 0.f), Vec2((float)g_theWindow->GetClientDimensions().x, (float)g_theWindow->GetClientDimensions().y)), g_theRenderer);
}

void App::EndFrame()
{
	DebugRenderEndFrame();
	g_theJobSystem->EndFrame();
	g_theEventSystem->EndFrame();
	g_theAudio->EndFrame();
	g_theConsole->EndFrame();
	g_theRenderer->EndFrame();
	g_theInput->EndFrame();
	g_theWindow->EndFrame();
}

void App::Restart()
{
	m_game->Shutdown();
	delete m_game;
	m_game = new Game();
	m_game->Startup();
}
