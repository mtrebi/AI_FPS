// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ShooterBot.generated.h"

UCLASS()
class AShooterBot : public AShooterCharacter
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditAnywhere, Category=Behavior)
	class UBehaviorTree* BotBehavior;

	virtual bool IsFirstPerson() const override;

	virtual void FaceRotation(FRotator NewRotation, float DeltaTime = 0.f) override;

public:
	//UAISense_Hearing::ReportNoiseEvent(this, GetActorLocation(), 10.0, this);
	//UPROPERTY(EditAnywhere, Category = "AI Behavior Tree")
	//TArray<FVector> PatrolPoints;


	UPROPERTY(EditAnywhere, Category = "AI Behavior Tree")
	APawn* Player;
	UPROPERTY(EditAnywhere, Category = "AI Behavior Tree")
	bool PlayerCanSeeMe = false;
	UPROPERTY(EditAnywhere, Category = "AI Behavior Tree")
	bool PlayerIsVisible = false;
	UPROPERTY(EditAnywhere, Category = "AI Behavior Tree")
	FVector PlayerLastLocation = FVector();

	UPROPERTY(EditAnywhere, Category = "AI Behavior Tree")
	FVector CurrentLocation = FVector();
	UPROPERTY(EditAnywhere, Category = "AI Behavior Tree")
	bool IAmReloading = false;
	UPROPERTY(EditAnywhere, Category = "AI Behavior Tree")
	bool NeedReloadNow = false;
	UPROPERTY(EditAnywhere, Category = "AI Behavior Tree")
	bool NeedReloadSoon = false;

	UPROPERTY(EditAnywhere, Category = "AI Behavior Tree")
	FVector NextLocation;
	UPROPERTY(EditAnywhere, Category = "AI Behavior Tree")
	bool NextLocationIsSafe = false;
	//UPROPERTY(EditAnywhere, Category = "AI Behavior Tree")
	//TArray<FVector> PatrolPoints;

};