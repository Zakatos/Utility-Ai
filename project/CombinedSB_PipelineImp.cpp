#include "stdafx.h"
#include "CombinedSB_PipelineImp.h"
#include "CombinedSB_Pipeline.h"

namespace CombinedSB
{
	//---------------
	//1. TARGETER

	Goal& FixedGoalTargeter::GetGoalRef()
	{
		return m_Goal;
	}

	Goal FixedGoalTargeter::GetGoal()
	{
		return m_Goal;
	}

	//---------------
	//3. CONSTRAINTS

	float AvoidAgentConstraint::WillViolate(const Path* pPath, AgentInfo* pAgent, float maxPriority)
	{
		float currPriority = 0.f;
		float smallestPriority = (std::numeric_limits<float>::max)();
		for (auto a : m_agents)
		{
			currPriority = WillViolateAgent(pPath, pAgent, maxPriority, a);
			if (currPriority < smallestPriority)
			{
				smallestPriority = currPriority;
			}
		}
		return smallestPriority;
	}

	Goal AvoidAgentConstraint::Suggest(const Path* pPath)
	{
		return m_SuggestGoal;
	}

	float AvoidAgentConstraint::WillViolateAgent(const Path* pPath, AgentInfo* pAgent, float maxPriority, Elite::Vector2& agent)
	{
		auto goal = pPath->GetGoal();

		if (!goal.PositionSet)
			return (std::numeric_limits<float>::max)();

		auto a2g = goal.Position - pAgent->Position;
		if (a2g.SqrtMagnitude() > 10)//if this close then just go
		{
			auto a2s = agent - pAgent->Position;
			auto moveDirection = a2g;
			moveDirection.Normalize();

			auto distanceToClosest = Elite::Dot(a2s, moveDirection);
			auto d = a2s.Magnitude() - (distanceToClosest);

			//check for collision
			auto radius = 20 + m_AvoidMargin;
			if (d < radius)
			{
				
				if (distanceToClosest > 0.f && distanceToClosest < maxPriority)
				{
					//Find closest point
					auto closestPoint = pAgent->Position + (distanceToClosest * moveDirection);

					//find the point of avoidance
					auto offsetDir = closestPoint - agent;
					offsetDir.Normalize();

					m_SuggestGoal.Position = agent + (radius * offsetDir);
					m_SuggestGoal.PositionSet = true;

					return distanceToClosest;
				}
			}
		}

		return (std::numeric_limits<float>::max)();
	}

	//---------------
	//4. ACTUATOR

	Path* BasicActuator::CreatePath()
	{
		delete(m_pPath);
		m_pPath = new Path();
		return m_pPath;
	}

	void BasicActuator::UpdatePath(Path* pPath, AgentInfo* pAgent, const Goal& goal)
	{
		pPath->SetGoal(goal);
		pPath->SetAgent(pAgent);
	}

	SteeringPlugin_Output BasicActuator::CalculateSteering(const Path* pPath, float deltaT, AgentInfo* pAgent)
	{
		auto goal = pPath->GetGoal();

#ifdef _DEBUG
		//Context::Current.renderer->DrawSolidCircle(goal.Position, 0.5f, { 0,0 }, { 0,1,1 });
#endif

		if (goal.PositionSet && m_pSeekingBehaviour)
		{
			auto newTarget = goal.Position;
			m_pSeekingBehaviour->SetTarget(&newTarget);
			return m_pSeekingBehaviour->CalculateSteering(deltaT, pAgent);
		}
		return m_pWanderBehaviour->CalculateSteering(deltaT, pAgent);
	}
}

