// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/GameActorBase.h"

#include "GridAISim/GridAISim.h"


// Sets default values
AGS_GameActorBase::AGS_GameActorBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* SceneComponent = CreateDefaultSubobject<USceneComponent>("Root");
	RootComponent = SceneComponent;

	SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	SkeletalMeshComp->SetupAttachment(RootComponent);
}

void AGS_GameActorBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (ActorState == EActorState::Moving)
	{
		if (GameActionDurationSeconds < SMALL_NUMBER)
		{
			UE_LOG(LogSim, Error, TEXT("[Tick] Zero division prevented."))
			return;
		}

		// TODO: grab this parameter from the GameMode
		const float SegmentSize = 100.f;
		SetActorLocation(FMath::VInterpConstantTo(GetActorLocation(), TargetLocation, DeltaSeconds,
		                                          SegmentSize * (1 / GameActionDurationSeconds)));
	}
}

void AGS_GameActorBase::BeginPlay()
{
	Super::BeginPlay();
	SetActorState(EActorState::Idle);
}

void AGS_GameActorBase::HandleZeroHealth()
{
	CurrentHealth = 0.f;
	bIsAlive = false;
	SetActorState(EActorState::Dead);

	if (GameActionDurationSeconds < SMALL_NUMBER)
	{
		UE_LOG(LogSim, Error, TEXT("[StartDestroy] Zero division prevented."))
		Destroy();
		return;
	}

	FTimerHandle UselessHandle;
	GetWorld()->GetTimerManager().SetTimer(UselessHandle, this, &ThisClass::StartDestroy
	                                       , GameActionDurationSeconds, false);
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

	TargetLocation = InNewLocation;
	SetActorRotation((TargetLocation - GetActorLocation()).Rotation());
	//ActorState = EActorState::Moving;
	SetActorState(EActorState::Moving);
}

void AGS_GameActorBase::SetActionDuration(float InGameActionDurationSeconds)
{
	GameActionDurationSeconds = InGameActionDurationSeconds;
}

void AGS_GameActorBase::Halt()
{
	SetActorState(EActorState::Idle);
}

float AGS_GameActorBase::GetAttackMaxRandomFraction() const
{
	return AttackMaxRandomFraction;
}

void AGS_GameActorBase::SetAttackMaxRandomFraction(float InAttackMaxRandomFraction)
{
	AttackMaxRandomFraction = InAttackMaxRandomFraction;
}

float AGS_GameActorBase::GetHealthMaxRandomFraction() const
{
	return HealthMaxRandomFraction;
}

void AGS_GameActorBase::SetHealthMaxRandomFraction(float InHealthMaxRandomFraction)
{
	HealthMaxRandomFraction = InHealthMaxRandomFraction;
}

void AGS_GameActorBase::SetActorState(EActorState NewState)
{
	ActorState = NewState;

	// PlayAnimation stage (move it to a separate method, or callback)

	TObjectPtr<UAnimSequenceBase> SequenceToPlay = nullptr;
	bool bLoopAnimation = false;
	switch (ActorState)
	{
	case EActorState::Idle:
		SequenceToPlay = IdleAnimation;
		bLoopAnimation = true;
		break;
	case EActorState::Moving:
		SequenceToPlay = RunAnimation;
		break;
	case EActorState::Attacking:
		SequenceToPlay = AttackAnimation;
		break;
	case EActorState::Dead:
		SequenceToPlay = DeathAnimation;
		break;
	default:
		break;
	};

	if (SequenceToPlay != nullptr)
	{
		if (GameActionDurationSeconds < SMALL_NUMBER)
		{
			UE_LOG(LogSim, Error, TEXT("[SetActorState] Zero division prevented."))
			return;
		}
		const float AnimDuration = SequenceToPlay->GetPlayLength();
		SequenceToPlay->RateScale = AnimDuration / GameActionDurationSeconds;
		SkeletalMeshComp->Stop();
		SkeletalMeshComp->PlayAnimation(SequenceToPlay, bLoopAnimation);
	}
}

void AGS_GameActorBase::SetTeam(ETeam InTeam)
{
	Team = InTeam;

	/*if(StaticMeshComp)
	{
		switch(Team)
		{
		case ETeam::BlueTeam:
			StaticMeshComp->SetMaterial(0, BlueTeamMaterial);
			SkeletalMeshComp->SetMaterial(2, BlueTeamMaterial);
			break;
		case ETeam::RedTeam:
			StaticMeshComp->SetMaterial(0, RedTeamMaterial);
			SkeletalMeshComp->SetMaterial(2, BlueTeamMaterial);
			break;
		};
	}*/

	OnTeamChanged(InTeam);
}

ETeam AGS_GameActorBase::GetTeam() const
{
	return Team;
}

FIntPoint AGS_GameActorBase::GetGridCoordinates() const
{
	return GridCoordinates;
}

void AGS_GameActorBase::OnTeamChanged_Implementation(ETeam InTeam)
{
}

void AGS_GameActorBase::SetGridCoordinates(const FIntPoint& InGridCoordinates)
{
	GridCoordinates = InGridCoordinates;
}

int32 AGS_GameActorBase::GetGridPointIndex() const
{
	return GridPointIndex;
}

void AGS_GameActorBase::SetGridPointIndex(const int32 InGridPointIndex)
{
	GridPointIndex = InGridPointIndex;
}

void AGS_GameActorBase::SetAttackPower(float InNewAttackPower)
{
	CurrentAttackPower = AttackPowerBase + InNewAttackPower;
}

float AGS_GameActorBase::GetAttackPower() const
{
	return CurrentAttackPower;
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
	CurrentHealth = InHealthPoints;
}

float AGS_GameActorBase::GetHealthPoints() const
{
	return CurrentHealth;
}

void AGS_GameActorBase::InitAttributes(float AdditionalHealth, float AdditionalAttackPower)
{
	CurrentAttackPower = AttackPowerBase + FMath::RandRange(0.f, AttackMaxRandomFraction);
	CurrentHealth = HealthBase + FMath::RandRange(0.f, HealthMaxRandomFraction);
}

bool AGS_GameActorBase::IsAlive() const
{
	return bIsAlive;
}

void AGS_GameActorBase::PlayHit()
{
	// TODO: Make it blink
}

void AGS_GameActorBase::PlayAttack(const AGS_GameActorBase* InTargetActor)
{
	if (InTargetActor)
	{
		SetActorRotation((InTargetActor->GetActorLocation() - GetActorLocation()).Rotation());
	}
	SetActorState(EActorState::Attacking);
}
