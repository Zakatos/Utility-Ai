#pragma once
#include "IExamPlugin.h"
#include "SteeringBehaviours.h"
#include "House.h"
#include "CombinedSB_PipelineImp.h"



class IBaseInterface;
class IExamInterface;
class SurvivorAi;
class Blackboard;

class Plugin :public IExamPlugin
{
public:
	Plugin() {};
	~Plugin();

	void Initialize(IBaseInterface* pInterface, PluginInfo& info) override;
	void DllInit() override;
	void DllShutdown() override;

	void InitGameDebugParams(GameDebugParams& params) override;
	void ProcessEvents(const SDL_Event& e) override;

	SteeringPlugin_Output UpdateSteering(float dt) override;
	void Render(float dt) const override;
	static std::vector<House*> m_houseArr;


private:

	float m_SlowRadius = 10.f;
	float m_TargetRadius = 2.f;
	bool m_GrabAction = false;
	bool m_UseItemAction = false;
	bool m_RemoveItemAction = false;
	bool m_RunAction = false;
	bool m_IsAgentSetUp = false;
	int m_SelectedInventorySlot = 0;

	//decisionmaking things
	AgentInfo* m_pAgent = nullptr;
	float* m_pdTime = new float(1);
	Elite::Vector2 m_Target;
	Elite::Vector2 m_Item;
	SteeringPlugin_Output* m_pOutput = nullptr;
	EntityInfo* m_pEntity = nullptr;
	EntityInfo* m_pEnemy = nullptr;
	Blackboard* m_pBlackBoard = nullptr;
	float* m_pkillCountDown = new float(1);
	//steerinpipeline
	SteeringBehaviours::Seek* m_pSeekBehaviour = nullptr;
	SteeringBehaviours::Wander* m_pWanderBehaviour = nullptr;
	CombinedSB::SteeringPipeline* m_pSteeringPipeline = nullptr;
	CombinedSB::FixedGoalTargeter* m_pTargeter = nullptr;
	CombinedSB::AvoidAgentConstraint* m_pConstraint = nullptr;
	CombinedSB::BasicActuator* m_pActuator = nullptr;

	SurvivorAi* m_pUtilityBuilder = nullptr;

	std::vector<ItemInfo*> m_InventoryArr;
	std::vector<EntityInfo*> m_KnowItemsArr;
	Elite::Vector2* m_pworldDimensions = nullptr;
	Elite::Vector2* m_pworldCenter = nullptr;
	int* m_pscout = nullptr;
	bool* m_pStepComplete = nullptr;
	//Interface, used to request data from/perform actions with the AI Framework
	IExamInterface* m_pInterface = nullptr;
	vector<HouseInfo> GetHousesInFOV() const;
	vector<EntityInfo> GetEntitiesInFOV() const;
	void SetUpAgents();

	bool m_CanRun = false; //Demo purpose
	bool m_GrabItem = false; //Demo purpose
	bool m_UseItem = false; //Demo purpose
	bool m_RemoveItem = false; //Demo purpose
	bool m_DropItem = false; //Demo purpose
	float m_AngSpeed = 0.f; //Demo purpose
};


//ENTRY
//This is the first function that is called by the host program
//The plugin returned by this function is also the plugin used by the host program
extern "C"
{
	__declspec (dllexport) IPluginBase* Register()
	{
		return new Plugin();
	}
}