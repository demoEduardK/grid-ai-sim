// Fill out your copyright notice in the Description page of Project Settings.


#include "Grid/GridTestActor.h"

// Sets default values
AGS_GridTestActor::AGS_GridTestActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGS_GridTestActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGS_GridTestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

