#pragma once
#include "Curves.h"

//consideration
class Utility_Consideration
{
public:
	Utility_Consideration(float score = 0, float weight = 1.0f) :
		m_PropertyScore(score),
		m_Weight(weight) {};
	~Utility_Consideration()
	{
		if (m_pCurve)
		{
			delete m_pCurve;
			m_pCurve = nullptr;
		}

	};
	float GetUtilityScore() const { return m_pCurve->Evaluate(m_PropertyScore); }
	void SetCurve(Curve* curve) { m_pCurve = curve; }
	float GetWeight() const { return m_Weight; }
	void SetWeight(float weight) { m_Weight = weight; }
	void SetScore(float score) { m_PropertyScore = score; }//input neurons go here
private:
	float m_PropertyScore;
	float m_Weight = 1.0f;
	Curve* m_pCurve = nullptr;

};