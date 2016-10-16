// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "CoverBaseClass.h"


// Sets default values
ACoverBaseClass::ACoverBaseClass()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACoverBaseClass::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACoverBaseClass::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

