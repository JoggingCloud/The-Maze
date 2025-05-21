#include "Game/AIActor.hpp"
#include "Game/Actor.hpp"
#include "Game/Player.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"
#include "Game/App.hpp"
#include "Game/ActorDefinitions.hpp"
#include "Game/Weapon.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Core/DebugDraw.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Math/Mat44.hpp"
#include <algorithm>
#include <map>
#include <cmath>

AIActor::AIActor(Game* game, Map* map)
	:m_currentGame(game), m_currentMap(map)
{
	m_currentState = AIState::PATROL;

	g_rng.SetSeed(GetRandomSeedFromTime());
	m_repathPeriod = g_rng.RollRandomFloatInRange(2.5f, 4.5f);
	m_repathTimer = Timer(m_repathPeriod, g_theApp->m_game->m_clock);
	m_repathTimer.Start();
}

void AIActor::DamagedBy(Actor* actor)
{
	if (actor != nullptr)
	{
		m_targetActorUID = actor->GetUID();
	}
}

void AIActor::Update()
{
 	m_actor = m_currentMap->GetActorByUID(m_actorUID);
 	IntVec2 startPos = m_currentMap->GetTileCoordsForPos(GetActor()->m_position);
	m_aiStartPos = startPos;

	if (m_currentMap->m_hasPlayerReachedGoal) return;

	switch (m_currentState)
	{
	case AIState::PATROL:
		m_movementSpeed = m_actor->m_walkSpeed;
		m_aiExteriorSenseColor = Rgba8::GREEN;
		m_aiInteriorSenseColor = Rgba8::TRANSLUCENT_GREEN;
		g_rng.SetSeed(GetRandomSeedFromTime());
		m_patrolRange = g_rng.SRollRandomIntInRange(2, 10);
		PatrolArea(m_patrolRange, startPos);
		break;
	case AIState::CHASE:
		m_movementSpeed = m_actor->m_runSpeed;
		m_aiExteriorSenseColor = Rgba8::RED;
		m_aiInteriorSenseColor = Rgba8::TRANSLUCENT_RED;
		ChaseTarget();
		break;
	}

	while (true)
	{
		Job* completedJob = g_theJobSystem->RetrieveCompletedJob();
		if (!completedJob) break;

		AStarPathfindingJob* pathingJob = dynamic_cast<AStarPathfindingJob*>(completedJob);
		if (!pathingJob)
		{
			delete completedJob;
			continue;
		}

		// Ensure the AI who requested this job gets the path
		pathingJob->m_ai->m_aiPath = pathingJob->GetResult();
		pathingJob->m_ai->m_isWaitingForPath = false;
		delete pathingJob;
	}

	AStarUpdate();
 
	if (m_currentGame->m_player->m_isShowingDebugOptions)
	{
		DebugCurrentAIPath();
		DebugCurrentAIGoalPosition();
	}
 
	Vec3 enemyActorPos = m_actor->m_position + Vec3(0.f, 0.f, m_actor->m_eyeHeight);
	DebugAddWorld3DRing(enemyActorPos, m_sensorRadius, 16, 0.25f, 0.f, m_aiExteriorSenseColor, m_aiExteriorSenseColor, DebugRenderMode::ALWAYS);
	DebugAddWorld3DTriangle(m_actor->GetModelMatrix(), m_actor->m_orientation.GetForwardVector(), m_actor->m_eyeHeight, m_sightDistance, 0.f, m_aiInteriorSenseColor, m_aiInteriorSenseColor, DebugRenderMode::ALWAYS);
	DebugAddWorld3DWireTriangle(m_actor->GetModelMatrix(), m_actor->m_orientation.GetForwardVector(), m_actor->m_eyeHeight, m_sightDistance, 0.f, m_aiExteriorSenseColor, m_aiExteriorSenseColor, DebugRenderMode::ALWAYS);
}

void AIActor::AStarUpdate()
{
	if (!m_aiPath.empty())
	{
		IntVec2 nextPathTileCoords = m_aiPath.back();

		// Calculate the center of the next tile
		Vec3 nextTileCenter = Vec3((float)nextPathTileCoords.x + 0.5f, (float)nextPathTileCoords.y + 0.5f, 0.0f);

		// Calculate direction from the current position to the center of the next tile
		Vec3 directionToTarget = nextTileCenter - m_actor->m_position;

		float turnTowards = directionToTarget.GetAngleAboutZDegrees();
		float maxturnAngle = m_actor->m_turnSpeed * m_currentGame->m_clock->GetDeltaSeconds();

		m_actor->TurnInDirection(turnTowards, maxturnAngle);
		m_actor->MoveInDirection(directionToTarget, m_movementSpeed);

		float distanceSqToNextTileCenter = GetDistanceSquared3D(m_actor->m_position, nextTileCenter);

		float actorRadiusSq = m_actor->m_physicsRadius * m_actor->m_physicsRadius;
		if (distanceSqToNextTileCenter < actorRadiusSq)
		{
			m_aiPath.pop_back(); // Reached the next way point pop and continue along path
		}
	}
}

void AIActor::DebugCurrentAIPath() const
{
	if (m_currentMap->m_canSeeAiPath)
	{
		if (!m_aiPath.empty())
		{
			for (int i = 0; i < m_aiPath.size(); i++)
			{
				Vec3 pbottomLeft = Vec3((float)m_aiPath[i].x - 0.5f, (float)m_aiPath[i].y - 0.5f, 0.1f);
				Vec3 pbottomRight = Vec3((float)m_aiPath[i].x + 0.5f, (float)m_aiPath[i].y - 0.5f, 0.1f);
				Vec3 ptopRight = Vec3((float)m_aiPath[i].x + 0.5f, (float)m_aiPath[i].y + 0.5f, 0.1f);
				Vec3 ptopLeft = Vec3((float)m_aiPath[i].x - 0.5f, (float)m_aiPath[i].y + 0.5f, 0.1f);
				DebugAddWorldQuad(0.f, pbottomLeft, pbottomRight, ptopRight, ptopLeft, Rgba8::GREEN, Rgba8::GREEN, DebugRenderMode::USE_DEPTH);
				DebugAddWorldWireQuad(0.f, pbottomLeft, pbottomRight, ptopRight, ptopLeft, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::USE_DEPTH);
			}
		}
	}
}

void AIActor::DebugCurrentAIGoalPosition() const
{
	if (m_currentMap->m_canSeeAiGoalPosition)
	{
		Vec3 obottomLeft = Vec3((float)m_storedGoalPosition.x - 0.5f, (float)m_storedGoalPosition.y - 0.5f, 0.1f);
		Vec3 obottomRight = Vec3((float)m_storedGoalPosition.x + 0.5f, (float)m_storedGoalPosition.y - 0.5f, 0.1f);
		Vec3 otopRight = Vec3((float)m_storedGoalPosition.x + 0.5f, (float)m_storedGoalPosition.y + 0.5f, 0.1f);
		Vec3 otopLeft = Vec3((float)m_storedGoalPosition.x - 0.5f, (float)m_storedGoalPosition.y + 0.5f, 0.1f);
		DebugAddWorldQuad(m_repathTimer.m_period, obottomLeft, obottomRight, otopRight, otopLeft, Rgba8::PURPLE, Rgba8::PURPLE, DebugRenderMode::USE_DEPTH);
		DebugAddWorldWireQuad(m_repathTimer.m_period, obottomLeft, obottomRight, otopRight, otopLeft, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::USE_DEPTH);
	}
}

void AIActor::RequestPathfindingJob(IntVec2 startPoint, IntVec2 goalPoint)
{
	if (m_isWaitingForPath) return;

	AStarPathfindingJob* job = new AStarPathfindingJob(this, startPoint, goalPoint, m_currentMap->GetMapDimensions());
	g_theJobSystem->QueueJob(job);
	m_isWaitingForPath = true;
}

void AIActor::PatrolArea(int patrolRange, IntVec2 startPos)
{
	Actor* targetActor = g_theApp->m_game->m_currentMap->GetPlayerActor();
	if (targetActor == nullptr)
	{
		return;
	}

	if (!CanSeeTarget(targetActor))
	{
		IntVec2 currnetGoalPos = startPos;
		bool hasReachedGoal = true;

		if (hasReachedGoal && m_repathTimer.HasPeriodElapsed())
		{
			// Get random goal tile position within patrol range 
			currnetGoalPos = m_currentMap->GetRandomTilewithinRange(startPos, patrolRange);
			m_storedGoalPosition = currnetGoalPos;
			if (!m_currentMap->IsSolidTile(currnetGoalPos.x, currnetGoalPos.y))
			{
				RequestPathfindingJob(startPos, currnetGoalPos);
			}

			// Reset timer
			m_repathTimer.DecrementPeriodIfElapsed();
			hasReachedGoal = false;
		}

		// Calculate the center of the goal tile & check if we have reached the goal
		Vec3 goalTileCenter = Vec3((float)currnetGoalPos.x + 0.5f, (float)currnetGoalPos.y + 0.5f, 0.0f);
		float distanceSqToGoalTileCenter = GetDistanceSquared3D(GetActor()->m_position, goalTileCenter);
		float actorRadiusSq = GetActor()->m_physicsRadius * GetActor()->m_physicsRadius;

		if (distanceSqToGoalTileCenter < actorRadiusSq) hasReachedGoal = true;
	}
	else
	{
		m_currentState = AIState::CHASE;
	}
}

bool AIActor::CanSeeTarget(Actor* playerActor)
{
	FindTargetWithinLineOfSight(playerActor, m_sightDistance, m_sightFOV);
	if (m_detectedActor || m_HasBeenAlertedByTeammate) return true;
	return false;
}

void AIActor::FindTargetWithinLineOfSight(Actor* playerActor, float distance, float angle)
{
	m_actor = m_currentMap->GetActorByUID(m_actorUID);
	Vec3 eyePos = m_actor->m_position + Vec3(0.f, 0.f, m_actor->m_eyeHeight);
	Vec3 fwdDir = m_actor->m_orientation.GetForwardVector();

	playerActor = m_currentMap->GetPlayerActor();
	Vec3 displacement = (playerActor->m_position - m_actor->m_position).GetNormalized();
	float playerDistance = (playerActor->m_position - m_actor->m_position).GetLength();

	float targetAngle = displacement.GetAngleAboutZDegrees();
	float actorForwardAngle = fwdDir.GetAngleAboutZDegrees();
	float deltAngle = GetShortestAngularDispDegrees(actorForwardAngle, targetAngle);

	while (deltAngle < -180)
	{
		deltAngle += 360;
	}
	while (deltAngle >= 180)
	{
		deltAngle -= 360;
	}

	if (playerDistance <= distance)
	{
		if (fabsf(deltAngle) <= angle)
		{
			RaycastResult raycastResult = m_currentMap->RaycastAll(m_actor, eyePos, displacement, distance);
			DebugAddWorldLine(GetActor()->GetModelMatrix(), raycastResult.m_impactDist, 0.1f, 32, 0.f, m_aiExteriorSenseColor, m_aiExteriorSenseColor, DebugRenderMode::ALWAYS);
			if (raycastResult.m_didImpact && raycastResult.m_impactedActor && raycastResult.m_impactedActor->IsPlayer())
			{
				m_detectedActor = playerActor;
				return;
			}
		}
	}

	FindTargetWithinSensorRadius(playerActor, playerDistance);
}

void AIActor::FindTargetWithinSensorRadius(Actor* playerActor, float playerDistance)
{
	Vec3 eyePos = m_actor->m_position + Vec3(0.f, 0.f, m_actor->m_eyeHeight);
	Vec3 displacement = (playerActor->m_position - m_actor->m_position).GetNormalized();

	if (playerDistance <= m_sensorRadius)
	{
		RaycastResult raycastResult = m_currentMap->RaycastAll(m_actor, eyePos, displacement, m_sensorRadius);
		DebugAddWorldLine(GetActor()->GetModelMatrix(), raycastResult.m_impactDist, 0.1f, 32, 0.f, m_aiExteriorSenseColor, m_aiExteriorSenseColor, DebugRenderMode::ALWAYS);
		if (raycastResult.m_didImpact && raycastResult.m_impactedActor && raycastResult.m_impactedActor->IsPlayer())
		{
			m_detectedActor = playerActor;
			return;
		}
	}
}

void AIActor::ChaseTarget()
{
	Actor* targetActor = m_currentMap->GetPlayerActor();
	if (targetActor == nullptr) return;

	Vec3 target = targetActor->m_position;
	m_currentTargetTileCoords = m_currentMap->GetTileCoordsForPos(target);
	
	Vec3 directionToTarget = (target - m_actor->m_position);
	float distanceToTarget = directionToTarget.GetLength();

	if (!HasLostSightOfTarget(targetActor, m_sightDistance, m_sensorRadius))
	{
		m_alertTeammatesTimer -= m_currentGame->m_clock->GetDeltaSeconds();
		if (m_alertTeammatesTimer <= 0.f)
		{
			m_didAlertTeammates = true;
			AlertOtherAiAgents(targetActor);
			m_alertTeammatesTimer = m_resetAlertTimer;
		}

		if (m_currentTargetTileCoords != m_lastKnownTargetTileCoords)
		{
			RequestPathfindingJob(m_aiStartPos, m_currentTargetTileCoords);
		}

		if (distanceToTarget <= m_actor->m_currentWeapon->m_enemyMeleeRange)
		{
			m_actor->Attack();
		}
	}
	else if (m_currentState == AIState::CHASE && HasLostSightOfTarget(targetActor, m_sightDistance, m_sensorRadius) && m_didAlertTeammates || m_HasBeenAlertedByTeammate)
	{
		m_chaseTimer -= m_currentGame->m_clock->GetDeltaSeconds();
		if (m_chaseTimer <= 0.f)
		{
			m_detectedActor = nullptr;
			m_HasBeenAlertedByTeammate = false;
			m_didAlertTeammates = false;
			m_chaseTimer = m_resetChasetimer;
			
			m_switchCurrentStateTimer -= m_currentGame->m_clock->GetDeltaSeconds();
			if (m_switchCurrentStateTimer <= 0.f)
			{
				m_currentState = AIState::PATROL;
				m_switchCurrentStateTimer = m_restSwitchStateTimer;
				return;
			}
		}
		else
		{
			if (m_currentTargetTileCoords != m_lastKnownTargetTileCoords)
			{
				RequestPathfindingJob(m_aiStartPos, m_currentTargetTileCoords);
			}

			if (distanceToTarget <= m_actor->m_currentWeapon->m_enemyMeleeRange)
			{
				m_actor->Attack();
			}
		}
	}
	else
	{
		// Individual AI check if it should revert to patrol state
		if (!m_didAlertTeammates && !m_HasBeenAlertedByTeammate)
		{
			m_losePlayerTimer -= m_currentGame->m_clock->GetDeltaSeconds();
			m_aiExteriorSenseColor = Rgba8::YELLOW;
			m_aiInteriorSenseColor = Rgba8::TRANSLUCENT_YELLOW;
			if (m_losePlayerTimer <= 0.f)
			{
				m_detectedActor = nullptr;
				
				m_switchStateTimer -= m_currentGame->m_clock->GetDeltaSeconds();
				if (m_switchStateTimer <= 0.f)
				{
					m_currentState = AIState::PATROL;
					m_losePlayerTimer = m_resetLosePlayerTimer;
					m_switchStateTimer = 1.f;
					return;
				}
			}
		}
		else
		{
			// Check for other AI agents; This AI agent stays in the chase state
			if (HasAllAIAgentsLostSightOfPlayer(targetActor))
			{
				m_losePlayerTimer -= m_currentGame->m_clock->GetDeltaSeconds();
				m_aiExteriorSenseColor = Rgba8::YELLOW;
				m_aiInteriorSenseColor = Rgba8::TRANSLUCENT_YELLOW;
				if (m_losePlayerTimer <= 0.f)
				{
					for (const Actor* actor : m_currentMap->m_actors) // Const because the num actors are not going to change since they can't die
					{
						if (actor->m_isAI)
						{
							AIActor* aiController = actor->GetAiController();
							aiController->m_detectedActor = nullptr;
							aiController->m_HasBeenAlertedByTeammate = false; // Reset the alert status
							
							m_switchStateTimer -= m_currentGame->m_clock->GetDeltaSeconds();
							if (m_switchStateTimer <= 0.f)
							{
								aiController->m_currentState = AIState::PATROL;
								aiController->m_losePlayerTimer = aiController->m_resetLosePlayerTimer;
								m_switchStateTimer = 1.f;
								return;
							}
						}
					}
				}
			}
		}
	}
}

void AIActor::AlertOtherAiAgents(Actor* playerActor)
{
	for (const Actor* actor : m_currentMap->m_actors)
	{
		if (actor)
		{
			if (actor->m_isAI)
			{
				AIActor* aiController = actor->GetAiController();
				if (aiController->m_currentState != AIState::CHASE)
				{
					aiController->m_currentState = AIState::CHASE;
					aiController->m_HasBeenAlertedByTeammate = true;
					aiController->m_detectedActor = playerActor;
					aiController->m_losePlayerTimer = aiController->m_resetLosePlayerTimer;
				}
			}
		}
	}
}

bool AIActor::HasLostSightOfTarget(Actor* playerActor, float fwdSightDistance, float innerSensorRadius) const
{
	float playerDistance = (playerActor->m_position - m_actor->m_position).GetLength();
	if (playerDistance > fwdSightDistance && playerDistance > innerSensorRadius) return true;
	return false;
}

bool AIActor::HasAllAIAgentsLostSightOfPlayer(Actor* playerActor)
{
	if (!CanSeeTarget(playerActor))
	{
		for (const Actor* actor : m_currentMap->m_actors) // Const because the num actors are not going to change since they can't die
		{
			if (actor->m_isAI)
			{
				AIActor* aiController = actor->GetAiController();
				if (aiController->m_currentState == AIState::CHASE && aiController->CanSeeTarget(playerActor)) return false; // If any AI Agent still sees the player return false
			}
		}
		return true; // If no AI Agents can see the player return true
	}
	return false; // Main AI Agent still sees the player
}

void AStarPathfindingJob::Execute()
{
	GridAStar pathfinder(m_mapDimensions);
	Map* map = m_ai->m_currentMap;
	pathfinder.SetDirectionMode(DirectionMode::Cardinal8);
	pathfinder.SetIsSolidCallback([map](IntVec2 coords) { return map->IsSolidTile(coords.x, coords.y); });
	pathfinder.SetCanMoveDiagonalCallback([map](IntVec2 from, IntVec2 to)
		{
			IntVec2 direction = to - from;
			if (direction.x != 0 && direction.y != 0)
			{
				IntVec2 adjacent1 = from + IntVec2(direction.x, 0);
				IntVec2 adjacent2 = from + IntVec2(0, direction.y);
				return !map->IsSolidTile(adjacent1.x, adjacent1.y) && !map->IsSolidTile(adjacent2.x, adjacent2.y);
			}
			return true; // Its not a valid move to go diagonal
		});
	pathfinder.ComputeAStar(m_start, m_goal, m_resultPath);
	m_state = JobStatus::COMPLETED;
}
