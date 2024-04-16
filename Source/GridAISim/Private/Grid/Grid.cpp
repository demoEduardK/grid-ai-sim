// Fill out your copyright notice in the Description page of Project Settings.


#include "Grid/Grid.h"

#include "Actors/GameActorBase.h"
#include "GridAISim/GridAISim.h"

/*
 * We may use the rules of the Grid formation, like: Rect, Hex, Oct. Which will define the directions in which to check if there is a node
 * NW NN NE | [1;-1]	[1;0]	[1;1]
 * WW OO EE | [0;-1]	[0;0]	[0;1]
 * SW SS SE | [-1;-1]	[-1;0]	[-1;1]
 * Rect: NN, WW, EE, SS connections.
 * Hex: NW, NE, WW, EE, SW, SE connections.
 * Oct: All 8 directions.
 *
 * Using such rules will declare, that all the neighbors are 1 point away from the node with no distance calculations.
 * To check: if the coordinates of interest are in bounds of the Grid size and are not below zero
 * 0 <= X <= Grid.Size.X
 * 0 <= Y <= Grid.Size.Y
 */
// Grid point coordinates modifiers for neighbors look-up. 
static const TArray<FIntPoint> RectModifiers
	{{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

static const TArray<FIntPoint> HexModifiers
	{{1, 0}, {-1, 0}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

static const TArray<FIntPoint> OctModifiers
	{{-1, 1}, {0, 1}, {1, 1}, {-1, 0}, {1, 0}, {-1, -1}, {0, -1}, {1, -1}};

// Use mapping to get a required array based on the Grid type
static const TMap<EGridType, TArray<FIntPoint>> GridModifiersMapping
{
	{EGridType::Rectangular, RectModifiers},
	{EGridType::Hexagonal, HexModifiers},
	{EGridType::Octagonal, OctModifiers}
};
//--

void FGrid::PrintGrid() const
{
	for (auto& Point : GridArray)
	{
		UE_LOG(LogSim, Display, TEXT("%s"), *Point.GetDebugString());
	}
}

FString FGridPoint::GetDebugString() const
{
	const FString DebugString(TEXT("Index:{0}, X:{1}, Y:{2}, Actor:{3}"));
	return FString::Format(*DebugString, {
		                       *FString::FromInt(Index),
		                       *FString::FromInt(GridCoords.X),
		                       *FString::FromInt(GridCoords.Y),
		                       *GetNameSafe(GameActor)
	                       });
}

void FGrid::Init(int32 InSizeX, int32 InSizeY, EGridType InGridType)
{
	SizeX = InSizeX;
	SizeY = InSizeY;
	GridType = InGridType;
	//GridArray.Init(FGridPoint, SizeX * SizeY);
	GridArray.SetNumZeroed(SizeX * SizeY);

	for (int Rows = 0; Rows < SizeY; ++Rows)
	{
		for (int Cols = 0; Cols < SizeX; ++Cols)
		{
			FGridPoint& GridPoint = GridArray[Cols + (Rows * SizeY)];
			GridPoint.GridCoords = FIntPoint{Cols, Rows};
			GridPoint.Index = Cols + (Rows * SizeY);
		}
	}
}

const TArray<FGridPoint>& FGrid::GetGrid() const
{
	return GridArray;
}

FGridPoint& FGrid::At(int32 Index)
{
	checkf(Index < GridArray.Num(), TEXT("[FGrid::At] Argument Index out of bounds."));
	return GridArray[Index];
}

FGridPoint& FGrid::At(FIntPoint Coordinates)
{
	//const int32 Index = Coordinates.X * Coordinates.Y;
	const int32 Index = Coordinates.X + (Coordinates.Y * SizeY);
	checkf(Index < GridArray.Num(), TEXT("[FGrid::At] Coordinates out of bounds."));

	return GridArray[Index];
}

const FGridPoint& FGrid::At(FIntPoint Coordinates) const
{
	//const int32 Index = Coordinates.X * Coordinates.Y;
	const int32 Index = Coordinates.X + (Coordinates.Y * SizeY);
	checkf(Index < GridArray.Num(), TEXT("[FGrid::At] Coordinates out of bounds."));

	return GridArray[Index];
}

bool FGrid::FindRandomEmptyPointOnGrid(FGridPoint& OutGridPoint) const
{
	bool bResult = false;
	if (EmptyPoints.Num() <= 0)
	{
		return false;
	}

	const auto RandomIndex = FMath::RandRange(0, EmptyPoints.Num() - 1);
	OutGridPoint = EmptyPoints[RandomIndex];

	if(GridArray[OutGridPoint.Index].GameActor != nullptr)
	{
		UE_LOG(LogSim, Display, TEXT("[FindRandomEmptyPointOnGrid] Cell is occupied."));
	}
	EmptyPoints.RemoveAt(RandomIndex);
	return true;

	/*int32 RandomIndex = INDEX_NONE;
	const FGridPoint RandomPoint = FindRandomPointOnGrid(RandomIndex);

	// If the GridPoint is not empty
	if (RandomPoint.GameActor != nullptr)
	{
		// Go heavy, copy all empty slots into temp grid copy and get random there.
		TArray<FGridPoint> TempGrid = GridArray;
		for (auto& Point : GridArray)
		{
			if (Point.GameActor == nullptr)
			{
				TempGrid.Add(Point);
			}
		}
		if (TempGrid.Num() > 0)
		{
			OutGridPoint = TempGrid[FMath::RandRange(0, TempGrid.Num() - 1)];
			bResult = true;
		}
		else
		{
			// The whole grid is occupied? Most probably, some sort of a error
			OutGridPoint = FGridPoint();
		}
	}
	else
	{
		OutGridPoint = RandomPoint;
		bResult = true;
	}
	
	return bResult;*/
}

FGridPoint FGrid::FindRandomPointOnGrid(int32& OutRandomIndex) const
{
	checkf(GridArray.Num() > 0, TEXT("[FGrid::FindRandomPointOnGrid] Operation on an empty grid."));
	//OutRandomIndex = FMath::RandRange(0, GridArray.Num() - 1);
	//return GridArray[OutRandomIndex];
	OutRandomIndex = FMath::RandRange(0, EmptyPoints.Num() - 1);
	return GridArray[OutRandomIndex];
}

EGridType FGrid::GetGridType() const
{
	return EGridType::Rectangular;
}

TArray<FGridPoint> FGrid::GetNodeConnections(const FGridPoint& Point) const
{
	TArray<FGridPoint> ResultPoints;

	if (GridModifiersMapping.Contains(GridType))
	{
		for (auto& Modifier : GridModifiersMapping[GridType])
		{
			FIntPoint TargetPoint = Point.GridCoords + Modifier;
			if (IsPointOnGrid(TargetPoint))
			{
				ResultPoints.Emplace(FGridPoint(At(TargetPoint)));
			}
		}
	}
	return ResultPoints;
}

bool FGrid::IsPointOnGrid(const FIntPoint& Point) const
{
	return (Point.X >= 0 && Point.X < SizeX)
		&& (Point.Y >= 0 && Point.Y < SizeY);
}

void FGrid::OnStartSpawningActors()
{
	EmptyPoints = GridArray;
}

void FGrid::OnFinishSpawningActors()
{
	EmptyPoints.Empty();
}
