// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
#include "Public/Bots/ShooterBot.h"
#include "Public/Bots/ShooterAIController.h"
#include "Public/EQS/KeepFirstPositionTest.h"



UKeepFirstPositionTest::UKeepFirstPositionTest(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Cost = EEnvTestCost::Low;
	ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();
	Context = UEnvQueryContext_Querier::StaticClass();
	SetWorkOnFloatValues(false);
}

void UKeepFirstPositionTest::RunTest(FEnvQueryInstance& QueryInstance) const
{
	//UE_LOG(LogTemp, Log, TEXT("F:RunTest"));

	UObject* QueryOwner = QueryInstance.Owner.Get();
	if (QueryOwner == nullptr)
	{
		return;
	}

	bool bWantsHit = BoolValue.GetValue();

	// Get all covers annotations within radius
	UWorld * World = GEngine->GetWorldFromContextObject(QueryOwner);

	// @ todo many bots
	TArray<FVector> ContextLocations;
	if (!QueryInstance.PrepareContext(Context, ContextLocations))
	{
		return;
	}

	//const FVector BotPosition = FVector(ContextLocations[0].X, ContextLocations[0].Y, HelperMethods::EYES_POS_Z);
	const FVector BotPosition = FVector(-1490, 1460, 150);

	TArray<FVector> FirstLocations;

	for (int Index = 0; Index < QueryInstance.Items.Num(); ++Index) {
		FirstLocations.Add(GetItemLocation(QueryInstance, Index));
	}

	for (int Index1 = 0; Index1 < QueryInstance.Items.Num(); ++Index1) {
		const FVector ItemLocation1 = GetItemLocation(QueryInstance, Index1);
		const FVector UpLocation1 = FVector(ItemLocation1.X, ItemLocation1.Y, 150);

		FVector Item1ToBot = ItemLocation1 - BotPosition;
		Item1ToBot.Normalize();

		for (int Index2 = Index1 + 1; Index2 < QueryInstance.Items.Num() && FirstLocations.Contains(ItemLocation1); ++Index2) {
			const FVector ItemLocation2 = GetItemLocation(QueryInstance, Index2);
			const FVector UpLocation2 = FVector(ItemLocation2.X, ItemLocation2.Y, 150);

			FVector Item2ToBot = ItemLocation2 - BotPosition;
			Item2ToBot.Normalize();
			const float Angle = FMath::RadiansToDegrees(acosf(Item1ToBot.CosineAngle2D(Item2ToBot)));

			if (Angle <= AngleDifference) {
				FVector PlayerPos = HelperMethods::GetPlayerPositionFromAI(World);
				if (FVector::Dist(PlayerPos, ItemLocation1) < FVector::Dist(PlayerPos, ItemLocation1)) {
					FirstLocations.Remove(ItemLocation2);
				}
				else {
					FirstLocations.Remove(ItemLocation1);
					break;
				}
			}
		}
	}

	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		
		const FVector ItemLocation = GetItemLocation(QueryInstance, *It);
		It.SetScore(TestPurpose, FilterType, FirstLocations.Contains(ItemLocation), bWantsHit);
	}
}

