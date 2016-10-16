// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
#include "EnvironmentQuery/Tests/EnvQueryTest_Trace.h"
#include "Bots/ShooterAIController.h"
#include "Bots/ShooterBot.h"
#include "Public/Others/HelperMethods.h"
#include "Public/EQS/MaxSpreadSearchPositionsTest.h"



UMaxSpreadSearchPositionsTest::UMaxSpreadSearchPositionsTest(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Cost = EEnvTestCost::Low;
	ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();
}

void UMaxSpreadSearchPositionsTest::RunTest(FEnvQueryInstance& QueryInstance) const
{
	//UE_LOG(LogTemp, Log, TEXT("F:RunTest"));

	UObject* QueryOwner = QueryInstance.Owner.Get();
	if (QueryOwner == nullptr)
	{
		return;
	}

	FloatValueMin.BindData(QueryOwner, QueryInstance.QueryID);
	float MinThresholdValue = FloatValueMin.GetValue();

	FloatValueMax.BindData(QueryOwner, QueryInstance.QueryID);
	float MaxThresholdValue = FloatValueMax.GetValue();


	// Get all covers annotations within radius
	UWorld * World = GEngine->GetWorldFromContextObject(QueryOwner);

	APawn * Pawn = Cast<APawn>(QueryOwner);

	if (!Pawn) {
		return;
	}

	AShooterAIController * AIController = Cast<AShooterAIController>(Pawn->GetController());

	if (!AIController) {
		return;
	}

	const TMap<FString, FVector> * AlreadyChosenSearchPositions = AIController->GetAI_SearchLocations();

	if (!AlreadyChosenSearchPositions) {
		return;
	}


	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It) {
		const FVector ItemLocation = GetItemLocation(QueryInstance, *It);

		float MinDistance = 9999;
		float DistanceFromMyOldPos = 0;
		float SumDistanceOtherBots = 0;
		int Counter = 0;
		for (auto& Elem : *AlreadyChosenSearchPositions) {
			const FString BotName = *Elem.Key;
			const FVector BotSearchPosition = Elem.Value;

			if (BotName != Pawn->GetName()) {
				// Check distance to others bot positions
				const float Distance = FVector::Dist(BotSearchPosition, ItemLocation);
				MinDistance = (Distance < MinDistance) ? Distance : MinDistance;
				SumDistanceOtherBots += Distance;
				++Counter;
			}
		}
		const float Score = SumDistanceOtherBots / Counter;
		It.SetScore(TestPurpose, FilterType, Score, MinThresholdValue, MaxThresholdValue);
	}
}
