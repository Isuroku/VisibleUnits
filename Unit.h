#pragma once

#include "Vector2f.h"

//Class stored info about unit
class SUnit
{
public:
	SUnit(float inXPos, float inYPos, float inXDir, float inYDir): _pos(inXPos, inYPos), _dir(inXDir, inYDir), _near_count(0)
	{ 
		_dir = _dir.GetNormalized();
	}

	SUnit(const SUnit& other): _pos(other._pos), _dir(other._dir), _near_count(0)
	{
	}

	inline Vector2f GetPosition() const { return _pos; }
	inline Vector2f GetDirection() const { return _dir; }

	inline unsigned int GetNearCount() const { return _near_count; }
	inline void IncrementNearCount() { ++_near_count; }

private:

	//Position
	Vector2f _pos;
	//Direction
	Vector2f _dir;

	//Count of other units, which will be visible
	std::atomic<unsigned int> _near_count;
};
