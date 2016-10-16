// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EnvironmentQuery/EnvQueryTest.h"
#include "KeepFirstPositionTest.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERGAME_API UKeepFirstPositionTest : public UEnvQueryTest
{
	GENERATED_UCLASS_BODY()
		UPROPERTY(EditDefaultsOnly, Category = Trace)
		TSubclassOf<UEnvQueryContext> Context;

		UPROPERTY(EditDefaultsOnly, Category = Trace)
		float AngleDifference = 5.0f;
	
	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
};
