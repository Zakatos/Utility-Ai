#pragma once
#include "SteeringBehaviours.h"


namespace CombinedSB
{
	using namespace SteeringBehaviours;

	//PIPELINE HELPERS
	//****************
#pragma region ***Goal***
	struct Goal
	{
		//Four seperate controllable channels
		Elite::Vector2 Position = { 0.f, 0.f };
		bool PositionSet = false;

		float Orientation = 0.f;
		bool OrientationSet = false;

		Elite::Vector2 Velocity = { 0.f, 0.f };
		bool VelocitySet = false;

		float Rotation = 0.f;
		bool RotationSet = false;

		Goal() {};

		//Reset Goal's Channels
		void Clear()
		{
			Position = Elite::Vector2();
			PositionSet = false;

			Velocity = Elite::Vector2();
			VelocitySet = false;

			Orientation = 0.f;
			OrientationSet = false;

			Rotation = 0.f;
			RotationSet = false;
		}

		//Update Goal
		void UpdateGoal(const Goal& goal)
		{
			if (goal.PositionSet)
			{
				Position = goal.Position;
				PositionSet = true;
			}
			if (goal.OrientationSet)
			{
				Orientation = goal.Orientation;
				OrientationSet = true;
			}
			if (goal.VelocitySet)
			{
				Velocity = goal.Velocity;
				VelocitySet = true;
			}
			if (goal.RotationSet)
			{
				Rotation = goal.Rotation;
				RotationSet = true;
			}
		}

		//Merge Goal
		bool CanMergeGoal(const Goal& goal)
		{
			return !(
				PositionSet && goal.PositionSet ||
				OrientationSet && goal.OrientationSet ||
				VelocitySet && goal.VelocitySet ||
				RotationSet && goal.RotationSet
				);
		}

	};
#pragma endregion

#pragma region ***Path***
	//SteeringPipeline Internal Path Segment (Path to (sub)goal)
	class Path
	{
	public:
		virtual ~Path() {}
		void SetGoal(Goal goal) { m_Goal = goal; }
		Goal GetGoal()const { return m_Goal; }
		void SetAgent(AgentInfo* pAgent) { m_pAgent = pAgent; }
		virtual float GetMaxPriority();

	protected:
		float m_MaxPriority = 50.f;
		Goal m_Goal = {};
		AgentInfo* m_pAgent = nullptr;
	};
#pragma endregion

	//1. TARGETER(S)
	//**************
#pragma region ***[PIPELINE-PART] Targeter***
	class Targeter
	{
	public:
		virtual ~Targeter() {}

		//Targeter Functions
		virtual Goal GetGoal() = 0;
	};
#pragma endregion

	//2. DECOMPOSER(S)
	//****************
#pragma region ***[PIPELINE-PART] Decomposer***
	class Decomposer
	{
	public:
		virtual ~Decomposer() {}

		//Decomposer Functions
		virtual Goal DecomposeGoal(const Goal& goal) = 0;
	};
#pragma endregion

	//3. CONSTRAINT(S)
	//****************
#pragma region ***[PIPELINE-PART] Constraint***
	class Constraint
	{
	public:
		virtual ~Constraint() {}

		//Constraint Functions
		virtual float WillViolate(const Path* pPath, AgentInfo* pAgent, float maxPriority) = 0;
		virtual Goal Suggest(const Path* pPath) = 0;
		void SetSuggestionUsed(bool val) { m_SuggestionUsed = val; }

	private:
		bool m_SuggestionUsed = false;
	};
#pragma endregion

	//4. ACTUATOR
	//***********
#pragma region ***[PIPELINE-PART] Actuator***
	class Actuator
	{
	public:
		virtual ~Actuator() {}

		//Actuator Functions
		virtual Path* CreatePath() = 0;
		virtual void UpdatePath(Path* pPath, AgentInfo* pAgent, const Goal& goal) = 0;
		virtual SteeringPlugin_Output CalculateSteering(const Path* pPath, float deltaT, AgentInfo* pAgent) = 0;
	};
#pragma endregion

	//THE PIPELINE
	//***********
#pragma region ***STEERING PIPELING***
	class SteeringPipeline : public ISteeringBehaviour
	{
	public:
		virtual ~SteeringPipeline() {};

		//SteeringPipeline Functions
		void SetActuator(Actuator* pActuator) { m_pActuator = pActuator; delete(m_pPath); }
		void SetTargeters(std::vector<Targeter*> targeters) { m_Targeters = targeters; }
		void SetDecomposers(std::vector<Decomposer*> decomposers) { m_Decomposers = decomposers; }
		void SetConstraints(std::vector<Constraint*> constraints) { m_Constraints = constraints; }
		void SetFallBack(ISteeringBehaviour* pFallback) { m_pFallbackBehaviour = pFallback; }

		//SteeringPipeline Behaviour
		SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;

	private:

		Path * m_pPath = nullptr;
		Actuator* m_pActuator = nullptr;
		std::vector<Targeter*> m_Targeters = {};
		std::vector<Decomposer*> m_Decomposers = {};
		std::vector<Constraint*> m_Constraints = {};

		UINT m_MaxConstraintSteps = 10;
		ISteeringBehaviour* m_pFallbackBehaviour = nullptr;
	};
#pragma endregion
}