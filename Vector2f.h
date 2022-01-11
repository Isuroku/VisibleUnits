#pragma once
#include <algorithm>

using namespace std;

//Class for work 2D coordinates
struct Vector2f
{
public:
	static const double MIN_LENGTH;

	float X;
	float Y;

	Vector2f() : X(0), Y(0) { }
	Vector2f(float inX, float inY): X(inX), Y(inY) { }
	Vector2f(const Vector2f& inValue) : X(inValue.X), Y(inValue.Y) { }

	Vector2f& operator=(const Vector2f& inValue) noexcept
	{
		X = inValue.X;
		Y = inValue.Y;
		return *this;
	}

	inline static Vector2f GetMinCorner(const Vector2f& v1, const Vector2f& v2)
	{
		return Vector2f(min(v1.X, v2.X), min(v1.Y, v2.Y));
	}

	inline static float GetDistanceSq(const Vector2f& v1, const Vector2f& v2)
	{
		float dx = v1.X - v2.X;
		float dy = v1.Y - v2.Y;
		return dx*dx + dy*dy;
	}

	inline static float GetDotProduct(const Vector2f& v1, const Vector2f& v2)
	{
		return v1.X * v2.X + v1.Y * v2.Y;
	}

	inline float GetLengthSq() const
	{
		return X * X + Y * Y;
	}

	inline float GetLength() const
	{
		return sqrtf(GetLengthSq());
	}

	inline Vector2f GetNormalized() const
	{
		float l = GetLength();
		if (abs(l) < MIN_LENGTH)
			return Vector2f();
		return Vector2f(X / l, Y / l);
	}
};

inline Vector2f operator-(const Vector2f& lfa, const Vector2f& rfa)
{
	return Vector2f(lfa.X - rfa.X, lfa.Y - rfa.Y);
}

inline Vector2f operator*(const Vector2f& lfa, float rfa)
{
	return Vector2f(lfa.X * rfa, lfa.Y * rfa);
}

inline Vector2f operator*(float lfa, const Vector2f& rfa)
{
	return Vector2f(rfa.X * lfa, rfa.Y * lfa);
}