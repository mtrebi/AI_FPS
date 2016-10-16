// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
#include "EnvironmentQuery/Tests/EnvQueryTest_Trace.h"
#include "Bots/ShooterAIController.h"
#include "Public/Others/HelperMethods.h"
#include "Public/EQS/MinDotProductChosenPositions.h"

UMinDotProductChosenPositions::UMinDotProductChosenPositions(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Cost = EEnvTestCost::Low;
	ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();
}

void UMinDotProductChosenPositions::RunTest(FEnvQueryInstance& QueryInstance) const
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
	const FVector PlayerPosition = HelperMethods::GetPlayerPositionFromAI(World);

	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It) {
		const FVector ItemLocation = GetItemLocation(QueryInstance, *It);
		FVector ItemToPlayer = PlayerPosition - ItemLocation;

		ItemToPlayer.Normalize();

		float MinAngle = 360;
		for (auto& Elem : *AlreadyChosenAttackPositions) {
			const FString BotName = *Elem.Key;
			const FVector BotAttackPosition = Elem.Value;
			//UE_LOG(LogTemp, Warning, TEXT("EQS Bot in the map %s"), *BotName);

			if (BotName != Pawn->GetName()) {
				// Dont compare with MY old position
				FVector AttackToPlayer = PlayerPosition - BotAttackPosition;
				AttackToPlayer.Normalize();

				const float Angle = FMath::RadiansToDegrees(acosf(ItemToPlayer.CosineAngle2D(AttackToPlayer)));
				MinAngle = (Angle > MinAngle) ? MinAngle : Angle;
			}
		}
		It.SetScore(TestPurpose, FilterType, MinAngle, MinThresholdValue, MaxThresholdValue);
	}
}
