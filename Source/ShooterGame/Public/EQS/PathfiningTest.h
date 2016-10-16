// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EnvironmentQuery/EnvQueryTest.h"
#include "EnvironmentQuery/Tests/EnvQueryTest_Pathfinding.h"
#include "PathfiningTest.generated.h"

UCLASS()
class SHOOTERGAME_API UPathfiningTest : public UEnvQueryTest
{
	GENERATED_UCLASS_BODY()
		/** testing mode */
		UPROPERTY(EditDefaultsOnly, Category = Pathfinding)
		TEnumAsByte<EEnvTestPathfinding::Type> TestMode;

	/** context: other end of pathfinding test */
	UPROPERTY(EditDefaultsOnly, Category = Pathfinding)
		TSubclassOf<UEnvQueryContext> Context;

	/** pathfinding direction */
	UPROPERTY(EditDefaultsOnly, Category = Pathfinding)
		FAIDataProviderBoolValue PathFromContext;

	/** if set, items with failed path will be invalidated (PathCost, PathLength) */
	UPROPERTY(EditDefaultsOnly, Category = Pathfinding, AdvancedDisplay)
		FAIDataProviderBoolValue SkipUnreachable;

	/** navigation filter to use in pathfinding */
	UPROPERTY(EditDefaultsOnly, Category = Pathfinding)
		TSubclassOf<UNavigationQueryFilter> FilterClass;

	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;

	virtual FText GetDescriptionTitle() const override;
	virtual FText GetDescriptionDetails() const override;

#if WITH_EDITOR
	/** update test properties after changing mode */
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void PostLoad() override;

protected:

	DECLARE_DELEGATE_RetVal_SixParams(bool, FTestPathSignature, const FVector&, const FVector&, EPathFindingMode::Type, const ANavigationData&, UNavigationSystem&, const UObject*);
	DECLARE_DELEGATE_RetVal_SixParams(float, FFindPathSignature, const FVector&, const FVector&, EPathFindingMode::Type, const ANavigationData&, UNavigationSystem&, const UObject*);

	bool TestPathFrom(const FVector& ItemPos, const FVector& ContextPos, EPathFindingMode::Type Mode, const ANavigationData& NavData, UNavigationSystem& NavSys, const UObject* PathOwner) const;
	bool TestPathTo(const FVector& ItemPos, const FVector& ContextPos, EPathFindingMode::Type Mode, const ANavigationData& NavData, UNavigationSystem& NavSys, const UObject* PathOwner) const;
	float FindPathCostFrom(const FVector& ItemPos, const FVector& ContextPos, EPathFindingMode::Type Mode, const ANavigationData& NavData, UNavigationSystem& NavSys, const UObject* PathOwner) const;
	float FindPathCostTo(const FVector& ItemPos, const FVector& ContextPos, EPathFindingMode::Type Mode, const ANavigationData& NavData, UNavigationSystem& NavSys, const UObject* PathOwner) const;
	float FindPathLengthFrom(const FVector& ItemPos, const FVector& ContextPos, EPathFindingMode::Type Mode, const ANavigationData& NavData, UNavigationSystem& NavSys, const UObject* PathOwner) const;
	float FindPathLengthTo(const FVector& ItemPos, const FVector& ContextPos, EPathFindingMode::Type Mode, const ANavigationData& NavData, UNavigationSystem& NavSys, const UObject* PathOwner) const;

	ANavigationData* FindNavigationData(UNavigationSystem& NavSys, UObject* Owner) const;
};
