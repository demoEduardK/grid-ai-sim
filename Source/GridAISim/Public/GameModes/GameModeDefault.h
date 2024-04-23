// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "StaticData.h"
#include "Grid/Grid.h"
#include "GameModeDefault.generated.h"

USTRUCT(Blueprintable)
struct GRIDAISIM_API FUnitsCountData
{
	GENERATED_BODY()
	
	TSubclassOf<class AGS_GameActorBase> UnitClass;
	int32 UnitCount;
};

class AGS_GridTestActor;
/**
 * 
 */
UCLASS()
class GRIDAISIM_API AGS_GameModeDefault : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGS_GameModeDefault(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void PostInitializeComponents() override;

	virtual void Tick(float DeltaSeconds) override;
	
	virtual void BeginPlay() override;

	void SpawnActorAt(TSubclassOf<AGS_GameActorBase> ActorClass, FIntPoint GridPoint, ETeam Team);

	UFUNCTION(BlueprintCallable, Category="Simulation Control", DisplayName="Start Simulation")
	void K2_StartSimulation();
	
protected:
	
	
	// The X size of grid to generate.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GameSettings")
	int32 GridSizeX = 100;

	// The Y size of grid to generate.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GameSettings")
	int32 GridSizeY = 100;

	// The size of grid cells for scaling to the world coordinates
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GameSettings")
	float GridCellSize = 50.f;

	// The number of actor each team will have
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GameSettings")
	int32 NumberOfActorsPerTeam = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GameSettings")
	FUnitsCountData UnitsCountData;

	// Generally, we don't need this variable, but for now I simply use it instead of magic numbers for spawning
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GameSettings")
	int32 NumberOfTeams = 2;

	// The subclass to be used for the simulation. It's just a single class at the moment tho
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GameSettings")
	TSubclassOf<AGS_GameActorBase> ActorClass;

	// The subclass to be used for the simulation. It's just a single class at the moment tho
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GameSettings")
	TArray<TSubclassOf<AGS_GameActorBase>> ActorClasses;

	/*
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GameSettings")
	TArray<FUnitsCountData> ActorClasses;
	 */
	// The subclass to be used for the simulation. It's just a single class at the moment tho
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GameSettings")
	TSubclassOf<AGS_GridTestActor> GridActorDummyClass;
	
	// Minimum Attack power that will be used for random AttackPower setup
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GameSettings|ActorsSetting|Attack")
	float AttackPowerMin = 1.f;
	// Maximum Attack power that will be used for random AttackPower setup
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GameSettings|ActorsSetting|Attack")
	float AttackPowerMax = 10.f;
	// Minimum Health that will be used for random Health setup
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GameSettings|ActorsSetting|Health")
	float HealthPointsMin = 1.f;
	// Maximum Health that will be used for random Health setup
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GameSettings|ActorsSetting|Health")
	float HealthPointsMax = 10.f;
	// TimeStep duration that will be used for simulation.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GameSettings")
	float SimulationTimeStep_ms = 0.1f;
	

private:
	/**
	 * Simple Actor spawning method
	 */
	void SpawnActors();

	/**
	 * Starts the simulation
	 */
	void StartSimulation();

	/**
	 * Ends the simulation
	 */
	void EndSimulation(/*EReason*/);
	void IsSimulationOver();

	/**
	 * Or rather a Step, not a turn.. A simulation iteration functional unit.
	 */
	void MakeSimulationTurn();

	/**
	 * Find the closest opponent
	 * @param InActor An actor to look opponents for
	 * @param OutDistanceSqr Square distance to the found opponent 
	 * @return Pointer to the found opponent
	 */
	AGS_GameActorBase* FindClosestActor(AGS_GameActorBase* InActor, int32& OutDistanceSqr);
	
	void ActorAttack(AGS_GameActorBase* InTargetActor, AGS_GameActorBase* InInstigatorActor);

	FIntPoint GetNextMoveLocation(AGS_GameActorBase* InActionActor, AGS_GameActorBase* InTargetActor, const FGrid& InGrid);
	void ActorMoveTowards(AGS_GameActorBase* InTargetActor, AGS_GameActorBase* InInstigatorActor);

	void HandleActorKilled(AGS_GameActorBase* InTargetActor, AGS_GameActorBase* InInstigatorActor);
	
	/**
	 * A conversion method to receive Global coordinates from the Grid Coordinates
	 * @param GridX Grid X coordinate
	 * @param GridY Grid Y coordinate
	 * @return Global Coordinates
	 */
	FVector GridToGlobal(const FIntPoint& GridCoordinates ) const;

	/**
	 * Spawns info actors at each Grid cell
	 */
	void TestGrid();

	// TODO: Move it to GameState.
	// An array of Game Actors.
	TArray<class AGS_GameActorBase*> GameActors;

	TMap<ETeam, int32> ActorsNumPerTeam;

	// An array of Game Actors pending to be destroyed.
	TArray<class AGS_GameActorBase*> KilledGameActors;

	// The grid
	FGrid Grid;

	// A bool flag to check if simulation is active
	bool bSimulationOngoing = false;

	// A counter to accumulate delta time from ticks to simulate TimeSteps
	float TimeStepAccumulator = 0.f;

	TPimplPtr<class GS_Pathfinder> Pathfinder;
};
