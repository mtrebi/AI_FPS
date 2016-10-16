// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EnvironmentQuery/EnvQueryTest.h"
#include "PlayerDistanceTest.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERGAME_API UPlayerDistanceTest : public UEnvQueryTest
{
	GENERATED_UCLASS_BODY()
	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;	
};
