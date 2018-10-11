#pragma once
#include "Utility_Selector.h"
#include "House.h"
#include "Behaviors.h"
#include "Utility_Action.h"

//Here we actually add the weight and the scores for each action
//It is the actual place where the selector gets execute the top action
class SurvivorAi
{
	
public:
	SurvivorAi(Blackboard* pBlackBoard) :
		m_pBlackboard(pBlackBoard) {}
	~SurvivorAi()
	{
		delete m_pSelector;
		m_pSelector = nullptr;

		delete m_pBlackboard;
		m_pBlackboard = nullptr;
	}

	void BuildUtility()
	{
		//considerations:
		

		m_pFoodConsideration = new Utility_Consideration();
		m_pFoodConsideration->SetCurve(new LinearCurve());

		m_pItemInFOV = new Utility_Consideration();
		m_pItemInFOV->SetCurve(new LinearCurve());
		m_pItemInFOV->SetWeight(0.8f);

		m_pItemInReachConsideration = new Utility_Consideration();
		m_pItemInReachConsideration->SetWeight(2.0f);
		m_pItemInReachConsideration->SetCurve(new LinearCurve());

		m_pHealthConsideration = new Utility_Consideration();
		m_pHealthConsideration->SetCurve(new LinearCurve());

		m_pAvailableHouse = new Utility_Consideration();
		auto lin = new LinearCurve();
		lin->b = -0.4f;
		m_pAvailableHouse->SetCurve(lin);
		m_pAvailableHouse->SetWeight(1.0f);

		m_pHasFood = new Utility_Consideration();
		auto Step = new StepCurve();
		Step->a = 0.8f;
		Step->b = 1.4f;
		m_pHasFood->SetCurve(Step);

		m_pHasMedkit = new Utility_Consideration();
		auto Step3 = new StepCurve();
		Step3->a = 0.8f;
		Step3->b = 1.4f;
		m_pHasMedkit->SetWeight(0.5f);
		m_pHasMedkit->SetCurve(Step3);

		m_pHasGun = new Utility_Consideration();
		auto Step2 = new StepCurve();
		m_pHasGun->SetCurve(Step2);

		m_pEnemyInRange = new Utility_Consideration();
		m_pEnemyInRange->SetWeight(4.0f);
		m_pEnemyInRange->SetCurve(new LinearCurve());

		m_pBiasUtillity = new Utility_Consideration();
		m_pBiasUtillity->SetCurve(new LinearCurve());
		m_pBiasUtillity->SetScore(0.5f);

		m_pShootRange = new Utility_Consideration();
		auto Step4 = new StepCurve();
		m_pShootRange->SetCurve(Step4);

		m_pCheckPointClose = new Utility_Consideration();
		auto Step5 = new StepCurve();
		m_pCheckPointClose->SetCurve(Step5);

		//actions:
		m_pPickUpItemAction = new Utility_Action(PickUpItem, "collect item", 3);
		m_pPickUpItemAction->AddConsideration(m_pItemInReachConsideration);

		m_pSeekItemAction = new Utility_Action(SeekItem, "seek item", 2);		
		m_pSeekItemAction->AddConsideration(m_pItemInFOV);

		//default behaviour
		m_pWanderAction = new Utility_Action(Scout, "Scout", 0);
		m_pWanderAction->AddConsideration(m_pBiasUtillity);

		m_pGoIntoHouse = new Utility_Action(Seek, "seek shelter", 1);
		m_pGoIntoHouse->AddConsideration(m_pAvailableHouse);

		m_pEatFood = new Utility_Action(EatFood, "eat Food", 4);
		m_pEatFood->AddConsideration(m_pFoodConsideration);
		m_pEatFood->AddConsideration(m_pHasFood);

		m_pUseMedKit = new Utility_Action(UseMedKit, "use Medkit", 5);
		m_pUseMedKit->AddConsideration(m_pHealthConsideration);
		m_pUseMedKit->AddConsideration(m_pHasMedkit);

		m_pDodgeEnemy = new Utility_Action(DodgeEnemy, "Dodge", 2);
		m_pDodgeEnemy->AddConsideration(m_pEnemyInRange);
		m_pDodgeEnemy->AddConsideration(m_pHasGun);

		/*m_pAimEnemy = new Utility_Action(Aim, "Aim", 2);
		m_pAimEnemy->AddConsideration(m_pEnemyInRange);
		m_pAimEnemy->AddConsideration(m_pHasGun);*/

		/*m_pShootEnemy = new Utility_Action(Shoot, "Shoot", 1);
		m_pShootEnemy->AddConsideration(m_pShootRange);
		m_pShootEnemy->AddConsideration(m_pHasGun);*/

		m_pSeekCheckPoint = new Utility_Action(SeekCheckPoint, "Check point", 5);
		m_pSeekCheckPoint->AddConsideration(m_pCheckPointClose);

		//filling selector:
		m_pSelector = new UtilitySelector();
		m_pSelector->AddAction(m_pWanderAction);
		m_pSelector->AddAction(m_pPickUpItemAction);
		m_pSelector->AddAction(m_pSeekItemAction);
		m_pSelector->AddAction(m_pGoIntoHouse);
		m_pSelector->AddAction(m_pEatFood);
		m_pSelector->AddAction(m_pUseMedKit);
		m_pSelector->AddAction(m_pDodgeEnemy);
		//m_pSelector->AddAction(m_pAimEnemy);
		//m_pSelector->AddAction(m_pShootEnemy);

	}
	void UpdateUtility(AgentInfo* agent,
		const std::vector<EntityInfo>& entitities,
		std::vector<House*>& houseArr,
		std::vector<ItemInfo*>& inventory
		,float killcountdowntime,float dTime)
	{
		//update consideration values here
		m_pHealthConsideration->SetScore(1 - (agent->Health / m_MaxHealth));
		m_pFoodConsideration->SetScore(1 - (agent->Energy / m_MaxFood));

		bool itemInFOV = false;
		std::vector<EntityInfo*>* knownItems;
		m_pBlackboard->GetData("KnownItems", knownItems);

		int closestEnemyID = -1;
		float closestDistance = 10000;
		int count = 0;
		for (auto i : entitities)
		{
			if (i.Type == eEntityType::ITEM)
			{

				bool known = false;
				int freeID = -1;
				int count = 0;
				for (auto knownItem : *knownItems)
				{
					if (knownItem)
					{
						if (knownItem->Location == i.Location)
						{
							known = true;
						}
					}
					else
					{
						freeID = count;
					}
					++count;
				}
				if (!known)
					if (freeID != -1)
					{
						EntityInfo* x = new EntityInfo();
						*x = i;
						knownItems->at(freeID) = x;
					}
					else
						std::cout << "knowitems container too small to hold all items";
			}
			if (i.Type == eEntityType::ENEMY)
			{
				auto distanceSquared = (i.Location - agent->Position).SqrtMagnitude();
				if (closestDistance > distanceSquared)
				{
					closestDistance = distanceSquared;
					closestEnemyID = count;
				}
			}
			++count;

		}

		
		//enemy
		if (closestEnemyID != -1)
		{
			EntityInfo* pEnemy = nullptr;
			m_pBlackboard->GetData("Targetenemy", pEnemy);
			
			pEnemy->Location = entitities[closestEnemyID].Location;
			if (closestDistance < 25) //only when the enemy gets really close (***for later**or the killcountdown is getting low )
				
				m_pEnemyInRange->SetScore(1);
				
		}
		else
			
			m_pEnemyInRange->SetScore(-1);
			
		//items

		std::vector<EntityInfo*>* validKnownItems = new std::vector<EntityInfo*>();
		for (size_t i = 0; i < knownItems->size(); ++i)
		{
			if (knownItems->at(i))
			{
				validKnownItems->push_back(knownItems->at(i));
			}

		}
		validKnownItems->size() > 0 ? m_pItemInFOV->SetScore(1) : m_pItemInFOV->SetScore(-1);

		bool thing = false;
		if (validKnownItems->size() > 0)
		{
			thing = true;
			auto closeToRadius = 0.0f;
			m_pBlackboard->GetData("CloseToRadius", closeToRadius);
			int closestItem = -1;
			float shortestDistanceSqr = 1000;
			for (unsigned int i = 0; i < validKnownItems->size(); ++i)
			{
				auto distanceSquared = (validKnownItems->at(i)->Location - agent->Position).SqrtMagnitude();
				if (shortestDistanceSqr > distanceSquared)
				{
					shortestDistanceSqr = distanceSquared;
					closestItem = i;
				}
			}
			if (closestItem >= 0)
			{
				Elite::Vector2* targetPos = nullptr;
				m_pBlackboard->GetData("ItemPosition", targetPos);
				*targetPos = validKnownItems->at(closestItem)->Location;
			}


			if (shortestDistanceSqr < closeToRadius * closeToRadius)
			{
				EntityInfo* entity;
				m_pBlackboard->GetData("Entity", entity);
				*entity = *validKnownItems->at(closestItem);
				m_pItemInReachConsideration->SetScore(1);
			}
			else
				m_pItemInReachConsideration->SetScore(-1);
		}
		else
			m_pItemInReachConsideration->SetScore(-1);

		delete validKnownItems;
		validKnownItems = nullptr;

		//check if gun
		std::vector<ItemInfo*>* inventoryArr = nullptr;
		m_pBlackboard->GetData("Inventory", inventoryArr);
		bool hasGun = false;
		for (size_t x = 0; x < inventoryArr->size(); ++x)
		{
			if (inventoryArr->at(x))
			{
				if (inventoryArr->at(x)->Type == eItemType::PISTOL)
					hasGun = true;
			}

		}
		hasGun ? m_pHasGun->SetScore(1) : m_pHasGun->SetScore(-1);


		//houses
		HasHousesInFOV(m_pBlackboard);
		int closestHouse = -1;
		float shortestDistanceSqr = 100000;
		if (houseArr.size() > 0)
		{
			for (unsigned int i = 0; i < houseArr.size(); ++i)
			{
				int* scoutCount;
				m_pBlackboard->GetData("scoutCount", scoutCount);
				*scoutCount > 8 ? houseArr[i]->Update(dTime, agent, true) : houseArr[i]->Update(dTime, agent, false);

				auto distanceSquared = (houseArr[i]->m_Center - agent->Position).SqrtMagnitude();
				if (shortestDistanceSqr > distanceSquared && !houseArr[i]->m_ReachedCenter)
				{
					shortestDistanceSqr = distanceSquared;
					closestHouse = i;
				}
			}

			if (closestHouse != -1)
			{
				Elite::Vector2* targetPos = nullptr;
				m_pBlackboard->GetData("TargetPosition", targetPos);
				*targetPos = houseArr[closestHouse]->m_Center;
				m_pAvailableHouse->SetScore(1);
			}
			else
				m_pAvailableHouse->SetScore(-1);

		}
		else
			m_pAvailableHouse->SetScore(-1);

		//items usage
		bool hasFood = false;
		bool hasMedkit = false;

		for (auto item : inventory)
		{
			if (item)
			{
				if (item->Type == eItemType::FOOD)
					hasFood = true;
				if (item->Type == eItemType::MEDKIT)
					hasMedkit = true;
			}
		}

		hasMedkit ? m_pHasMedkit->SetScore(1.0f) : m_pHasMedkit->SetScore(-1.0f);
		hasFood ? m_pHasFood->SetScore(1.0f) : m_pHasFood->SetScore(-1.0f);

		//CheckPoint
		
		IExamInterface* m_pInterface = nullptr;
		m_pBlackboard->GetData("IExamInterface", m_pInterface);

		if(Elite::Distance(m_pInterface->World_GetCheckpointLocation(),agent->Position) < 30.f)
		{
			m_pCheckPointClose->SetScore(1.0f);
		}
		else
		{
			m_pCheckPointClose->SetScore(-1.0f);
		}
		//Here the selector gets the top action and executes it
		m_pSelector->GetTopAction()->Execute(m_pBlackboard);

	}


	Blackboard* GetBlackboard() const
	{
		return m_pBlackboard;
	}
private:
	Blackboard * m_pBlackboard = nullptr;
	UtilitySelector* m_pSelector = nullptr;

	//setup AI (pointers managed by selector)
	//considerations
	Utility_Consideration* m_pHealthConsideration = nullptr;
	Utility_Consideration* m_pFoodConsideration = nullptr;
	Utility_Consideration* m_pItemInFOV = nullptr;
	Utility_Consideration* m_pItemInReachConsideration = nullptr;
	Utility_Consideration* m_pAvailableHouse = nullptr;
	Utility_Consideration* m_pBiasUtillity = nullptr;
	Utility_Consideration* m_pHasFood = nullptr;
	Utility_Consideration* m_pHasMedkit = nullptr;
	Utility_Consideration* m_pHasGun = nullptr;
	Utility_Consideration* m_pEnemyInRange = nullptr;
	Utility_Consideration* m_pShootRange = nullptr;
	Utility_Consideration* m_pCheckPointClose = nullptr;
	//actions
	Utility_Action* m_pGoIntoHouse = nullptr;
	Utility_Action* m_pPickUpItemAction = nullptr;
	Utility_Action* m_pSeekItemAction = nullptr;
	Utility_Action* m_pSeekCheckPoint = nullptr;
	Utility_Action* m_pSeekEnemy = nullptr;
	Utility_Action* m_pShootEnemy = nullptr;
	Utility_Action* m_pAimEnemy = nullptr;	
	Utility_Action* m_pDodgeEnemy = nullptr;
	Utility_Action* m_pWanderAction = nullptr;
	Utility_Action* m_pEatFood = nullptr;
	Utility_Action* m_pUseMedKit = nullptr;

	//consts
	const float m_MaxHealth = 10.0f;
	const float m_MaxFood = 20.0f;
	const float m_MaxStamina = 10.0f;
};
