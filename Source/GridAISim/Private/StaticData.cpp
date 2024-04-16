#include "StaticData.h"


FGridCoordinates_Dep::FGridCoordinates_Dep()
	: GridX(0), GridY(0)
{}

FGridCoordinates_Dep::FGridCoordinates_Dep(int32 X, int32 Y)
{
	GridX = X;
	GridY = Y;
}

bool FGridCoordinates_Dep::operator==(const FGridCoordinates_Dep& InCoordinates) const
{
	return (GridX == InCoordinates.GridX) && (GridY == InCoordinates.GridY);
}

FGridCoordinates_Dep FGridCoordinates_Dep::operator-(const FGridCoordinates_Dep& InCoordinates) const
{
	return FGridCoordinates_Dep{this->GridX - InCoordinates.GridX, this->GridY - InCoordinates.GridY};
}

FString FGridCoordinates_Dep::ToString() const
{
	const FString DebugString(TEXT("X:{0}, Y:{1}"));
	return FString::Format(*DebugString, {	*FString::FromInt(GridX), *FString::FromInt(GridY)});
}