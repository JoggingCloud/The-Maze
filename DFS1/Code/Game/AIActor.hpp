#pragma once
#include "Game/Controller.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/JobSystem.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/AI/Pathfinding/Grid/GridAStar.hpp"
#include <vector>
#include <queue>

class Game;
class Map;
class Timer;

constexpr int MAX_DISTANCE_THRESHOLD = 32;

enum class AIState
{
	PATROL,
	CHASE,
	NONE
};

class AIActor : public Controller
{
public:
	AIActor() = default;
	AIActor(Game* game, Map* map);
	virtual ~AIActor() = default;

	virtual void DamagedBy(Actor* actor) override;
	virtual void Update() override;

	void AStarUpdate();

	void DebugCurrentAIPath() const;
	void DebugCurrentAIGoalPosition() const;
	
	// A-Star
	void RequestPathfindingJob(IntVec2 startPoint, IntVec2 goalPoint);

	// Patrol state
	void PatrolArea(int patrolRange, IntVec2 startPos);
	bool CanSeeTarget(Actor* playerActor);
	void FindTargetWithinLineOfSight(Actor* playerActor, float distance, float angle);
	void FindTargetWithinSensorRadius(Actor* playerActor, float playerDistance);

	// Chase State
	void ChaseTarget();
	void AlertOtherAiAgents(Actor* playerActor);
	bool HasLostSightOfTarget(Actor* playerActor, float fwdSightDistance, float innerSensorRadius) const;
	bool HasAllAIAgentsLostSightOfPlayer(Actor* playerActor);

public:
	Game* m_currentGame = nullptr;
	Map* m_currentMap = nullptr;
	
	bool m_didAlertTeammates = false;
	bool m_HasBeenAlertedByTeammate = false;
	int m_patrolRange = 0; // Make patrol range random between two values
	IntVec2 m_storedGoalPosition = IntVec2::ZERO;

	float m_sensorRadius = 3.f;
	float m_sightDistance = 10.f;
	float m_sightFOV = 30.f;
	Rgba8 m_aiExteriorSenseColor;
	Rgba8 m_aiInteriorSenseColor;
	
	float m_movementSpeed = 0;
	
	Timer m_repathTimer;
	float m_repathPeriod = 0;
	
	float m_losePlayerTimer = 5.f;
	float m_resetLosePlayerTimer = 5.f;

	float m_alertTeammatesTimer = 2.5f;
	float m_resetAlertTimer = 2.5f;

	float m_chaseTimer = 5.f;
	float m_resetChasetimer = 5.f;

	float m_switchCurrentStateTimer = 1.f;
	float m_restSwitchStateTimer = 1.f;

	float m_switchStateTimer = 1.f;

public:
	std::vector<IntVec2> m_aiPath;

	size_t m_pathIndex = 0;
	std::vector<Node> m_nodeGrid;
	
	IntVec2 m_currentTargetTileCoords = IntVec2::ZERO;
	IntVec2 m_lastKnownTargetTileCoords = IntVec2::ZERO;
	const IntVec2 INVALID_POSITION = IntVec2(-999, -999);
	
	ActorUID m_targetActorUID = ActorUID::INVALID;
	Actor* m_actor = nullptr;
	Actor* m_detectedActor = nullptr;
	IntVec2 m_aiStartPos = IntVec2::ZERO;

private:
	bool m_isWaitingForPath = false; // Tracks if a path request is in progress
	bool m_isPathStored = false;
	std::vector<IntVec2> m_currentPth;
	IntVec2 m_currentStartPos;
	IntVec2 m_currentTargetPos;

public:
	AIState m_currentState = AIState::NONE;
};

class AStarPathfindingJob : public Job
{
public:
	AStarPathfindingJob(AIActor* ai, IntVec2 start, IntVec2 goal, IntVec2 mapDimensions)
		: m_ai(ai), m_start(start), m_goal(goal), m_mapDimensions(mapDimensions) { m_state = JobStatus::NEW; }

	virtual void Execute() override;

	std::vector<IntVec2> GetResult() const { return m_resultPath; }

public:
	AIActor* m_ai = nullptr;
	IntVec2 m_start = IntVec2::ZERO;
	IntVec2 m_goal = IntVec2::ZERO;
	IntVec2 m_mapDimensions = IntVec2::ZERO;
	std::vector<IntVec2> m_resultPath;
};
