// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "SearchLocations.generated.h"

UCLASS()
class SHOOTERGAME_API ASearchLocations : public AActor
{
	GENERATED_BODY()
	
public:
	TMap<FString, FVector> * SearchMap;
public:	
	// Sets default values for this actor's properties
	ASearchLocations();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	
	
};
