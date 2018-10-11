#include "stdafx.h"
#include "SteeringBehaviours.h"


namespace SteeringBehaviours
{
	//SEEK
	//****
	SteeringPlugin_Output Seek::CalculateSteering(float deltaT, AgentInfo* pAgent)
	{
		SteeringPlugin_Output steering = {};

		auto targetVelocity = (*m_pTargetRef) - pAgent->Position;
		targetVelocity.Normalize();
		targetVelocity *= pAgent->MaxLinearSpeed;

		steering.LinearVelocity = targetVelocity - pAgent->LinearVelocity;

		return steering;
	}

	//FLEE
	//****
	SteeringPlugin_Output Flee::CalculateSteering(float deltaT, AgentInfo* pAgent)
	{
		SteeringPlugin_Output steering = {};

		auto targetVelocity = pAgent->Position - (*m_pTargetRef);
		targetVelocity.Normalize();
		targetVelocity *= pAgent->MaxLinearSpeed;

		steering.LinearVelocity = targetVelocity - pAgent->LinearVelocity;

		return steering;
	}

	//WANDER
	//******
	SteeringPlugin_Output Wander::CalculateSteering(float deltaT, AgentInfo* pAgent)
	{
		auto offset = pAgent->LinearVelocity;
		offset.Normalize();
		offset *= m_Offset;

		b2Vec2 circleOffset = { cos(m_WanderAngle) * m_Radius, sin(m_WanderAngle) * m_Radius };

		m_WanderAngle += Elite::randomFloat() * m_AngleChange - (m_AngleChange * .5f); //RAND[-angleChange/2,angleChange/2]

		auto newTarget = new Elite::Vector2(pAgent->Position + offset + circleOffset);
		Seek::m_pTargetRef = newTarget;

		return Seek::CalculateSteering(deltaT, pAgent);
	}

	//ARRIVE
	//******
	SteeringPlugin_Output Arrive::CalculateSteering(float deltaT, AgentInfo* pAgent)
	{
		SteeringPlugin_Output steering = {};

		auto targetVelocity = (*m_pTargetRef - pAgent->Position);
		auto distance = targetVelocity.Normalize() - m_TargetRadius;

		if (distance < m_SlowRadius) //Inside SlowRadius
		{
			targetVelocity *= pAgent->MaxLinearSpeed * (distance / (m_SlowRadius + m_TargetRadius));
		}
		else
		{
			targetVelocity *= pAgent->MaxLinearSpeed;
		}

		//Calculate Steering
		steering.LinearVelocity = targetVelocity - pAgent->LinearVelocity;

		return steering;
	}
}
