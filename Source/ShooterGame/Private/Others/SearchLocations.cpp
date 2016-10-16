// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "Public/Others/SearchLocations.h"


// Sets default values
ASearchLocations::ASearchLocations()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASearchLocations::BeginPlay()
{
	Super::BeginPlay();
	SearchMap = new TMap<FString, FVector>();
}

// Called every frame
void ASearchLocations::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

