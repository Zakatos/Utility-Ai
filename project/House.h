#pragma once
#include <Exam_HelperStructs.h>

class House
{
public:
	House(Elite::Vector2 worldDim) :m_WorldDim(worldDim) {};
	~House() {};

	void Update(float dTime, AgentInfo* agent, bool finishedScouting)
	{
		if (m_FirstTime == -1)
		{
			m_FirstTime = (m_WorldDim.x * m_WorldDim.y) / 230.0f;
			m_Maxtime = (m_WorldDim.x * m_WorldDim.y) / 350.0f;
		}

		if (m_ReachedCenter)
		{
			m_TimeOut += dTime;
			if (m_TimeOut > m_FirstTime && m_isFirsttime)
			{
				m_TimeOut = 0;
				m_ReachedCenter = false;
				m_isFirsttime = false;
			}
			else if (m_TimeOut > m_Maxtime && !m_isFirsttime)
			{
				m_TimeOut = 0;
				m_ReachedCenter = false;
			}

		}
		else
		{
			auto distanceSquared = (m_Center - agent->Position).Magnitude();
			if (distanceSquared < 1.0f)
				m_ReachedCenter = true;
		}
	}
	Elite::Vector2 m_WorldDim;
	Elite::Vector2 m_Center = Elite::Vector2();
	Elite::Vector2 m_Size = Elite::Vector2();
	bool m_ReachedCenter = false;
	float m_TimeOut = 0.0f;

private:
	float m_FirstTime = -1.0f;
	float m_Maxtime = -1.0f;
	bool m_isFirsttime = true;
	
};
