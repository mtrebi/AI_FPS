// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "Public/Others/AttackPositions.h"


// Sets default values
AAttackPositions::AAttackPositions()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AAttackPositions::BeginPlay()
{
	Super::BeginPlay();
	AttackMap = new TMap<FString, FVector>();
}

// Called every frame
void AAttackPositions::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

