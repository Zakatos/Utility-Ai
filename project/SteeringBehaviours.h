#pragma once
#include "Exam_HelperStructs.h"

namespace SteeringBehaviours
{
#pragma region **ISTEERINGBEHAVIOUR** (BASE)
	class ISteeringBehaviour
	{
	public:
		ISteeringBehaviour() {}
		virtual ~ISteeringBehaviour() {}

		virtual SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) = 0;

		template<class T, typename std::enable_if<std::is_base_of<ISteeringBehaviour, T>::value>::type* = nullptr>
		T* As()
		{
			return static_cast<T*>(this);
		}
		const Elite::Vector2* GetTargetPos() const { return m_pTargetRef; }

	protected:
	
		
		const Elite::Vector2* m_pTargetRef;
	};
#pragma endregion

	//SEEK
	//****
	class Seek : public ISteeringBehaviour
	{
	public:
		Seek() {};
		virtual ~Seek() {};

		//Seek Behaviour
		SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;

		//Seek Functions
		virtual void SetTarget(const Elite::Vector2* pTarget) { m_pTargetRef = pTarget; }

	
	};

	//FLEE
	//****
	class Flee : public Seek
	{
	public:
		Flee() {};
		virtual ~Flee() {};

		//Seek Behaviour
		SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;
	};

	//WANDER
	//******
	class Wander : public Seek
	{
	public:
		Wander() {};
		virtual ~Wander() {};

		//Wander Behaviour
		SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;

		void SetWanderOffset(float offset) { m_Offset = offset; }
		void SetWanderRadius(float radius) { m_Radius = radius; }
		void SetMaxAngleChange(float rad) { m_AngleChange = rad; }

	protected:

		float m_Offset = 6.f; //Offset (Agent Direction)
		float m_Radius = 4.f; //WanderRadius
		float m_AngleChange = Elite::ToRadians(45); //Max WanderAngle change per frame
		float m_WanderAngle = 0.f; //Internal

	private:
		void SetTarget(const Elite::Vector2* pTarget) override {} //Hide SetTarget, No Target needed for Wander
	};

	//ARRIVE
	//******
	class Arrive : public ISteeringBehaviour
	{
	public:
		Arrive() {};
		virtual ~Arrive() {};

		//Seek Behaviour
		SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;

		//Seek Functions
		virtual void SetTarget(const Elite::Vector2* pTarget) { m_pTargetRef = pTarget; }
		void SetSlowRadius(float radius) { m_SlowRadius = radius; }

	protected:

		const Elite::Vector2* m_pTargetRef;
		float m_SlowRadius = 10.f;
		float m_TargetRadius = 2.f;
	};


}