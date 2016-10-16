// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
#include "Public/Others/HelperMethods.h"
#include "Public/EQS/PlayerDistanceTest.h"

UPlayerDistanceTest::UPlayerDistanceTest(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Cost = EEnvTestCost::Low;
	ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();
}

void UPlayerDistanceTest::RunTest(FEnvQueryInstance& QueryInstance) const {
	//UE_LOG(LogTemp, Log, TEXT("F:RunTest"));

	UObject* QueryOwner = QueryInstance.Owner.Get();
	BoolValue.BindData(QueryOwner, QueryInstance.QueryID);

	bool bWantsHit = BoolValue.GetValue();

	if (QueryOwner == nullptr)
	{
		return;
	}

	FloatValueMin.BindData(QueryOwner, QueryInstance.QueryID);
	float MinThresholdValue = FloatValueMin.GetValue();

	FloatValueMax.BindData(QueryOwner, QueryInstance.QueryID);
	float MaxThresholdValue = FloatValueMax.GetValue();


	UWorld * World = GEngine->GetWorldFromContextObject(QueryOwner);
	const FVector PlayerPosition = HelperMethods::GetPlayerPositionFromAI(World);
	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		const FVector ItemLocation = GetItemLocation(QueryInstance, *It);
		const float Distance = FVector::Dist(PlayerPosition, ItemLocation);

		It.SetScore(TestPurpose, FilterType, Distance, MinThresholdValue, MaxThresholdValue);
	}
}


