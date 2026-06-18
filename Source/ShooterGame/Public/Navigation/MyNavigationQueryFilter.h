// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NavFilters/NavigationQueryFilter.h"
#include "MyNavigationQueryFilter.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERGAME_API UMyNavigationQueryFilter : public UNavigationQueryFilter
{
	GENERATED_UCLASS_BODY()
		                        //const ANavigationData& NavData, const UObject* Querier, FNavigationQueryFilter& Filter
	virtual void InitializeFilter(const ANavigationData& NavData, const UObject* Querier, FNavigationQueryFilter& Filter) const override;
};
