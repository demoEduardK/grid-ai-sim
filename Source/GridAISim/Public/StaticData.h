#pragma once

/***/
UENUM()
enum class ETeam
{
	NoTeam UMETA(DisplayName="NoTeam"),
	BlueTeam UMETA(DisplayName="BlueTeam"),
	RedTeam UMETA(DisplayName="RedTeam"),
	MAX UMETA(DisplayName="MaxTeams")
};

/**
 * Grid oriented coordinates
 */
struct FGridCoordinates_Dep
{
	FGridCoordinates_Dep();
	
	explicit FGridCoordinates_Dep(int32 X, int32 Y);
		
	int32 GridX = 0;
	int32 GridY = 0;

	bool operator==(const FGridCoordinates_Dep& InCoordinates) const;

	FGridCoordinates_Dep operator-(const FGridCoordinates_Dep& InCoordinates) const;

	// Static helpers / utility
	static const FGridCoordinates_Dep Empty()
	{
		return FGridCoordinates_Dep{-1, -1};
	};

	static float DistSqr(const FGridCoordinates_Dep& InCoordinatesLeft, const FGridCoordinates_Dep& InCoordinatesRight)
	{
		return FMath::Square(InCoordinatesLeft.GridX - InCoordinatesRight.GridX) + FMath::Square(
			InCoordinatesLeft.GridY - InCoordinatesRight.GridY);
	}

	FString ToString() const;
};
