// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterGame.h"
#include "AI/Navigation/NavigationSystem.h"
#include "AI/Navigation/NavAgentInterface.h"
#include "Public/Navigation/MyNavigationQueryFilter.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
//#include "AIModulePrivate.h"
#include "Public/EQS/PathfiningTest.h"

#define LOCTEXT_NAMESPACE "EnvQueryGenerator"

UPathfiningTest::UPathfiningTest(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Context = UEnvQueryContext_Querier::StaticClass();
	Cost = EEnvTestCost::High;
	ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();
	TestMode = EEnvTestPathfinding::PathExist;
	PathFromContext.DefaultValue = true;
	SkipUnreachable.DefaultValue = true;
	FloatValueMin.DefaultValue = 1000.0f;
	FloatValueMax.DefaultValue = 1000.0f;

	SetWorkOnFloatValues(TestMode != EEnvTestPathfinding::PathExist);
}

void UPathfiningTest::RunTest(FEnvQueryInstance& QueryInstance) const
{
	UObject* QueryOwner = QueryInstance.Owner.Get();
	BoolValue.BindData(QueryOwner, QueryInstance.QueryID);
	PathFromContext.BindData(QueryOwner, QueryInstance.QueryID);
	SkipUnreachable.BindData(QueryOwner, QueryInstance.QueryID);
	FloatValueMin.BindData(QueryOwner, QueryInstance.QueryID);
	FloatValueMax.BindData(QueryOwner, QueryInstance.QueryID);

	bool bWantsPath = BoolValue.GetValue();
	bool bPathToItem = PathFromContext.GetValue();
	bool bDiscardFailed = SkipUnreachable.GetValue();
	float MinThresholdValue = FloatValueMin.GetValue();
	float MaxThresholdValue = FloatValueMax.GetValue();

	UNavigationSystem* NavSys = QueryInstance.World->GetNavigationSystem();
	if (NavSys == nullptr)
	{
		return;
	}




	ANavigationData* DefaultNavData = FindNavigationData(*NavSys, QueryOwner);
	if (DefaultNavData == nullptr)
	{
		return;
	}

	AMyRecastNavMesh* NavData = Cast<AMyRecastNavMesh>(DefaultNavData);

	if (NavData == nullptr)
	{
		return;
	}
	FRecastQueryFilter_Example* MyFRecastQueryFilter = NavData->GetCustomFilter();
	//AMyRecastNavMesh* MyNavMesh = Cast<AMyRecastNavMesh>(DefaultNavData);



	TArray<FVector> ContextLocations;
	if (!QueryInstance.PrepareContext(Context, ContextLocations))
	{
		return;
	}

	EPathFindingMode::Type PFMode(EPathFindingMode::Regular);

	if (GetWorkOnFloatValues())
	{
		FFindPathSignature FindPathFunc;
		FindPathFunc.BindUObject(this, TestMode == EEnvTestPathfinding::PathLength ?
			(bPathToItem ? &UPathfiningTest::FindPathLengthTo : &UPathfiningTest::FindPathLengthFrom) :
			(bPathToItem ? &UPathfiningTest::FindPathCostTo : &UPathfiningTest::FindPathCostFrom));

		NavData->BeginBatchQuery();
		for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
		{
			const FVector ItemLocation = GetItemLocation(QueryInstance, It.GetIndex());
			for (int32 ContextIndex = 0; ContextIndex < ContextLocations.Num(); ContextIndex++)
			{
				const float PathValue = FindPathFunc.Execute(ItemLocation, ContextLocations[ContextIndex], PFMode, *NavData, *NavSys, QueryOwner);
				It.SetScore(TestPurpose, FilterType, PathValue, MinThresholdValue, MaxThresholdValue);

				if (bDiscardFailed && PathValue >= BIG_NUMBER)
				{
					It.ForceItemState(EEnvItemStatus::Failed);
				}
			}
		}
		NavData->FinishBatchQuery();
	}
	else
	{
		NavData->BeginBatchQuery();
		if (bPathToItem)
		{
			for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
			{
				const FVector ItemLocation = GetItemLocation(QueryInstance, It.GetIndex());
				for (int32 ContextIndex = 0; ContextIndex < ContextLocations.Num(); ContextIndex++)
				{
					const bool bFoundPath = TestPathTo(ItemLocation, ContextLocations[ContextIndex], PFMode, *NavData, *NavSys, QueryOwner);
					It.SetScore(TestPurpose, FilterType, bFoundPath, bWantsPath);
				}
			}
		}
		else
		{
			for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
			{
				const FVector ItemLocation = GetItemLocation(QueryInstance, It.GetIndex());
				for (int32 ContextIndex = 0; ContextIndex < ContextLocations.Num(); ContextIndex++)
				{
					const bool bFoundPath = TestPathFrom(ItemLocation, ContextLocations[ContextIndex], PFMode, *NavData, *NavSys, QueryOwner);
					It.SetScore(TestPurpose, FilterType, bFoundPath, bWantsPath);
				}
			}
		}
		NavData->FinishBatchQuery();
	}
}

FText UPathfiningTest::GetDescriptionTitle() const
{
	FString ModeDesc[] = { TEXT("PathExist"), TEXT("PathCost"), TEXT("PathLength") };

	FString DirectionDesc = PathFromContext.IsDynamic() ?
		FString::Printf(TEXT("%s, direction: %s"), *UEnvQueryTypes::DescribeContext(Context).ToString(), *PathFromContext.ToString()) :
		FString::Printf(TEXT("%s %s"), PathFromContext.DefaultValue ? TEXT("from") : TEXT("to"), *UEnvQueryTypes::DescribeContext(Context).ToString());

	return FText::FromString(FString::Printf(TEXT("%s: %s"), *ModeDesc[TestMode], *DirectionDesc));
}

FText UPathfiningTest::GetDescriptionDetails() const
{
	FText DiscardDesc = LOCTEXT("DiscardUnreachable", "discard unreachable");
	FText Desc2;
	if (SkipUnreachable.IsDynamic())
	{
		Desc2 = FText::Format(FText::FromString("{0}: {1}"), DiscardDesc, FText::FromString(SkipUnreachable.ToString()));
	}
	else if (SkipUnreachable.DefaultValue)
	{
		Desc2 = DiscardDesc;
	}

	FText TestParamDesc = GetWorkOnFloatValues() ? DescribeFloatTestParams() : DescribeBoolTestParams("existing path");
	if (!Desc2.IsEmpty())
	{
		return FText::Format(FText::FromString("{0}\n{1}"), Desc2, TestParamDesc);
	}

	return TestParamDesc;
}

#if WITH_EDITOR
void UPathfiningTest::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UPathfiningTest, TestMode))
	{
		SetWorkOnFloatValues(TestMode != EEnvTestPathfinding::PathExist);
	}
}
#endif

void UPathfiningTest::PostLoad()
{
	Super::PostLoad();

	SetWorkOnFloatValues(TestMode != EEnvTestPathfinding::PathExist);
}

bool UPathfiningTest::TestPathFrom(const FVector& ItemPos, const FVector& ContextPos, EPathFindingMode::Type Mode, const ANavigationData& NavData, UNavigationSystem& NavSys, const UObject* PathOwner) const
{
	FPathFindingQuery Query(PathOwner, NavData, ItemPos, ContextPos, UNavigationQueryFilter::GetQueryFilter(NavData, FilterClass));
	Query.SetAllowPartialPaths(false);

	const bool bPathExists = NavSys.TestPathSync(Query, Mode);
	return bPathExists;
}

bool UPathfiningTest::TestPathTo(const FVector& ItemPos, const FVector& ContextPos, EPathFindingMode::Type Mode, const ANavigationData& NavData, UNavigationSystem& NavSys, const UObject* PathOwner) const
{
	const AMyRecastNavMesh* MyNavData = Cast<AMyRecastNavMesh>(&NavData);
	const FRecastQueryFilter_Example* MyFRecastQueryFilter = MyNavData->GetCustomFilter();
	
	FPathFindingQuery Query(PathOwner, NavData, ContextPos, ItemPos, UNavigationQueryFilter::GetQueryFilter(NavData, FilterClass));
	//FPathFindingQuery Query(PathOwner, NavData, ContextPos, ItemPos, MyFRecastQueryFilter);
	Query.SetAllowPartialPaths(false);

	const bool bPathExists = NavSys.TestPathSync(Query, Mode);
	return bPathExists;
}

float UPathfiningTest::FindPathCostFrom(const FVector& ItemPos, const FVector& ContextPos, EPathFindingMode::Type Mode, const ANavigationData& NavData, UNavigationSystem& NavSys, const UObject* PathOwner) const
{
	FPathFindingQuery Query(PathOwner, NavData, ItemPos, ContextPos, UNavigationQueryFilter::GetQueryFilter(NavData, FilterClass));
	Query.SetAllowPartialPaths(false);

	FPathFindingResult Result = NavSys.FindPathSync(Query, Mode);
	return (Result.IsSuccessful()) ? Result.Path->GetCost() : BIG_NUMBER;
}

float UPathfiningTest::FindPathCostTo(const FVector& ItemPos, const FVector& ContextPos, EPathFindingMode::Type Mode, const ANavigationData& NavData, UNavigationSystem& NavSys, const UObject* PathOwner) const
{
	TSharedPtr<const FNavigationQueryFilter> QueryFilter = UMyNavigationQueryFilter::GetQueryFilter(NavData, FilterClass);
	FPathFindingQuery Query(PathOwner, NavData, ContextPos, ItemPos, QueryFilter);
	Query.SetAllowPartialPaths(false);

	FPathFindingResult Result = NavSys.FindPathSync(Query, Mode);
	return (Result.IsSuccessful()) ? Result.Path->GetCost() : BIG_NUMBER;
}

float UPathfiningTest::FindPathLengthFrom(const FVector& ItemPos, const FVector& ContextPos, EPathFindingMode::Type Mode, const ANavigationData& NavData, UNavigationSystem& NavSys, const UObject* PathOwner) const
{
	FPathFindingQuery Query(PathOwner, NavData, ItemPos, ContextPos, UNavigationQueryFilter::GetQueryFilter(NavData, FilterClass));
	Query.SetAllowPartialPaths(false);

	FPathFindingResult Result = NavSys.FindPathSync(Query, Mode);
	return (Result.IsSuccessful()) ? Result.Path->GetLength() : BIG_NUMBER;
}

float UPathfiningTest::FindPathLengthTo(const FVector& ItemPos, const FVector& ContextPos, EPathFindingMode::Type Mode, const ANavigationData& NavData, UNavigationSystem& NavSys, const UObject* PathOwner) const
{
	FPathFindingQuery Query(PathOwner, NavData, ContextPos, ItemPos, UNavigationQueryFilter::GetQueryFilter(NavData, FilterClass));
	Query.SetAllowPartialPaths(false);

	FPathFindingResult Result = NavSys.FindPathSync(Query, Mode);
	return (Result.IsSuccessful()) ? Result.Path->GetLength() : BIG_NUMBER;
}

ANavigationData* UPathfiningTest::FindNavigationData(UNavigationSystem& NavSys, UObject* Owner) const
{
	INavAgentInterface* NavAgent = Cast<INavAgentInterface>(Owner);
	if (NavAgent)
	{
		return NavSys.GetNavDataForProps(NavAgent->GetNavAgentPropertiesRef());
	}

	return NavSys.GetMainNavData(FNavigationSystem::DontCreate);
}

#undef LOCTEXT_NAMESPACE


