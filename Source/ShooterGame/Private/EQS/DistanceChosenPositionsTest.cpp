// Fill out your copyright notice in the Description page of Project Settings.
#include "ShooterGame.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
#include "EnvironmentQuery/Tests/EnvQueryTest_Trace.h"
#include "Bots/ShooterAIController.h"
#include "Public/Others/HelperMethods.h"
#include "Public/EQS/DistanceChosenPositionsTest.h"

UDistanceChosenPositionsTest::UDistanceChosenPositionsTest(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Cost = EEnvTestCost::Low;
	ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();
}

void UDistanceChosenPositionsTest::RunTest(FEnvQueryInstance& QueryInstance) const
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

	const TMap<FString, FVector> * AlreadyChosenAttackPositions = AIController->GetAI_fAttackLocations();

	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It) {
		const FVector ItemLocation = GetItemLocation(QueryInstance, *It);
	
		float MinDistance = 10000;
		for (auto& Elem : *AlreadyChosenAttackPositions) {
			const FString BotName = *Elem.Key;
			const FVector BotAttackPosition = Elem.Value;

			if (BotName != Pawn->GetName()) {
				// Dont compare with MY old position
				const float Distance = FVector::Dist(BotAttackPosition, ItemLocation);
				MinDistance = (Distance > MinDistance) ? MinDistance : Distance;
			}
		}
		It.SetScore(TestPurpose, FilterType, MinDistance, MinThresholdValue, MaxThresholdValue);
	}
}
