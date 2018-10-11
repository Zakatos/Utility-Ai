#pragma once
#include "stdafx.h"
#include "BlackBoard.h"
#include "SteeringBehaviours.h"
#include <iostream>
#include "Plugin.h"
#include "House.h"
#include "CombinedSB_PipelineImp.h"


//*** GENERAL BEHAVIORS ***
enum BehaviourState
{
	Failure,
	Success,
	Running
};



//ACTIONS

inline BehaviourState Seek(Blackboard* pBlackboard)
{
	//get data
#pragma region DATA
	AgentInfo* pAgent = nullptr;
	CombinedSB::FixedGoalTargeter* pTargeter = nullptr;
	CombinedSB::SteeringPipeline* pSteering = nullptr;
	SteeringPlugin_Output* output = {};
	float* deltaT = 0;
	Elite::Vector2* target = nullptr;

	auto result = pBlackboard->GetData("Agent", pAgent)
		&& pBlackboard->GetData("Targeter", pTargeter)
		&& pBlackboard->GetData("SteeringPipeline", pSteering)
		&& pBlackboard->GetData("SteeringPlugin_Output", output)
		&& pBlackboard->GetData("deltaT", deltaT)
		&& pBlackboard->GetData("TargetPosition", target);

	if (!result)
		return Failure;

	if (!pAgent || !pTargeter || !pSteering)
		return Failure;

	IExamInterface* m_pInterface;
	result = pBlackboard->GetData("IExamInterface", m_pInterface);
	if (!result)
		return Failure;
#pragma endregion 

	*target = m_pInterface->NavMesh_GetClosestPathPoint(*target);
	pTargeter->GetGoalRef().Position = *target;
	pTargeter->GetGoalRef().PositionSet = true;
	*output = pSteering->CalculateSteering(*deltaT, pAgent);
	std::cout << "I am Seeking shelter" << std::endl;

	return Success;

}

inline BehaviourState Wander(Blackboard* pBlackboard)
{
	//get data
#pragma region DATA
	AgentInfo* pAgent = nullptr;
	IExamInterface* m_pInterface;
	CombinedSB::FixedGoalTargeter* pTargeter = nullptr;
	CombinedSB::SteeringPipeline* pSteering = nullptr;
	SteeringBehaviours::ISteeringBehaviour* pWanderBehaviour = nullptr;
	SteeringPlugin_Output* output = {};
	float* deltaT = 0;

	auto result = pBlackboard->GetData("Agent", pAgent)
		&& pBlackboard->GetData("Targeter", pTargeter)
		&& pBlackboard->GetData("WanderBehaviour", pWanderBehaviour)
		&& pBlackboard->GetData("SteeringPipeline", pSteering)
		&& pBlackboard->GetData("SteeringPlugin_Output", output)
		&& pBlackboard->GetData("deltaT", deltaT)
		&& pBlackboard->GetData("IExamInterface", m_pInterface);
	if (!result)
		return Failure;
#pragma endregion 

	if (!pAgent || !pSteering || !pTargeter)
		return Failure;
	pWanderBehaviour->CalculateSteering(*deltaT, pAgent);
	auto target = m_pInterface->NavMesh_GetClosestPathPoint(*pWanderBehaviour->GetTargetPos());
	pTargeter->GetGoalRef().Position = target;
	pTargeter->GetGoalRef().PositionSet = true;
	*output = pSteering->CalculateSteering(*deltaT, pAgent);
	output->AutoOrientate = true;
	std::cout << "Wandering" << std::endl;

	return Success;
}

inline BehaviourState Scout(Blackboard* pBlackboard)
{
	//get data
#pragma region DATA
	AgentInfo* pAgent = nullptr;
	Elite::Vector2* worldDimensions = nullptr;
	Elite::Vector2* worldCenter = nullptr;
	int* scout = nullptr;
	IExamInterface* m_pInterface = nullptr;
	CombinedSB::FixedGoalTargeter* pTargeter = nullptr;
	CombinedSB::SteeringPipeline* pSteering = nullptr;
	SteeringPlugin_Output* output = {};
	float* deltaT = 0;

	auto result = pBlackboard->GetData("Agent", pAgent)
		&& pBlackboard->GetData("Targeter", pTargeter)
		&& pBlackboard->GetData("WorldDimensions", worldDimensions)
		&& pBlackboard->GetData("worldCenter", worldCenter)
		&& pBlackboard->GetData("scoutCount", scout)
		&& pBlackboard->GetData("SteeringPipeline", pSteering)
		&& pBlackboard->GetData("SteeringPlugin_Output", output)
		&& pBlackboard->GetData("deltaT", deltaT)
		&& pBlackboard->GetData("IExamInterface", m_pInterface);
	if (!result)
		return Failure;
#pragma endregion 

	if (!pAgent || !pSteering || !pTargeter)
		return Failure;

	Elite::Vector2 target{};
	//According to scout step we increase or decrease the "Scouting area of our agent"
	//x
	switch (*scout)
	{
	case 0:
	case 1:
	case 4:
		target.x = worldCenter->x - 0.2f* worldDimensions->x;
		break;
	case 2:
	case 3:
		target.x = worldCenter->x + 0.2f* worldDimensions->x;
		break;
	case 5:
	case 6:
	case 9:
		target.x = worldCenter->x - 0.4f* worldDimensions->x;
		break;
	case 7:
	case 8:
		target.x = worldCenter->x + 0.4f* worldDimensions->x;
		break;
	default:
		target.x = 0;
		break;
	}

	//y
	switch (*scout)
	{
	case 0:
	case 3:
	case 4:
		target.y = worldCenter->y - 0.2f* worldDimensions->y;
		break;
	case 2:
	case 1:
		target.y = worldCenter->y + 0.2f* worldDimensions->y;
		break;
	case 5:
	case 8:
	case 9:
		target.y = worldCenter->y - 0.4f* worldDimensions->y;
		break;
	case 7:
	case 6:
		target.y = worldCenter->y + 0.4f* worldDimensions->y;
		break;
	default:
		target.y = 0;
		break;
	}

	auto distanceSquared = (pAgent->Position - target).SqrtMagnitude();
	if (distanceSquared < 50)
	{
		*scout += 1;
		if (*scout > 9)
			*scout = 0;
	}

	target = m_pInterface->NavMesh_GetClosestPathPoint(target);
	pTargeter->GetGoalRef().Position = target;
	pTargeter->GetGoalRef().PositionSet = true;
	*output = pSteering->CalculateSteering(*deltaT, pAgent);
	output->AutoOrientate = true;
	
	std::cout << "Scouting step is currently: " << *scout << std::endl;

	return Success;
}


inline BehaviourState PickUpItem(Blackboard* pBlackboard)
{
#pragma region DATA
	EntityInfo* entity = nullptr;
	IExamInterface* m_pInterface = nullptr;
	std::vector<ItemInfo*>* inventoryArr = nullptr;
	std::vector<EntityInfo*>* knownItems = nullptr;
	auto result = pBlackboard->GetData("Entity", entity)
		&& pBlackboard->GetData("KnownItems", knownItems)
		&& pBlackboard->GetData("Inventory", inventoryArr);
	if (!result)
		return Failure;

	ItemInfo item;
	result = pBlackboard->GetData("IExamInterface", m_pInterface);
	if (!result)
		return Failure;

#pragma endregion 
	std::cout << "I'm picking up an item" << std::endl;

	std::vector<ItemInfo*> inventoryRef = *inventoryArr;

	auto isSuccessful = m_pInterface->Item_Grab(*entity, item);

	if (isSuccessful)
	{
		for (size_t i = 0; i < knownItems->size(); ++i)
		{
			if (knownItems->operator[](i))
			{
				
				if (knownItems->operator[](i)->Location == entity->Location)
				{
					delete knownItems->operator[](i);
					knownItems->operator[](i) = nullptr;
				}

			}

		}

		int slotID = -1;
		int medCount = 0;
		int foodCount = 0;
		int gunCount = 0;
		for (unsigned int i = 0; i < inventoryRef.size(); ++i)
		{	
			if (inventoryArr->at(i) == nullptr || int(inventoryArr->at(i)->Type) < 0)  
			{
				slotID = i;
			}
			else
			{
				if (inventoryRef[i]->Type == eItemType::PISTOL)
					++gunCount;
				if (inventoryRef[i]->Type == eItemType::MEDKIT)
					++medCount;
				if (inventoryRef[i]->Type == eItemType::FOOD)
					++foodCount;
			}

		}

		if (slotID != -1)
		{
			
			if (item.Type != eItemType::GARBAGE &&
				!(gunCount > 0 && item.Type == eItemType::PISTOL)
				&& !(foodCount > 0 && item.Type == eItemType::FOOD)
				&& !(medCount > 1 && item.Type == eItemType::MEDKIT))
			{
				m_pInterface->Inventory_AddItem(slotID, item);
				auto itemptr = new ItemInfo();
				itemptr->Type = item.Type;
				itemptr->ItemHash = item.ItemHash;
				inventoryArr->at(slotID) = itemptr;

			}
			else
			{
				m_pInterface->Inventory_AddItem(slotID, item);
				m_pInterface->Inventory_RemoveItem(slotID);
			}
		}
	}


	return Success;
}

inline BehaviourState EatFood(Blackboard* pBlackboard)
{
#pragma region DATA
	std::vector<ItemInfo*>* inventoryArr;
	IExamInterface* m_pInterface = nullptr;
	auto result = pBlackboard->GetData("Inventory", inventoryArr)
		&& pBlackboard->GetData("IExamInterface", m_pInterface);
	if (!result)
		return Failure;

#pragma endregion 

	std::vector<ItemInfo*> inventoryRef = *inventoryArr;

	int count = 0;
	for (auto i : inventoryRef)
	{
		if (i != nullptr)
		{
			if (i->Type == eItemType::FOOD)
			{
				ItemInfo item;
				m_pInterface->Inventory_GetItem(count, item);
				m_pInterface->Inventory_UseItem(count);
				m_pInterface->Item_GetMetadata(item, "energy");

				m_pInterface->Inventory_RemoveItem(count);
				delete inventoryArr->at(count);
				inventoryArr->at(count) = nullptr;

				break;
			}
		}

		++count;
	}

	std::cout << "I am eating food!" << std::endl;
	return Success;
}

inline BehaviourState UseMedKit(Blackboard* pBlackboard)
{
#pragma region DATA
	std::vector<ItemInfo*>* inventoryArr;
	IExamInterface* m_pInterface = nullptr;
	auto result = pBlackboard->GetData("Inventory", inventoryArr)
		&& pBlackboard->GetData("IExamInterface", m_pInterface);
	if (!result)
		return Failure;

#pragma endregion 

	std::vector<ItemInfo*> inventoryRef = *inventoryArr;

	int count = 0;
	for (auto i : inventoryRef)
	{
		if (i != nullptr)
		{
			if (i->Type == eItemType::MEDKIT)
			{

				ItemInfo item;
				m_pInterface->Inventory_GetItem(count, item);
				m_pInterface->Inventory_UseItem(count);
				m_pInterface->Item_GetMetadata(item, "health");

				delete inventoryArr->at(count);
				inventoryArr->at(count) = nullptr;
				m_pInterface->Inventory_RemoveItem(count);

				break;

			}
		}
		++count;
	}

	std::cout << "I am using a medikit!" << std::endl;
	return Success;
}

inline BehaviourState SeekItem(Blackboard* pBlackboard)
{
	//get data
#pragma region DATA
	AgentInfo* pAgent = nullptr;
	CombinedSB::FixedGoalTargeter* pTargeter = nullptr;
	CombinedSB::SteeringPipeline* pSteering = nullptr;
	SteeringPlugin_Output* output = {};
	float* deltaT = 0;
	Elite::Vector2* target = nullptr;

	auto result = pBlackboard->GetData("Agent", pAgent)
		&& pBlackboard->GetData("Targeter", pTargeter)
		&& pBlackboard->GetData("SteeringPipeline", pSteering)
		&& pBlackboard->GetData("SteeringPlugin_Output", output)
		&& pBlackboard->GetData("deltaT", deltaT)
		&& pBlackboard->GetData("ItemPosition", target);

	if (!result)
		return Failure;

	if (!pAgent || !pTargeter || !pSteering)
		return Failure;

	
	IExamInterface* m_pInterface;
	result = pBlackboard->GetData("IExamInterface", m_pInterface);
	if (!result)
		return Failure;
#pragma endregion 

	*target = m_pInterface->NavMesh_GetClosestPathPoint(*target);
	pTargeter->GetGoalRef().Position = *target;
	pTargeter->GetGoalRef().PositionSet = true;
	*output = pSteering->CalculateSteering(*deltaT, pAgent);
	output->AutoOrientate = true;
	std::cout << "I am currently seeking an item" << std::endl;

	return Success;

}

inline BehaviourState SeekCheckPoint(Blackboard* pBlackboard)
{
	//get data
#pragma region DATA
	AgentInfo* pAgent = nullptr;
	CombinedSB::FixedGoalTargeter* pTargeter = nullptr;
	CombinedSB::SteeringPipeline* pSteering = nullptr;
	SteeringPlugin_Output* output = {};
	float* deltaT = 0;
	Elite::Vector2* target = nullptr;

	auto result = pBlackboard->GetData("Agent", pAgent)
		&& pBlackboard->GetData("Targeter", pTargeter)
		&& pBlackboard->GetData("SteeringPipeline", pSteering)
		&& pBlackboard->GetData("SteeringPlugin_Output", output)
		&& pBlackboard->GetData("deltaT", deltaT);

	if (!result)
		return Failure;

	if (!pAgent || !pTargeter || !pSteering)
		return Failure;


	IExamInterface* m_pInterface;
	result = pBlackboard->GetData("IExamInterface", m_pInterface);
	if (!result)
		return Failure;
#pragma endregion 

	*target = m_pInterface->NavMesh_GetClosestPathPoint(m_pInterface->World_GetCheckpointLocation());
	pTargeter->GetGoalRef().Position = *target;
	pTargeter->GetGoalRef().PositionSet = true;
	*output = pSteering->CalculateSteering(*deltaT, pAgent);
	output->AutoOrientate = true;
	
	std::cout << "I am currently going to the checkpoint" << std::endl;

	return Success;

}



inline BehaviourState DodgeEnemy(Blackboard* pBlackboard)
{
	//get data
#pragma region DATA
	std::vector<ItemInfo*>* inventoryArr = nullptr;
	AgentInfo* pAgent = nullptr;
	CombinedSB::FixedGoalTargeter* pTargeter = nullptr;
	CombinedSB::SteeringPipeline* pSteering = nullptr;
	SteeringPlugin_Output* output = {};
	float* deltaT = 0;
	EntityInfo* pEnemy = nullptr;

	auto result = pBlackboard->GetData("Agent", pAgent)
		&& pBlackboard->GetData("Inventory", inventoryArr)
		&& pBlackboard->GetData("Targeter", pTargeter)
		&& pBlackboard->GetData("SteeringPipeline", pSteering)
		&& pBlackboard->GetData("SteeringPlugin_Output", output)
		&& pBlackboard->GetData("deltaT", deltaT)
		&& pBlackboard->GetData("Targetenemy", pEnemy);

	if (!result)
		return Failure;

	if (!pAgent || !pTargeter || !pSteering)
		return Failure;

	
	IExamInterface* m_pInterface = nullptr;
	result = pBlackboard->GetData("IExamInterface", m_pInterface);
	if (!result)
		return Failure;
#pragma endregion 

	Elite::Vector2 target = pEnemy->Location;

	Elite::Vector2 a2g = pTargeter->GetGoalRef().Position - pAgent->Position;

	if (a2g.SqrtMagnitude() > 10)//if this close then just go
	{
		auto a2s = pEnemy->Location - pAgent->Position;
		auto moveDirection = a2g;
		
		moveDirection.Normalize();
		
		auto distanceToClosest = Elite::Dot(a2s, moveDirection);
		auto d = a2s.Magnitude() - (distanceToClosest);

		///DODGING
		//Here we check for collisions
		auto radius = 10.f;// giving it a larger base margin
		if (d < radius)
		{
			if (distanceToClosest > 0.f)
			{
				//Find closest point
				auto closestPoint = pAgent->Position + (distanceToClosest * moveDirection);

				//Find the point of avoidance
				auto offsetDir = closestPoint - pEnemy->Location;
				offsetDir.Normalize();
				auto target = m_pInterface->NavMesh_GetClosestPathPoint(pEnemy->Location + (radius * offsetDir));
				pTargeter->GetGoalRef().Position = target;
				pTargeter->GetGoalRef().PositionSet = true;
			}
		}

	}

	*output = pSteering->CalculateSteering(*deltaT, pAgent);
	output->AutoOrientate = false;

	int pistolID = -1;
	for (unsigned int i = 0; i < inventoryArr->size(); ++i)
	{
		if (inventoryArr->at(i))
		{
			if (inventoryArr->at(i)->Type == eItemType::PISTOL)
				pistolID = i;
		}

	}
	
	auto vecBetweenTargetAndAgent = target - pAgent->Position;
	vecBetweenTargetAndAgent.Normalize();
	auto zerovec = Elite::Vector2(1, 0);
	auto dot = Elite::Dot(vecBetweenTargetAndAgent, zerovec);
	auto orient = acos(dot);
	auto goalOrient = pAgent->Orientation - orient;
	output->AngularVelocity = goalOrient;
	
	//std::cout << "Shooting gun" << std::endl;

	ItemInfo item;
	if (pistolID != -1) 
	{
		m_pInterface->Inventory_GetItem(pistolID, item);
		m_pInterface->Inventory_UseItem(pistolID);
		m_pInterface->Item_GetMetadata(item, "ammo");
		
			if (inventoryArr->at(pistolID))
			{
				delete inventoryArr->at(pistolID);
				inventoryArr->at(pistolID) = nullptr;
				m_pInterface->Inventory_RemoveItem(pistolID);
			}

		
	}


	return Success;

}


inline BehaviourState Rest(Blackboard* pBlackboard)
{
#pragma region DATA
	AgentInfo* pAgent = nullptr;
	CombinedSB::FixedGoalTargeter* pTargeter = nullptr;
	CombinedSB::SteeringPipeline* pSteering = nullptr;
	SteeringPlugin_Output* output = {};
	float* deltaT = 0;
	Elite::Vector2* target = nullptr;

	auto result = pBlackboard->GetData("Agent", pAgent)
		&& pBlackboard->GetData("Targeter", pTargeter)
		&& pBlackboard->GetData("SteeringPipeline", pSteering)
		&& pBlackboard->GetData("SteeringPlugin_Output", output)
		&& pBlackboard->GetData("deltaT", deltaT)
		&& pBlackboard->GetData("ItemPosition", target);

	if (!result)
		return Failure;

	if (!pAgent || !pTargeter || !pSteering)
		return Failure;

	
	IExamInterface* m_pInterface;
	result = pBlackboard->GetData("IExamInterface", m_pInterface);
	if (!result)
		return Failure;
#pragma endregion 

	*target = pAgent->Position;
	pTargeter->GetGoalRef().Position = *target;
	pTargeter->GetGoalRef().PositionSet = true;
	*output = pSteering->CalculateSteering(*deltaT, pAgent);
	output->AutoOrientate = true;

	std::cout << "I am currently resting!" << std::endl;

	return Success;

}



inline BehaviourState Aim(Blackboard* pBlackboard)
{
	//get data
#pragma region DATA
	std::vector<ItemInfo*>* inventoryArr = nullptr;
	AgentInfo* pAgent = nullptr;
	CombinedSB::FixedGoalTargeter* pTargeter = nullptr;
	CombinedSB::SteeringPipeline* pSteering = nullptr;
	SteeringPlugin_Output* output = {};
	float* deltaT = 0;
	EntityInfo* pEnemy = nullptr;

	auto result = pBlackboard->GetData("Agent", pAgent)
		&& pBlackboard->GetData("Inventory", inventoryArr)
		&& pBlackboard->GetData("Targeter", pTargeter)
		&& pBlackboard->GetData("SteeringPipeline", pSteering)
		&& pBlackboard->GetData("SteeringPlugin_Output", output)
		&& pBlackboard->GetData("deltaT", deltaT)
		&& pBlackboard->GetData("Targetenemy", pEnemy);

	if (!result)
		return Failure;

	if (!pAgent || !pTargeter || !pSteering)
		return Failure;


	IExamInterface* m_pInterface = nullptr;
	result = pBlackboard->GetData("IExamInterface", m_pInterface);
	if (!result)
		return Failure;
#pragma endregion 

	

	auto vecBetweenAgentAndEnemy = (pEnemy->Location - m_pInterface->Agent_GetInfo().Position);
	Elite::Normalize(vecBetweenAgentAndEnemy);

	Elite::Vector2 forward{};
	AgentInfo agentInfo = m_pInterface->Agent_GetInfo();

	forward.x = agentInfo.LinearVelocity.x * cos(agentInfo.Orientation) - agentInfo.LinearVelocity.y * sin(agentInfo.Orientation);
	forward.y = agentInfo.LinearVelocity.x * sin(agentInfo.Orientation) + agentInfo.LinearVelocity.y * cos(agentInfo.Orientation);

	float angle = atan2(Elite::Cross(forward, vecBetweenAgentAndEnemy), Elite::Dot(forward, vecBetweenAgentAndEnemy));

	//float angle = (atan2(velocity.x, velocity.z) * 180 / XM_PI) + 180.f;

	

	if (angle > 2.0f)
	{
		return Running;
	}

	output->AngularVelocity = angle;
	
	return Success;


}

inline BehaviourState Shoot(Blackboard* pBlackboard)
{
	//get data
#pragma region DATA
	std::vector<ItemInfo*>* inventoryArr = nullptr;
	AgentInfo* pAgent = nullptr;
	CombinedSB::FixedGoalTargeter* pTargeter = nullptr;
	CombinedSB::SteeringPipeline* pSteering = nullptr;
	SteeringPlugin_Output* output = {};
	float* deltaT = 0;
	EntityInfo* pEnemy = nullptr;

	auto result = pBlackboard->GetData("Agent", pAgent)
		&& pBlackboard->GetData("Inventory", inventoryArr)
		&& pBlackboard->GetData("Targeter", pTargeter)
		&& pBlackboard->GetData("SteeringPipeline", pSteering)
		&& pBlackboard->GetData("SteeringPlugin_Output", output)
		&& pBlackboard->GetData("deltaT", deltaT)
		&& pBlackboard->GetData("Targetenemy", pEnemy);

	if (!result)
		return Failure;

	if (!pAgent || !pTargeter || !pSteering)
		return Failure;


	IExamInterface* m_pInterface = nullptr;
	result = pBlackboard->GetData("IExamInterface", m_pInterface);
	if (!result)
		return Failure;
#pragma endregion 

	auto vecBetweenAgentAndEnemy = (pEnemy->Location - m_pInterface->Agent_GetInfo().Position);
	Elite::Normalize(vecBetweenAgentAndEnemy);

	Elite::Vector2 forward{};
	AgentInfo agentInfo = m_pInterface->Agent_GetInfo();

	forward.x = agentInfo.LinearVelocity.x * cos(agentInfo.Orientation) - agentInfo.LinearVelocity.y * sin(agentInfo.Orientation);
	forward.y = agentInfo.LinearVelocity.x * sin(agentInfo.Orientation) + agentInfo.LinearVelocity.y * cos(agentInfo.Orientation);

	float angle = atan2(Elite::Cross(forward, vecBetweenAgentAndEnemy), Elite::Dot(forward, vecBetweenAgentAndEnemy));

	//float angle = (atan2(velocity.x, velocity.z) * 180 / XM_PI) + 180.f;
	if (angle > 2.0f)
	{
		return Running;
	}

	output->AngularVelocity = angle;


	int pistolID = -1;
	for (unsigned int i = 0; i < inventoryArr->size(); ++i)
	{
		if (inventoryArr->at(i))
		{
			if (inventoryArr->at(i)->Type == eItemType::PISTOL)
				pistolID = i;
		}

	}

	ItemInfo item;
	if (pistolID != -1)// && (abs(output->AngularVelocity - orient)) < 0.3)
	{
		m_pInterface->Inventory_GetItem(pistolID, item);
		m_pInterface->Inventory_UseItem(pistolID);
		m_pInterface->Item_GetMetadata(item, "ammo");
		
			if (inventoryArr->at(pistolID))
			{
				delete inventoryArr->at(pistolID);
				inventoryArr->at(pistolID) = nullptr;
				m_pInterface->Inventory_RemoveItem(pistolID);
			}

		
	}
	std::cout << "Shooting gun" << std::endl;

	return Success;
}


inline bool HasHousesInFOV(Blackboard* pBlackboard)
{
	//get data
#pragma region DATA
	std::vector<HouseInfo> houses;
	auto result = pBlackboard->GetData("Houses", houses);
	if (!result)
		return false;
	IExamInterface* m_pInterface;

	result = pBlackboard->GetData("IExamInterface", m_pInterface);
	if (!result)
		return Failure;
#pragma endregion
	if (houses.size() > 0)
	{
		for (auto h : houses)
		{
			bool exists = false;
			int index = 0;
			int count = 0;
			for (auto r : Plugin::m_houseArr)
			{
				if (r->m_Center.x == h.Center.x && r->m_Center.y == h.Center.y)
				{
					exists = true;
					index = count;
				}
				++count;
			}
			if (!exists)
			{
				House* house = new House(m_pInterface->World_GetInfo().Dimensions);
				house->m_Center = h.Center;
				house->m_Size = h.Size;
				Plugin::m_houseArr.push_back(house);
				index = Plugin::m_houseArr.size() - 1;
			}

		}
		return true;
	}
	return false;
}






//// Returns true if any enemy is within range of the agent's longest range pistol
//inline bool HasEnemyInRange(Blackboard* pBlackboard)
//{
//	AgentInfo* pAgentInfo = nullptr;
//	std::vector<EnemyInfo>* knownEnemies = nullptr;
//	float longestPistolRange;
//	bool dataAvailable =
//		pBlackboard->GetData("AgentInfo", pAgentInfo) &&
//		pBlackboard->GetData("KnownEnemies", knownEnemies) &&
//		pBlackboard->GetData("LongestPistolRange", longestPistolRange);
//
//	if (!dataAvailable || !pAgentInfo || longestPistolRange == 0.0f)
//		return false;
//
//	float dist;
//	EnemyInfo nearestEnemy;
//	if (NearestEnemyInFOV(knownEnemies, pAgentInfo, nearestEnemy, dist))
//	{
//		if (dist < longestPistolRange)
//		{
//			return true;
//		}
//	}
//
//	return false;
//}
////
