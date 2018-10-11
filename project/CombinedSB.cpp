#include "stdafx.h"
#include "CombinedSB.h"
#include <Exam_HelperStructs.h>


namespace CombinedSB
{
	//BLENDED STEERING
	//****************
	SteeringPlugin_Output BlendedSteering::CalculateSteering(float deltaT, AgentInfo* pAgent)
	{
		SteeringPlugin_Output steering = {};
		auto totalWeight = 0.f;

		for (auto pair : m_WeightedBehavioursVec)
		{
			auto retSteering = pair.pBehaviour->CalculateSteering(deltaT, pAgent);
			steering.LinearVelocity += pair.Weight * retSteering.LinearVelocity;
			steering.AngularVelocity += pair.Weight * retSteering.AngularVelocity;

			totalWeight += pair.Weight;
		}

		if (totalWeight > 0.f)
		{
			auto scale = 1.f / totalWeight;
			steering.LinearVelocity *= scale;
			steering.AngularVelocity *= scale;
		}

		return steering;
	}

	//PRIORITY STEERING
	//*****************
	SteeringPlugin_Output PrioritySteering::CalculateSteering(float deltaT, AgentInfo* pAgent)
	{
		SteeringPlugin_Output steering = {};

		for (auto pBehaviour : m_Behaviours)
		{
			steering = pBehaviour->CalculateSteering(deltaT, pAgent);

			if (!((steering.LinearVelocity.SqrtMagnitude() + (steering.AngularVelocity*steering.AngularVelocity)) < (m_Epsilon*m_Epsilon)))
				break;
		}

		//If non of the behaviour return a valid output, last behaviour is returned
		return steering;
	}

	//BLENDED-PRIORITY STEERING
	//*************************
	SteeringPlugin_Output BlendedPrioritySteering::CalculateSteering(float deltaT, AgentInfo* pAgent)
	{
		SteeringPlugin_Output steering = {};

		for (auto pBlendedBehaviour : m_PriorityGroups)
		{
			steering = pBlendedBehaviour->CalculateSteering(deltaT, pAgent);

			if (!((steering.LinearVelocity.SqrtMagnitude() + (steering.AngularVelocity*steering.AngularVelocity)) < (m_Epsilon*m_Epsilon)))
				break;
		}

		//If none of the behaviour return a valid output, last behaviour is returned
		return steering;
	}
}
