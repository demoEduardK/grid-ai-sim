// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

enum class EGridType
{
	None,
	Rectangular,
	Hexagonal,
	Octagonal
};

/**
 * The struct to represent a grid element.
 */
struct FGridPoint
{
	FIntPoint GridCoords;
	TObjectPtr<class AGS_GameActorBase> GameActor = nullptr;
	int32 Index = 0;
	
	FString GetDebugString() const;

	bool operator==(const FGridPoint& InPoint) const
	{
		return GridCoords == InPoint.GridCoords;
	}
};

/**
 * The struct (but rather a class already) to represent the grid.
 */
struct FGrid
{
	/**
	 * Init Grid
	 * @param InSizeX Size of the X side of the Grid 
	 * @param InSizeY Size of the Y side of the Grid
	 */
	void Init(int32 InSizeX, int32 InSizeY, EGridType InGridType = EGridType::Rectangular);

	void PrintGrid() const;

	/**
	 * A getter for the Grid Array
	 * @return GridArray
	 */
	const TArray<FGridPoint>& GetGrid() const;

	/**
	 * Get Grid element at index
	 * @param Index Element index
	 * @return Element at index
	 */
	FGridPoint& At(int32 Index);

	/**
	 * Get Grid element by coordinates
	 * @param Coordinates Element coordinates
	 * @return Element at coordinates
	 */
	FGridPoint& At(FIntPoint Coordinates);

	/**
	 * Get Grid element by coordinates. Constant version
	 * @param Coordinates Element coordinates
	 * @return Element at coordinates
	 */
	const FGridPoint& At(FIntPoint Coordinates) const;
	
	/**
	* @return Returns a random position on Grid that is not yet occupied
	*/
	bool FindRandomEmptyPointOnGrid(FGridPoint& OutGridPoint) const;

	/**
	* @return Returns a random position on Grid
	*/
	FGridPoint FindRandomPointOnGrid(int32& OutRandomIndex) const;

	EGridType GetGridType() const;

	TArray<FGridPoint> GetNodeConnections(const FGridPoint& Point) const;

	bool IsPointOnGrid(const FIntPoint& Point) const;

	// These two methods should be called before and after the spawning of actors
	// TODO: consider moving the spawning functionality to under the grid responsibility, or under some generator class
	void OnStartSpawningActors();
	void OnFinishSpawningActors();
private:
	TArray<FGridPoint> GridArray;
	int32 SizeX = 0;
	int32 SizeY = 0;
	EGridType GridType = EGridType::None;

	// Should be populated before and cleared after the spawning stage 
	mutable TArray<FGridPoint> EmptyPoints;
};