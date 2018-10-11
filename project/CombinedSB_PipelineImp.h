#pragma once
#include <vector>
#include "SteeringBehaviours.h"
#include "CombinedSB_Pipeline.h"
namespace CombinedSB
{
	//---------------
	//1. TARGETER
	class FixedGoalTargeter : public Targeter
	{
	public:
		Goal & GetGoalRef();
		Goal GetGoal()override;
	private:
		Goal m_Goal = {};
	};

	//---------------
	//2. DECOMPOSER



	//---------------
	//3. CONSTRAINTS
	struct SphereObstacle
	{
		Elite::Vector2 Position;
		float Radius;
	};

	class AvoidAgentConstraint : public Constraint
	{
	public:
		//constructor
		AvoidAgentConstraint() : Constraint() {}

		//functions
		float WillViolate(const Path* pPath, AgentInfo* pAgent, float maxPriority) override;
		Goal Suggest(const Path* pPath) override;
		std::vector<Elite::Vector2>& GetAgentsRef() { return m_agents; };

	private:
		//datamembers
		std::vector<Elite::Vector2 > m_agents = {};
		float m_AvoidMargin = 2.0f;
		Goal m_SuggestGoal = {};

		//functions
		float WillViolateAgent(const Path* pPath, AgentInfo* pAgent, float maxPriority,
			Elite::Vector2& agent);
	};

	//---------------
	//4. ACTUATOR
	class BasicActuator : public Actuator
	{
	public:

		//constructor
		BasicActuator(SteeringBehaviours::Seek* pSeekBehaviour, SteeringBehaviours::Wander* pWanderBehaviour) : Actuator(),
			m_pSeekingBehaviour(pSeekBehaviour),
			m_pWanderBehaviour(pWanderBehaviour)
		{}
		virtual ~BasicActuator() { delete(m_pPath); }

		//functions
		Path* CreatePath() override;
		void UpdatePath(Path* pPath, AgentInfo* pAgent, const Goal& goal) override;
		SteeringPlugin_Output CalculateSteering(const Path* pPath, float deltaT, AgentInfo* pAgent) override;
	private:
		SteeringBehaviours::Seek* m_pSeekingBehaviour = nullptr;
		SteeringBehaviours::Wander* m_pWanderBehaviour = nullptr;
		Path* m_pPath = nullptr;
	};
}
