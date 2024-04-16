// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/GameActorBase.h"

// Sets default values
AGS_GameActorBase::AGS_GameActorBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	USceneComponent* SceneComponent = CreateDefaultSubobject<USceneComponent>("Root");
	RootComponent = SceneComponent;
	
	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	StaticMeshComp->SetupAttachment(RootComponent);
}

void AGS_GameActorBase::HandleZeroHealth()
{
	Health = 0.f;
	bIsAlive = false;
}

void AGS_GameActorBase::StartDestroy()
{
	// TODO: spawn some emitter maybe?
	Destroy();
}

void AGS_GameActorBase::MoveActorInterp(const FVector& InNewLocation, float InInterpTime_ms)
{
	// TODO: make it lerp here..
	//SetActorLocation(FMath::Lerp(GetActorLocation(), InNewLocation, InInterpTime_ms));
}

void AGS_GameActorBase::SetTeam(ETeam InTeam)
{
	Team = InTeam;

	if(StaticMeshComp)
	{
		switch(Team)
		{
		case ETeam::BlueTeam:
			StaticMeshComp->SetMaterial(0, BlueTeamMaterial);
			break;
		case ETeam::RedTeam:
			StaticMeshComp->SetMaterial(0, RedTeamMaterial);
			break;
		};
	}
}

ETeam AGS_GameActorBase::GetTeam() const
{
	return Team;
}

FIntPoint AGS_GameActorBase::GetGridCoordinates() const
{
	return GridCoordinates;
}

void AGS_GameActorBase::SetAttackPower(float InNewAttackPower)
{
	AttackPower = InNewAttackPower;
}

float AGS_GameActorBase::GetAttackPower() const
{
	return AttackPower;
}

void AGS_GameActorBase::SetAttackRange(int32 InNewAttackRange)
{
	AttackRange = InNewAttackRange;
}

int32 AGS_GameActorBase::GetAttackRange() const
{
	return AttackRange;
}

void AGS_GameActorBase::SetHealthPoints(float InHealthPoints)
{
	Health = InHealthPoints;
}

float AGS_GameActorBase::GetHealthPoints() const
{
	return Health;
}

bool AGS_GameActorBase::IsAlive() const
{
	return bIsAlive;
}

void AGS_GameActorBase::PlayHit()
{
	// TODO: Make it blink
}

void AGS_GameActorBase::PlayAttack()
{
	// TODO: Make it blink
}

void AGS_GameActorBase::SetGridCoordinates(const FIntPoint& InGridCoordinates)
{
	GridCoordinates = InGridCoordinates;
}
