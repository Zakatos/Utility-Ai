#pragma once
#include "SteeringBehaviours.h"

namespace CombinedSB
{
	using namespace SteeringBehaviours;

#pragma region ***WEIGHTED BLENDING***
	struct BehaviourAndWeight
	{
		ISteeringBehaviour* pBehaviour;
		float Weight;

		BehaviourAndWeight(ISteeringBehaviour* pBeh, float weight) :
			pBehaviour(pBeh),
			Weight(weight)
		{};
	};

	class BlendedSteering : public ISteeringBehaviour
	{
	public:
		BlendedSteering(std::vector<BehaviourAndWeight> weightedBehaviours) :m_WeightedBehavioursVec(weightedBehaviours) {};
		virtual ~BlendedSteering() {};

		//BlendedSteering Functions
		void AddBehaviour(BehaviourAndWeight pair) { m_WeightedBehavioursVec.push_back(pair); }

		//BlendedSteering Behaviour
		SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;

	private:
		std::vector<BehaviourAndWeight> m_WeightedBehavioursVec = {};
	};
#pragma endregion

#pragma region PRIORITY
	class PrioritySteering : public ISteeringBehaviour
	{
	public:
		PrioritySteering(std::vector<ISteeringBehaviour*> behaviours) :m_Behaviours(behaviours) {}
		virtual ~PrioritySteering() {}

		//PrioritySteering Behaviour
		SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;

	private:
		std::vector<ISteeringBehaviour*> m_Behaviours = {};
		float m_Epsilon = 0.0001f;
	};
#pragma endregion

#pragma region BLENDED_PRIORITY
	class BlendedPrioritySteering : public ISteeringBehaviour
	{
	public:
		BlendedPrioritySteering(std::vector<BlendedSteering*> priorityGroups) :m_PriorityGroups(priorityGroups) {}
		virtual ~BlendedPrioritySteering() {}

		//PrioritySteering Behaviour
		SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;

	private:
		std::vector<BlendedSteering*> m_PriorityGroups = {};
		float m_Epsilon = 0.0001f;
	};
#pragma endregion
}
