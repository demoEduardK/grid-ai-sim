// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/GameModeDefault.h"
#include "Actors/GameActorBase.h"
#include "Grid/GridTestActor.h"
#include "Grid/Pathfinder.h"
#include "GridAISim/GridAISim.h"


AGS_GameModeDefault::AGS_GameModeDefault(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;

	bStartPlayersAsSpectators = true;
	ActorClass = AGS_GameActorBase::StaticClass();

	//GS_Pathfinder = MakeUnique<IT_GS_Pathfinder>();
	Pathfinder = MakePimpl<GS_Pathfinder>();
}

void AGS_GameModeDefault::TestGrid()
{
	for (auto& Point : Grid.GetGrid())
	{
		if (auto* const World = GetWorld())
		{
			FTransform SpawnTransform;
			SpawnTransform.SetLocation(GridToGlobal(Point.GridCoords));
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AGS_GridTestActor* GridActor = World->SpawnActor<AGS_GridTestActor>(
				GridActorDummyClass, SpawnTransform, Params);
			GridActor->DebugText.Append(FString::FromInt(Point.Index)).Append("\n").Append(Point.GridCoords.ToString());
		}
	}
}

void AGS_GameModeDefault::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	Grid.Init(GridSizeX, GridSizeY, EGridType::Rectangular);

	if (Pathfinder.IsValid())
	{
		Pathfinder->InitGraph(Grid);
	}
}

void AGS_GameModeDefault::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bSimulationOngoing)
	{
		TimeStepAccumulator += DeltaSeconds;

		if (TimeStepAccumulator >= SimulationTimeStep_ms)
		{
			MakeSimulationTurn();

			TimeStepAccumulator = 0.f;
		}
	}
}

void AGS_GameModeDefault::BeginPlay()
{
	Super::BeginPlay();

	SpawnActors();

	// For now, simulation start is triggered from K2_StartSimulation method
	//StartSimulation();
}

void AGS_GameModeDefault::SpawnActorAt(TSubclassOf<AGS_GameActorBase> InActorClass, FIntPoint InGridPoint, ETeam InTeam)
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		UE_LOG(LogSim, Warning, TEXT("[SpawnActorAt] World is nullptr."))
		return;
	}

	AGS_GameActorBase* SpawnedActor = World->SpawnActorDeferred<AGS_GameActorBase>(
		ActorClass,
		FTransform(), nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	SpawnedActor->SetTeam(InTeam);

	SpawnedActor->SetAttackPower(FMath::RandRange(AttackPowerMin, AttackPowerMax));
	SpawnedActor->SetHealthPoints(FMath::RandRange(HealthPointsMin, HealthPointsMax));

	FGridPoint& GridPoint = Grid.At(InGridPoint);
	GridPoint.GameActor = SpawnedActor;

	FTransform SpawnTransform{};
	SpawnTransform.SetLocation(GridToGlobal(GridPoint.GridCoords));

	GridPoint.GameActor = SpawnedActor;
	SpawnedActor->SetGridPointIndex(Grid.At(GridPoint.Index).Index);
	SpawnedActor->SetGridCoordinates(InGridPoint);

	SpawnedActor->FinishSpawning(SpawnTransform);

	GameActors.Add(SpawnedActor);
}

void AGS_GameModeDefault::K2_StartSimulation()
{
	StartSimulation();
}

void AGS_GameModeDefault::SpawnActors()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogSim, Display, TEXT("[SpawnActors] World ptr is nullptr."));
		return;
	}

	TArray<FGridPoint> SpawnLocations;
	Grid.OnStartSpawningActors();
	// Spawn actors

	for(const auto ActorTypeClass : ActorClasses)
	{
	for (int32 Index = 0; Index < NumberOfActorsPerTeam * 2/*NumberOfTeams*/; ++Index)
	{
		// First create an actor and populate it with required gameplay information
		AGS_GameActorBase* SpawnedActor = World->SpawnActorDeferred<AGS_GameActorBase>(
			ActorTypeClass,
			FTransform(), nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		SpawnedActor->SetTeam(Index % 2 ? ETeam::BlueTeam : ETeam::RedTeam);

		// Leave these settings here for the further ability to apply GameMode's modifications as well.
		SpawnedActor->SetActionDuration(SimulationTimeStep_ms);
		// TODO: Decide how the attribute initialization should look like. Should Gamemode decide the random fraction or just leave it to the actor?
		SpawnedActor->InitAttributes(0.f, 0.f);

		// Next, pass it to the Grid, or GridManager, or GridGenerator to register it on the grid
		// if( bool bSuccessful = RegisterActor(SpawnedActor) )

		//re-make FindRandomEmptyPointOnGrid to return an Index, I guess. Get Point ref by that point then
		FGridPoint GridPoint;
		Grid.FindRandomEmptyPointOnGrid(GridPoint);
		FGridPoint& GridPointRef = Grid.At(GridPoint.Index);
		if (GridPointRef.GameActor)
		{
			UE_LOG(LogSim, Display, TEXT("[SpawnActors] Received an occupied grid point."));
		}
		// Do Grid related stuff here.
		GridPointRef.GameActor = SpawnedActor;
		SpawnedActor->SetGridPointIndex(GridPointRef.Index);
		SpawnedActor->SetGridCoordinates(GridPointRef.GridCoords);


		// --
		// Next, with an Actor registered on the Grid, it should know it's Grid coordinates.
		// Use them to finalize the spawn
		FTransform FinalTransform;
		FinalTransform.SetLocation(GridToGlobal(SpawnedActor->GetGridCoordinates()));
		SpawnedActor->FinishSpawning(FinalTransform);

		++(ActorsNumPerTeam.FindOrAdd(SpawnedActor->GetTeam()));
		GameActors.Add(SpawnedActor);
	}
		
	} //~ for actor classes
	Grid.OnFinishSpawningActors();
}

void AGS_GameModeDefault::StartSimulation()
{
	bSimulationOngoing = true;
}

void AGS_GameModeDefault::EndSimulation()
{
	bSimulationOngoing = false;
	UE_LOG(LogSim, Display, TEXT("[EndSimulation] Simulation is over."))
}

void AGS_GameModeDefault::IsSimulationOver()
{
	for (auto& TeamActorsNum : ActorsNumPerTeam)
	{
		// Technically we check, if any of teams is the only one that is left on the board.
		if (TeamActorsNum.Value == GameActors.Num())
		{
			EndSimulation();
			break;
		}
	}
}

void AGS_GameModeDefault::MakeSimulationTurn()
{
	// If there is just one, or even no Actors - cease the simulation
	// TODO: remove or modify this condition into "CanStartSimultaionTurn" 
	if (GameActors.Num() <= 1)
	{
		UE_LOG(LogSim, Display, TEXT("[MakeSimulationTurn] Simulation is over."))
		bSimulationOngoing = false;
		return;
	}

	// for each actor:
	for (auto* Actor : GameActors)
	{
		// find closest
		int32 DistanceSqr = 0;
		if (auto* TargetActor = FindClosestActor(Actor, DistanceSqr))
		{
			if (DistanceSqr <= FMath::Square(Actor->GetAttackRange()))
			{
				ActorAttack(TargetActor, Actor);
			}
			else
			{
				ActorMoveTowards(TargetActor, Actor);
			}
		}
		else
		{
			Actor->Halt();
			UE_LOG(LogSim, Warning,
			       TEXT("[MakeSimulationTurn] Failed to find the closest actor."));
		}
	}

	// Clean-up
	for (auto* Actor : KilledGameActors)
	{
		GameActors.Remove(Actor);
		// TODO: Let the actor HandleZeroHealth as well as self-destroy
		//Actor->StartDestroy();
	}
	KilledGameActors.Empty();

	// Check simulation end conditions
	IsSimulationOver();
}

AGS_GameActorBase* AGS_GameModeDefault::FindClosestActor(AGS_GameActorBase* InActor, int32& OutDistanceSqr)
{
	AGS_GameActorBase* TargetActor = nullptr;

	if (InActor == nullptr)
	{
		UE_LOG(LogSim, Warning, TEXT("[FindClosestActor] Use of null pointer."))
		return TargetActor;
	}

	int32 ClosestDist = -1;
	AGS_GameActorBase* ClosestTarget = nullptr;
	// TODO: upon adding new actors, put them into separate arrays, and iterate through the opponents array here.
	for (auto* Actor : GameActors)
	{
		if (Actor == InActor || !Actor->IsAlive())
		{
			continue;
		}

		if (InActor->GetTeam() != Actor->GetTeam())
		{
			FIntPoint Diff = InActor->GetGridCoordinates() - Actor->GetGridCoordinates();
			const int32 DistSqr = FIntPoint(InActor->GetGridCoordinates() - Actor->GetGridCoordinates()).SizeSquared();

			if ((ClosestDist < 0) || (DistSqr < ClosestDist))
			{
				ClosestDist = DistSqr;
				ClosestTarget = Actor;
			}

			// Maybe, this is a good idea to break as soon as we find an actor that is one square away from us.
			if (DistSqr <= 1)
			{
				break;
			}
		}
	}

	if (ClosestTarget != nullptr)
	{
		OutDistanceSqr = ClosestDist;
	}

	return ClosestTarget;
}

void AGS_GameModeDefault::ActorAttack(AGS_GameActorBase* InTargetActor, AGS_GameActorBase* InActionActor)
{
	UE_LOG(LogSim, Display, TEXT("[ActorAttack] Target: %s, Instigator %s."),
	       *GetNameSafe(InTargetActor), *GetNameSafe(InActionActor));

	if (!InTargetActor || !InActionActor)
	{
		UE_LOG(LogSim, Warning, TEXT("[ActorAttack] One of the actors is nullptr."));
		return;
	}
	InTargetActor->SetHealthPoints(InTargetActor->GetHealthPoints() - InActionActor->GetAttackPower());
	InActionActor->PlayAttack(InTargetActor);
	if (InTargetActor->GetHealthPoints() <= 0.f)
	{
		HandleActorKilled(InTargetActor, InActionActor);
	}
}

FIntPoint AGS_GameModeDefault::GetNextMoveLocation(AGS_GameActorBase* InActionActor, AGS_GameActorBase* InTargetActor,
                                                   const FGrid& InGrid)
{
	FIntPoint ResultPoint = FIntPoint::ZeroValue;

	if (InTargetActor != nullptr)
	{
		FGridPoint CurrentGridPoint = InGrid.At(InActionActor->GetGridCoordinates());
		const TArray<FGridPoint> NeighborPoints = Grid.GetNodeConnections(CurrentGridPoint);

		int32 LeastDistance = FIntPoint(InTargetActor->GetGridCoordinates() - InActionActor->GetGridCoordinates()).
			SizeSquared();

		auto IsCloserThanBefore = [&LeastDistance](const FIntPoint& Left, const FIntPoint& Right)
		{
			return FIntPoint(Left - Right).SizeSquared() < LeastDistance;
		};

		for (const auto& Point : NeighborPoints)
		{
			const auto PointCoordinates = Point.GridCoords;
			if (Point.GameActor == nullptr && IsCloserThanBefore(PointCoordinates, InTargetActor->GetGridCoordinates()))
			{
				LeastDistance = FIntPoint(PointCoordinates - InTargetActor->GetGridCoordinates()).SizeSquared();
				ResultPoint = PointCoordinates;
			}
		}
	}

	return ResultPoint;
}

void AGS_GameModeDefault::ActorMoveTowards(AGS_GameActorBase* InTargetActor, AGS_GameActorBase* InActionActor)
{
	UE_LOG(LogSim, Display, TEXT("[ActorMove] Target: %s, ActionActor %s."),
	       *GetNameSafe(InTargetActor), *GetNameSafe(InActionActor));

	if (InTargetActor == nullptr || InActionActor == nullptr)
	{
		UE_LOG(LogSim, Warning, TEXT("[ActorMove] Nullptr is passed as an argument."));
		return;
	}

	FIntPoint NextMove = GetNextMoveLocation(InActionActor, InTargetActor, Grid);
	// Alternatively, use pathfinding, if the grid will have obstacles:
	//auto DummyPath = GS_Pathfinder->FindPath(Path::FNode(InActionActor->GetGridCoordinates()),
	//                                      Path::FNode(InTargetActor->GetGridCoordinates()));
	//if(DummyPath.Num() > 0)
	//{
	//	NextMove = (*DummyPath.begin()).XY;
	//}


	if (NextMove == FIntPoint::ZeroValue)
	{
		UE_LOG(LogSim, Warning, TEXT("[ActorMove] Failed to find a point closer"));
		return;
	}

	// Clear current point on grid, then assign new coordinates to the actor and assign the actor to the new grid point
	Grid.At(InActionActor->GetGridCoordinates()).GameActor = nullptr;
	InActionActor->SetGridCoordinates(NextMove);
	Grid.At(NextMove).GameActor = InActionActor;


	// TODO: fix the lerp first. Then delete the SetActorLocation call.
	InActionActor->MoveActorInterp(GridToGlobal(NextMove), SimulationTimeStep_ms);
	//InActionActor->SetActorLocation(GridToGlobal(NextMove));
}

void AGS_GameModeDefault::HandleActorKilled(AGS_GameActorBase* InTargetActor, AGS_GameActorBase* InInstigatorActor)
{
	UE_LOG(LogSim, Warning, TEXT("[AHandleActorKilled] %s is killed by %s."),
	       *GetNameSafe(InTargetActor), *GetNameSafe(InInstigatorActor));

	InTargetActor->HandleZeroHealth();
	Grid.At(InTargetActor->GetGridCoordinates()).GameActor = nullptr;
	KilledGameActors.Add(InTargetActor);
	--ActorsNumPerTeam.FindOrAdd(InTargetActor->GetTeam());
}

FVector AGS_GameModeDefault::GridToGlobal(const FIntPoint& InCoordinates) const
{
	return FVector{InCoordinates.X * GridCellSize, InCoordinates.Y * GridCellSize, 0.f};
}
