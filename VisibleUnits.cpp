#include <iostream>
#include <vector>
#include <map>
#include <chrono>
#include <future>
#include "SerializationUtils.h"
#include "CellKey.h"

using namespace std;

//Files for input and output
const string UnitsFileName("Units.txt");
const string UnitsOutputFileName("UnitOutput.txt");

//Math constants
const float SQRT_2(sqrtf(2));
const float PI = 3.14159265358979323846264f;
const float D2R(PI / 180); //for transform dergee to radians

std::atomic<unsigned int> AsyncCount;

//Map grid type
//Use tree map (not hash map), because we need in sort cells from left to right and bottom to top
//key - coords of cell; value - vector of unit indexes in unit vector
typedef map<SCellKey, vector<size_t>, less<SCellKey>> CUnitMap;

//Collect all units to cells in grid
void CreateMap(float inViewDist, const Vector2f& inMinCorner, vector<SUnit>& inAllUnits, CUnitMap& outCells);

//Units of every cell check its visibility
void IteratingMapCells(const CUnitMap& inCells, float inViewDistSq, float inCosHalfSector, vector<SUnit>& inAllUnits);

//Units of every cell check its visibility
void IteratingMapCell(const SCellKey& inCellKey, const vector<size_t>& inCellIndexesVector, const CUnitMap& inCells,
	float inViewDistSq, float inCosHalfSector, vector<SUnit>& inAllUnits);

//inUnit check visibility
void CheckUnitVisibility(size_t inUnitIndex1, float inViewDistSq, float inCosHalfSector, const vector<size_t>& inIndexes, vector<SUnit>& inAllUnits, size_t inStartIndex = 0);

//Units check visibility to each other
void CheckUnitsVisibilityEachOther(size_t inUnitIndex1, size_t inUnitIndex2, float inViewDistSq, float inCosHalfSector, vector<SUnit>& inAllUnits);

int main()
{
	SGameParams game_params;
	vector<SUnit> units;

	auto start_time = chrono::high_resolution_clock::now();

	//read data file
	if (!ReadUnits(UnitsFileName, game_params, units))
		return 1;

	auto stop_time = chrono::high_resolution_clock::now();
	auto read_mseconds = chrono::duration_cast<std::chrono::microseconds>(stop_time - start_time);

	start_time = chrono::high_resolution_clock::now();

	//Set all units in cells in map grid
	CUnitMap cells;
	CreateMap(game_params.ViewDist, game_params.MinCorner, units, cells);

	float view_dist_sq = game_params.ViewDist * game_params.ViewDist;
	float angle_cos = cos(game_params.ViewSector / 2 * D2R);
	//iterating all cells and units into
	IteratingMapCells(cells, view_dist_sq, angle_cos, units);

	/*while (AsyncCount > 0)
	{
		Sleep(0);
	}*/

	stop_time = chrono::high_resolution_clock::now();
	auto calc_mseconds = chrono::duration_cast<std::chrono::microseconds>(stop_time - start_time);

	start_time = chrono::high_resolution_clock::now();
	//write results
	WriteUnitNeatCounts(UnitsOutputFileName, units);

	stop_time = chrono::high_resolution_clock::now();
	auto write_mseconds = chrono::duration_cast<std::chrono::microseconds>(stop_time - start_time);

	std::cout << "Unit count " << units.size() << endl;
	std::cout << "read " << read_mseconds.count() << " mksec" << endl;
	std::cout << "calc " << calc_mseconds.count() << " mksec" << endl;
	std::cout << "write " << write_mseconds.count() << " mksec" << endl;

	return 0;
}

//Collect all units to cells in grid
void CreateMap(float inViewDist, const Vector2f& inMinCorner, vector<SUnit>& inAllUnits, CUnitMap& outCells)
{
	//Square cell diagonal - viewing distance
	//so all units in one cell can see to each other by distance
	//may be we could optimize check distance (unfortunately not)

	float cell_width = inViewDist / SQRT_2;

	for (size_t i = 0; i < inAllUnits.size(); i++)
	{
		const SUnit& u = inAllUnits[i];

		//changing in position (use new coord center) allows us use only positive cell coords
		Vector2f rel_pos = u.GetPosition() - inMinCorner;

		SCellKey key;
		key.X = (unsigned int)(rel_pos.X / cell_width);
		key.Y = (unsigned int)(rel_pos.Y / cell_width);

		//may be cell present already
		const auto it = outCells.find(key);
		if (it == outCells.end())
		{
			//create cell and set index of current unit
			vector<size_t> vec(1);
			vec[0] = i;
			outCells.insert(make_pair(key, vec));
		}
		else
		{
			//add neighbour unit in cell
			vector<size_t>& vec = it->second;
			vec.push_back(i);
		}
	}
}

//Units of every cell check its visibility
void IteratingMapCells(const CUnitMap& inCells, float inViewDistSq, float inCosHalfSector, vector<SUnit>& inAllUnits)
{
	for (CUnitMap::const_iterator it = inCells.begin(); it != inCells.end(); it++)
	{
		SCellKey cell_key = it->first;
		const vector<size_t>& vec = it->second;
		//async(IteratingMapCell, ref(cell_key), ref(vec), ref(inCells), inViewDistSq, inCosHalfSector, ref(inAllUnits));
		IteratingMapCell(cell_key, vec, inCells, inViewDistSq, inCosHalfSector, inAllUnits);
	}
}

//Units of every cell check its visibility
void IteratingMapCell(const SCellKey& inCellKey, const vector<size_t>& inCellIndexesVector, const CUnitMap& inCells,
	float inViewDistSq, float inCosHalfSector, vector<SUnit>& inAllUnits)
{
	AsyncCount++;

	//around current cell eight neighbours cells (except border cells)
		//other cells are too far
		//we need check current cell and four next
		//previous four cells checked us already

	CUnitMap::const_iterator neib_it_right = inCells.find(SCellKey(inCellKey.X + 1, inCellKey.Y));
	CUnitMap::const_iterator neib_it_left_up = inCellKey.X > 0 ? inCells.find(SCellKey(inCellKey.X - 1, inCellKey.Y + 1)) : inCells.end();
	CUnitMap::const_iterator neib_it_up = inCells.find(SCellKey(inCellKey.X, inCellKey.Y + 1));
	CUnitMap::const_iterator neib_it_right_up = inCells.find(SCellKey(inCellKey.X + 1, inCellKey.Y + 1));

	for (size_t i = 0; i < inCellIndexesVector.size(); i++)
	{
		size_t index1 = inCellIndexesVector[i];

		//current unit check next units in cell: i + 1
		//prev units checked current unit already
		size_t st_i = i + 1;
		//future<void> f = async(CheckUnitVisibility, index1, inViewDistSq, inCosHalfSector, ref(inCellIndexesVector), ref(inAllUnits), st_i);
		CheckUnitVisibility(index1, inViewDistSq, inCosHalfSector, inCellIndexesVector, inAllUnits, st_i);

		//check all units in four neighbours celss
		if (neib_it_right != inCells.end())
		{
			const vector<size_t>& vec2 = neib_it_right->second;
			//async(CheckUnitVisibility, index1, inViewDistSq, inCosHalfSector, ref(vec2), ref(inAllUnits), 0);
			CheckUnitVisibility(index1, inViewDistSq, inCosHalfSector, vec2, inAllUnits);
		}

		if (neib_it_left_up != inCells.end())
		{
			const vector<size_t>& vec2 = neib_it_left_up->second;
			//async(CheckUnitVisibility, index1, inViewDistSq, inCosHalfSector, ref(vec2), ref(inAllUnits), 0);
			CheckUnitVisibility(index1, inViewDistSq, inCosHalfSector, vec2, inAllUnits);
		}

		if (neib_it_up != inCells.end())
		{
			const vector<size_t>& vec2 = neib_it_up->second;
			//async(CheckUnitVisibility, index1, inViewDistSq, inCosHalfSector, ref(vec2), ref(inAllUnits), 0);
			CheckUnitVisibility(index1, inViewDistSq, inCosHalfSector, vec2, inAllUnits);
		}

		if (neib_it_right_up != inCells.end())
		{
			const vector<size_t>& vec2 = neib_it_right_up->second;
			//async(CheckUnitVisibility, index1, inViewDistSq, inCosHalfSector, ref(vec2), ref(inAllUnits), 0);
			CheckUnitVisibility(index1, inViewDistSq, inCosHalfSector, vec2, inAllUnits);
		}
	}
	AsyncCount--;
}

//inUnit check visibility
//inStartIndex - for check inUnit's cell
void CheckUnitVisibility(size_t inUnitIndex1, float inViewDistSq, float inCosHalfSector, const vector<size_t>& inIndexes, vector<SUnit>& inAllUnits, size_t inStartIndex)
{
	for (size_t i = inStartIndex; i < inIndexes.size(); i++)
	{
		size_t index = inIndexes[i];
		CheckUnitsVisibilityEachOther(inUnitIndex1, index, inViewDistSq, inCosHalfSector, inAllUnits);
	}
}

//Units check visibility to each other
void CheckUnitsVisibilityEachOther(size_t inUnitIndex1, size_t inUnitIndex2, float inViewDistSq, float inCosHalfSector, vector<SUnit>& inAllUnits)
{
	SUnit& unit1 = inAllUnits[inUnitIndex1];
	SUnit& unit2 = inAllUnits[inUnitIndex2];
	float d2 = Vector2f::GetDistanceSq(unit1.GetPosition(), unit2.GetPosition());
	if (d2 > inViewDistSq)
		return; //too far away

	float dist = sqrt(d2);
	if (dist <= Vector2f::MIN_LENGTH)
	{
		//units has equal position
		//we think that they see each other
		unit1.IncrementNearCount();
		unit2.IncrementNearCount();
		return;
	}

	Vector2f r1 = unit2.GetPosition() - unit1.GetPosition();
	//Normalization - we already have distance, not need it again
	r1.X /= dist;
	r1.Y /= dist;

	//both vectors are unit (len 1)
	//inUnit1.Dir was normalized on load
	//in this case: dot product == cos of angle between vectors
	float acos = Vector2f::GetDotProduct(r1, unit1.GetDirection());
	if (acos > inCosHalfSector)
	{
		unit1.IncrementNearCount();
	}

	Vector2f r2 = -1 * r1;
	acos = Vector2f::GetDotProduct(r2, unit2.GetDirection());
	if (acos > inCosHalfSector)
	{
		unit2.IncrementNearCount();
	}
}