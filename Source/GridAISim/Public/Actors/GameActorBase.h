// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StaticData.h"
#include "GameActorBase.generated.h"

UCLASS()
class GRIDAISIM_API AGS_GameActorBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGS_GameActorBase();

	void SetTeam(ETeam InTeam);
	ETeam GetTeam() const;

	void SetGridCoordinates(const FIntPoint& GridCoordinates);
	FIntPoint GetGridCoordinates() const;

	void SetAttackPower(float InNewAttackPower);
	float GetAttackPower() const;

	void SetAttackRange(int32 InNewAttackRange);
	int32 GetAttackRange() const;

	void SetHealthPoints(float InHealthPoints);
	float GetHealthPoints() const;

	bool IsAlive() const;

	void PlayHit();
	void PlayAttack();
	void HandleZeroHealth();
	void StartDestroy();

	void MoveActorInterp(const FVector& InNewLocation, float InInterpTime_ms);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	int32 GridPointIndex;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Mesh")
	TObjectPtr<UStaticMeshComponent> StaticMeshComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Mesh")
	UMaterialInterface* RedTeamMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Mesh")
	UMaterialInterface* BlueTeamMaterial;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	ETeam Team;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	FIntPoint GridCoordinates;
	

	float AttackPower = 1.f;
	int32 AttackRange = 1;
	float Health = 1.f;
	bool bIsAlive = true;
	
};
