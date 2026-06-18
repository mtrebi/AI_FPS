// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EnvironmentQuery/EnvQueryTest.h"
#include "Navigation/MyRecastNavMesh.h"
#include "Others/HelperMethods.h"
#include "PlayerVisibilityTest.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERGAME_API UPlayerVisibilityTest : public UEnvQueryTest
{
	GENERATED_UCLASS_BODY()
	UPROPERTY(EditDefaultsOnly, Category = Trace)
	bool PanoramicView = false;

	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
private:
	TArray<Triangle> GetPlayerVisibility(UWorld * World) const;
	FVector GetPlayerForwardVectorFromAI(UWorld * World) const;
};
