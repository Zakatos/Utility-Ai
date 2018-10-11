#pragma once
#include <cmath>

//Here are the curves structures that are used for the implementation of the Utility Ai
struct Curve
{
public:
	Curve() {};
	~Curve() {};
	float a = 1;
	float b = 0;
	virtual float Evaluate(float x) const = 0;
};

struct LinearCurve : Curve
{
public:
	float a = 1;
	float b = 0;
	float Evaluate(float x) const override
	{
		return a * x + b;
	}
};

struct StepCurve : Curve
{
public:
	float a = 0.5f;
	float b = 1.0;
	float Evaluate(float x) const override
	{
		return x > a ? b : -b;
	}
};

struct ExpCurve : Curve
{
public:
	float a = 2.0f;
	float b = 0;
	float Evaluate(float x) const override
	{
		return pow(x, a);
	}
};

struct SigmCurve : Curve
{
public:
	float a = 1.0f;
	float b = 0;
	float Evaluate(float x) const override
	{
		return 1.0f / (1.0f + pow(exp(1.0f), x)) + a;
	}
};