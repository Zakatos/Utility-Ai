#include "stdafx.h"
#include "Plugin.h"
#include "IExamInterface.h"
#include "SurvivorAi.h"

std::vector<House*> Plugin::m_houseArr{};

Plugin::~Plugin()
{
	
	for (size_t i = 0; i < m_houseArr.size(); ++i)
	{
		delete m_houseArr[i];
		m_houseArr[i] = nullptr;
	}

	for (size_t i = 0; i < m_InventoryArr.size(); ++i)
	{
		delete m_InventoryArr[i];
		m_InventoryArr[i] = nullptr;
	}

	for (size_t i = 0; i < m_KnowItemsArr.size(); ++i)
	{
		delete m_KnowItemsArr[i];
		m_KnowItemsArr[i] = nullptr;
	}

	delete m_pEnemy;
	m_pEnemy = nullptr;

	delete m_pEntity;
	m_pEntity = nullptr;

	delete m_pOutput;
	m_pOutput = nullptr;

	delete m_pdTime;
	m_pdTime = nullptr;

	delete m_pAgent;
	m_pAgent = nullptr;

	delete m_pUtilityBuilder;
	m_pUtilityBuilder = nullptr;

	delete m_pSeekBehaviour;
	m_pSeekBehaviour = nullptr;

	delete m_pWanderBehaviour;
	m_pWanderBehaviour = nullptr;

	delete m_pSteeringPipeline;
	m_pSteeringPipeline = nullptr;

	delete m_pTargeter;
	m_pTargeter = nullptr;

	delete m_pConstraint;
	m_pConstraint = nullptr;

	delete m_pActuator;
	m_pActuator = nullptr;

	delete m_pworldCenter;
	m_pworldCenter = nullptr;

	delete m_pworldDimensions;
	m_pworldDimensions = nullptr;

	delete m_pscout;
	m_pscout = nullptr;

	delete m_pStepComplete;
	m_pStepComplete = nullptr;

	delete m_pkillCountDown;
	m_pkillCountDown = nullptr;

}

//Called only once, during initialization
void Plugin::Initialize(IBaseInterface* pInterface, PluginInfo& info)
{
	//memleak stuff:
	//_CrtSetBreakAlloc(60202);
	m_pInterface = static_cast<IExamInterface*>(pInterface);

	//Bit information about the plugin
	//Please fill this in!!
	info.BotName = "Survivor Scout";
	info.Student_FirstName = "Andreas";
	info.Student_LastName = "Meletiadis-Megas";
	info.Student_Class = "2DAE1";


}

//Called only once
void Plugin::DllInit()
{
	//Can be used to figure out the source of a Memory Leak
	//Possible undefined behavior, you'll have to trace the source manually 
	//if you can't get the origin through _CrtSetBreakAlloc(0) [See CallStack]
	//_CrtSetBreakAlloc(0);

	//Called when the plugin is loaded
}

//Called only once
void Plugin::DllShutdown()
{
	//Called wheb the plugin gets unloaded
}

//Called only once, during initialization
void Plugin::InitGameDebugParams(GameDebugParams& params)
{
	params.AutoFollowCam = true; //Automatically follow the AI? (Default = true)
	params.RenderUI = true; //Render the IMGUI Panel? (Default = true)
	params.SpawnEnemies = true; //Do you want to spawn enemies? (Default = true)
	params.EnemyCount = 20; //How many enemies? (Default = 20)
	params.GodMode = false; //GodMode > You can't die, can be usefull to inspect certain behaviours (Default = false)
							//params.LevelFile = "LevelTwo.gppl";
	params.AutoGrabClosestItem = true; //A call to Item_Grab(...) returns the closest item that can be grabbed. (EntityInfo argument is ignored)
	params.OverrideDifficulty = false; //Override Difficulty?
	params.Difficulty = 1.f; //Difficulty Override: 0 > 1 (Overshoot is possible, >1)
}

//Only Active in DEBUG Mode
//(=Use only for Debug Purposes)
void Plugin::ProcessEvents(const SDL_Event& e)
{
	//Demo Event Code
	//In the end your AI should be able to walk around without external input
	switch (e.type)
	{
	case SDL_MOUSEBUTTONUP:
	{
		if (e.button.button == SDL_BUTTON_LEFT)
		{
			int x, y;
			SDL_GetMouseState(&x, &y);
			const Elite::Vector2 pos = Elite::Vector2(static_cast<float>(x), static_cast<float>(y));
			m_Target = m_pInterface->Debug_ConvertScreenToWorld(pos);
		}
		break;
	}
	case SDL_KEYDOWN:
	{
		if (e.key.keysym.sym == SDLK_SPACE)
		{
			m_CanRun = true;
		}
		else if (e.key.keysym.sym == SDLK_LEFT)
		{
			m_AngSpeed -= Elite::ToRadians(10);
		}
		else if (e.key.keysym.sym == SDLK_RIGHT)
		{
			m_AngSpeed += Elite::ToRadians(10);
		}
		else if (e.key.keysym.sym == SDLK_g)
		{
			m_GrabItem = true;
		}
		else if (e.key.keysym.sym == SDLK_u)
		{
			m_UseItem = true;
		}
		else if (e.key.keysym.sym == SDLK_r)
		{
			m_RemoveItem = true;
		}
		else if (e.key.keysym.sym == SDLK_d)
		{
			m_DropItem = true;
		}
		break;
	}
	case SDL_KEYUP:
	{
		if (e.key.keysym.sym == SDLK_SPACE)
		{
			m_CanRun = false;
		}
		break;
	}
	}
}

//Update
//This function calculates the new SteeringOutput, called once per frame
SteeringPlugin_Output Plugin::UpdateSteering(float dt)
{
	SetUpAgents();
	auto killcountdown = m_pInterface->World_GetStats().KillCountdown;
	
	auto agentInfo = m_pInterface->Agent_GetInfo();
	*m_pAgent = agentInfo;
	
	auto entities = GetEntitiesInFOV();

	std::vector<Elite::Vector2> enemies;
	for (auto i : entities)
	{
		if (i.Type == eEntityType::ENEMY)
			enemies.push_back(i.Location);
	}
	m_pConstraint->GetAgentsRef() = enemies;

	auto houses = GetHousesInFOV();

	auto pBlackboard = m_pUtilityBuilder->GetBlackboard();
	if (pBlackboard)
	{
		pBlackboard->ChangeData("EntityInfo", entities);
		pBlackboard->ChangeData("Houses", houses);
	}

	if (m_pUtilityBuilder)
		m_pUtilityBuilder->UpdateUtility(m_pAgent, entities, m_houseArr, m_InventoryArr, killcountdown, dt);

	float* dTime = nullptr;
	pBlackboard->GetData("deltaT", dTime);

	*dTime = dt;
	m_pOutput->RunMode = true;

	return *m_pOutput;
}

//This function should only be used for rendering debug elements
void Plugin::Render(float dt) const
{
	//This Render function should only contain calls to Interface->Draw_... functions
	m_pInterface->Draw_SolidCircle(m_Target, .7f, { 0,0 }, { 1, 0, 0 });
}

vector<HouseInfo> Plugin::GetHousesInFOV() const
{
	vector<HouseInfo> vHousesInFOV = {};

	HouseInfo hi = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetHouseByIndex(i, hi))
		{
			vHousesInFOV.push_back(hi);
			continue;
		}

		break;
	}

	return vHousesInFOV;
}

vector<EntityInfo> Plugin::GetEntitiesInFOV() const
{
	vector<EntityInfo> vEntitiesInFOV = {};

	EntityInfo ei = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetEntityByIndex(i, ei))
		{
			vEntitiesInFOV.push_back(ei);
			continue;
		}

		break;
	}

	return vEntitiesInFOV;
}


void Plugin::SetUpAgents()
{

	if (!m_IsAgentSetUp)
	{
		for (size_t i = 0; i < m_pInterface->Inventory_GetCapacity(); ++i)
		{
			m_InventoryArr.push_back(nullptr);
		}
		for (size_t i = 0; i < 20; ++i)
		{
			m_KnowItemsArr.push_back(nullptr);
		}

		std::vector<EntityInfo> entities;
		std::vector<HouseInfo> houses;
		m_pEnemy = new EntityInfo();
		m_pOutput = new SteeringPlugin_Output();
		m_pEntity = new EntityInfo();
		// Create agent
		m_pAgent = new AgentInfo();
		
		m_pTargeter = new CombinedSB::FixedGoalTargeter();
		m_pConstraint = new CombinedSB::AvoidAgentConstraint();
		m_pSeekBehaviour = new SteeringBehaviours::Seek();
		m_pSeekBehaviour->SetTarget(&m_Target);
		m_pWanderBehaviour = new SteeringBehaviours::Wander();
		m_pActuator = new CombinedSB::BasicActuator(m_pSeekBehaviour, m_pWanderBehaviour);

		m_pSteeringPipeline = new CombinedSB::SteeringPipeline();
		m_pSteeringPipeline->SetTargeters({ m_pTargeter });
		m_pSteeringPipeline->SetConstraints({ m_pConstraint });
		m_pSteeringPipeline->SetActuator(m_pActuator);
		m_pSteeringPipeline->SetFallBack(m_pWanderBehaviour);

		//*** Create blackboard ***
		auto pBlackboard = new Blackboard();
		pBlackboard->AddData("Agent", m_pAgent);
		pBlackboard->AddData("Inventory", &m_InventoryArr);//cleanup
		pBlackboard->AddData("KnownItems", &m_KnowItemsArr);
		pBlackboard->AddData("TargetPosition", &m_Target);
		pBlackboard->AddData("ItemPosition", &m_Item);
		pBlackboard->AddData("TargetSet", false);
		pBlackboard->AddData("EntityInfo", entities);
		pBlackboard->AddData("Houses", houses);
		pBlackboard->AddData("Entity", m_pEntity);
		pBlackboard->AddData("EntitySet", false);
		pBlackboard->AddData("IExamInterface", m_pInterface);
		pBlackboard->AddData("Targetenemy", m_pEnemy);

		pBlackboard->AddData("Targeter",
			m_pTargeter);
		pBlackboard->AddData("SteeringPipeline",
			m_pSteeringPipeline);
		pBlackboard->AddData("SeekBehaviour",
			static_cast<SteeringBehaviours::ISteeringBehaviour*>(m_pSeekBehaviour));
		pBlackboard->AddData("WanderBehaviour",
			static_cast<SteeringBehaviours::ISteeringBehaviour*>(m_pWanderBehaviour));
		pBlackboard->AddData("CloseToRadius", 1.0f);
		pBlackboard->AddData("SteeringPlugin_Output", m_pOutput);
		pBlackboard->AddData("deltaT", m_pdTime);
		pBlackboard->AddData("KillCountdown", m_pkillCountDown);
		//scout
		m_pworldDimensions = new Elite::Vector2();
		*m_pworldDimensions = m_pInterface->World_GetInfo().Dimensions;
		m_pworldCenter = new Elite::Vector2();
		*m_pworldCenter = m_pInterface->World_GetInfo().Center;
		pBlackboard->AddData("WorldDimensions", m_pworldDimensions);
		pBlackboard->AddData("worldCenter", m_pworldCenter);
		m_pscout = new int();
		*m_pscout = 0;
		pBlackboard->AddData("scoutCount", m_pscout);
		m_pStepComplete = new bool();
		*m_pStepComplete = false;
		pBlackboard->AddData("StepComplete", m_pStepComplete);

		m_pUtilityBuilder = new SurvivorAi(pBlackboard);
		m_pUtilityBuilder->BuildUtility();

		m_IsAgentSetUp = true;
	}
}