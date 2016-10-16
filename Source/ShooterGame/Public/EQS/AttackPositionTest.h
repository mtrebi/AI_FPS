// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EnvironmentQuery/EnvQueryTest.h"
#include "AttackPositionTest.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERGAME_API UAttackPositionTest : public UEnvQueryTest
{
	GENERATED_UCLASS_BODY()
	UPROPERTY(EditDefaultsOnly, Category = Trace)
	TSubclassOf<UEnvQueryContext> Context;

	UPROPERTY(EditDefaultsOnly, Category = Trace)
	float MaxRadius = 2000.0f;

	UPROPERTY(EditDefaultsOnly, Category = Trace)
	float DiscardDistanceMax = 150.0f;
	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
	virtual void PostLoad() override;

private:
	TArray<FVector> GetLocationOfCoverAnnotationsWithinRadius(UWorld * World, const FVector ContextLocation) const;

	
	
	
};
