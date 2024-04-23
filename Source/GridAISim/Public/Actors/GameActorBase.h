// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StaticData.h"
#include "GameModes/GameModeDefault.h"
#include "GameActorBase.generated.h"

UCLASS()
class GRIDAISIM_API AGS_GameActorBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGS_GameActorBase();

	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	
	ETeam GetTeam() const;
	void SetTeam(ETeam InTeam);

	UFUNCTION(BlueprintNativeEvent)
	void OnTeamChanged(ETeam InTeam);

	virtual void OnTeamChanged_Implementation(ETeam InTeam);
	
	void SetGridCoordinates(const FIntPoint& InGridCoordinates);
	FIntPoint GetGridCoordinates() const;
	
	int32 GetGridPointIndex() const;
	void SetGridPointIndex(const int32 InGridPointIndex);
	
	void SetAttackPower(float InNewAttackPower);
	float GetAttackPower() const;

	void SetAttackRange(int32 InNewAttackRange);
	int32 GetAttackRange() const;

	void SetHealthPoints(float InHealthPoints);
	float GetHealthPoints() const;

	/**
	 * A method for the attributes initialization. The idea is to keep the initialization external.
	 * But the idea is not complete yet, so just let the GameMode trigger the method and forget about it.
	 * @param AdditionalHealth An amount of Health that should be added by the rules of the Simulation
	 * @param AdditionalAttackPower An amount of Attack Power that should be added by the rules of the Simulation
	 */
	void InitAttributes(float AdditionalHealth, float AdditionalAttackPower);

	bool IsAlive() const;

	void PlayHit();
	void PlayAttack(const AGS_GameActorBase* InTargetActor);
	void HandleZeroHealth();
	void StartDestroy();

	void MoveActorInterp(const FVector& InNewLocation, float InInterpTime_ms);
	void SetActionDuration(float InGameActionDurationSeconds);
	void Halt();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Game Actor|Visual|Mesh")
	TObjectPtr<UStaticMeshComponent> StaticMeshComp;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Game Actor|Visual|Mesh")
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Game Actor|Visual|Materials")
	UMaterialInterface* RedTeamMaterial;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Game Actor|Visual|Materials")
	UMaterialInterface* BlueTeamMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Game Actor|Visual|Animations")
	TObjectPtr<UAnimSequenceBase> IdleAnimation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Game Actor|Visual|Animations")
	TObjectPtr<UAnimSequenceBase> RunAnimation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Game Actor|Visual|Animations")
	TObjectPtr<UAnimSequenceBase> AttackAnimation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Game Actor|Visual|Animations")
	TObjectPtr<UAnimSequenceBase> HitAnimation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Game Actor|Visual|Animations")
	TObjectPtr<UAnimSequenceBase> DeathAnimation;

	// The property which defines the difference between the Melee and Ranged game actors
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Game Actor|Attributes", meta = (ClampMin = "1", ClampMax = "2"))
	int32 AttackRange = 1;
	
	// The base value of Attack attribute
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Game Actor|Attributes")
	float AttackPowerBase = 1.f;

	// The maximum random value that may be added to Attack attribute
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Game Actor|Attributes")
	float AttackMaxRandomFraction = 1.f;

public:
	float GetAttackMaxRandomFraction() const;
	void SetAttackMaxRandomFraction(const float AttackMaxRandomFraction);
	float GetHealthMaxRandomFraction() const;
	void SetHealthMaxRandomFraction(const float HealthMaxRandomFraction);

protected:
	// The base value of Health attribute
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Game Actor|Attributes")
	float HealthBase = 1.f;

	// The maximum random value that may be added to Health attribute
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Game Actor|Attributes")
	float HealthMaxRandomFraction = 1.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Game Actor|Debug")
	ETeam Team;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Game Actor|Debug")
	FIntPoint GridCoordinates;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Game Actor|Debug")
	int32 GridPointIndex;

protected:
	// TODO: check movement components
	FVector TargetLocation;
	
	
	float CurrentAttackPower = 0.f;
	float CurrentHealth = 0.f;
	bool bIsAlive = true;

	// This is the length of a single game action, so the visuals could be scaled in time
	float GameActionDurationSeconds = 1.f;

	// Set it with a corresponding setter
	EActorState ActorState = EActorState::Idle;

	void SetActorState(EActorState NewState);
	void OnActorStateChanged(EActorState NewState){};
	
};
