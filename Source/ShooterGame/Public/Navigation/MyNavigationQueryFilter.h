// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AI/Navigation/NavFilters/NavigationQueryFilter.h"
#include "MyNavigationQueryFilter.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERGAME_API UMyNavigationQueryFilter : public UNavigationQueryFilter
{
	GENERATED_UCLASS_BODY()
	
	virtual void InitializeFilter(const ANavigationData& NavData, FNavigationQueryFilter& Filter) const override;
};
