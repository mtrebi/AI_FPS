// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EnvironmentQuery/EnvQueryTest.h"
#include "NearBehindObstacleTest.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERGAME_API UNearBehindObstacleTest : public UEnvQueryTest
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditDefaultsOnly, Category = Trace)
	bool TraceToPlayer = true;
	UPROPERTY(EditDefaultsOnly, Category = Trace)
	float MaxRadius = 2000.0f;

	UPROPERTY(EditDefaultsOnly, Category = Trace)
	float DiscardDistanceMax = 200.0f;
	UPROPERTY(EditDefaultsOnly, Category = Trace)
	float DiscardDistanceMin = 175.0f;

	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;

private:
	FVector GetPlayerPositionFromAI(UWorld * World) const;
};
