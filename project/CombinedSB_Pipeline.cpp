#include "stdafx.h"
#include "CombinedSB_Pipeline.h"



namespace CombinedSB
{
	//PATH (GetMaxPriority - Default Implementation)
	//****
	float Path::GetMaxPriority() //Distance between (sub)goal and character (current path)
	{
		if (!m_pAgent)
			return 0.f;

		return (m_pAgent->Position - m_Goal.Position).SqrtMagnitude();
	}

	//STEERING PIPELINE - Calculate Steering
	//**************************************
	SteeringPlugin_Output SteeringPipeline::CalculateSteering(float deltaT, AgentInfo* pAgent)
	{
		Goal currGoal = {}; //Goal used by the pipeline

							//1. TARGETER
							//Try to merge the targeter's goals
		for (auto pTargeter : m_Targeters)
		{
			Goal targeterGoal = pTargeter->GetGoal();
			if (currGoal.CanMergeGoal(targeterGoal))
			{
				currGoal.UpdateGoal(targeterGoal);
			}
		}




		//2. DECOMPOSER
		//Decompose the current goal
		for (auto pDecomposer : m_Decomposers)
		{
			currGoal = pDecomposer->DecomposeGoal(currGoal);
		}

		//3. PATH & CONSTRAINTS
		if (!m_pPath) m_pPath = m_pActuator->CreatePath();

		float shortestViolation, currentViolation, maxViolation;

		auto output = SteeringPlugin_Output();

		for (UINT i = 0; i < m_MaxConstraintSteps; ++i)
		{
			//Get the path to the goal
			m_pActuator->UpdatePath(m_pPath, pAgent, currGoal);

			//Find violating constraint
			shortestViolation = maxViolation = m_pPath->GetMaxPriority();
			for (auto pConstraint : m_Constraints)
			{
				if (i == 0)
					pConstraint->SetSuggestionUsed(false);

				//Check if this constraint is violated earlier than the (current) shortestViolation
				currentViolation = pConstraint->WillViolate(m_pPath, pAgent, shortestViolation);
				if (currentViolation > 0 && currentViolation < shortestViolation)
				{
					shortestViolation = currentViolation; //Constraint Violation
				}

				//Check for violations
				if (shortestViolation < maxViolation)
				{
					//Change the goal (based on violating constraint)
					currGoal = pConstraint->Suggest(m_pPath);
					pConstraint->SetSuggestionUsed(true);
					output.RunMode = true;
				}
				else
				{
					//Found a goal (in peace with all constraints)
					auto o = m_pActuator->CalculateSteering(m_pPath, deltaT, pAgent);
					output.LinearVelocity = o.LinearVelocity;
					output.AngularVelocity = o.AngularVelocity;
					output.AutoOrientate = o.AutoOrientate;
					return output;
				}
			}
		}

		//ConstraintStep reached (no solution found, use fallback behaviour for now)
		if (m_pFallbackBehaviour)
			return m_pFallbackBehaviour->CalculateSteering(deltaT, pAgent);

		return SteeringPlugin_Output(); //Empty steeringoutput (Ultra-Fallback...)
	}
}

