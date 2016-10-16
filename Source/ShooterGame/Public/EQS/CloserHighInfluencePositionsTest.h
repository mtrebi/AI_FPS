// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EnvironmentQuery/EnvQueryTest.h"
#include "CloserHighInfluencePositionsTest.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERGAME_API UCloserHighInfluencePositionsTest : public UEnvQueryTest
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Score")
	bool UseAverage = false;

	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;

};
