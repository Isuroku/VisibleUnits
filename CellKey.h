#pragma once

//Class stores cell's coordinates in map grid
struct SCellKey
{
public:
	unsigned int X;
	unsigned int Y;

	SCellKey() : X(0), Y(0) { }
	SCellKey(unsigned int inX, unsigned int inY) : X(inX), Y(inY) { }
	SCellKey(const SCellKey& inValue) : X(inValue.X), Y(inValue.Y) { }

	SCellKey& operator=(const SCellKey& inValue) noexcept
	{
		X = inValue.X;
		Y = inValue.Y;
		return *this;
	}

	//Need for sorting of cells in grid's map
	bool operator<(const SCellKey& _Right) const noexcept
	{
		if (Y == _Right.Y)
			return X < _Right.X;
		return Y < _Right.Y;
	}
};
