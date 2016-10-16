// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
#include "Bots/ShooterAIController.h"
#include "Bots/ShooterBot.h"
#include "Public/Others/HelperMethods.h"
#include "Public/EQS/CloserHighInfluencePositionsTest.h"


UCloserHighInfluencePositionsTest::UCloserHighInfluencePositionsTest(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Cost = EEnvTestCost::Low;
	ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();
}

void UCloserHighInfluencePositionsTest::RunTest(FEnvQueryInstance& QueryInstance) const
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


	AMyInfluenceMap * InfluenceMap = AIController->GetAI_PredictionMap();
	
	if (!InfluenceMap) {
		return;
	}

	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It) {
		const FVector ItemLocation = GetItemLocation(QueryInstance, *It);
		InfluenceTile * ItemTile = InfluenceMap->GetInfluence(ItemLocation);
		TArray<InfluenceTile *> Neighbors = InfluenceMap->GetWalkableNeighbors(ItemTile->Index);

		float Score = 0;
		float SumNeighborsInfluence = 0;
		for (auto ItNeighbor = Neighbors.CreateConstIterator(); ItNeighbor; ++ItNeighbor) {
			InfluenceTile * NeighborTile = *ItNeighbor;
			SumNeighborsInfluence += NeighborTile->Influence;

		}
		Score = 0.6 * ItemTile->Influence + 0.4 * (SumNeighborsInfluence / Neighbors.Num());
		It.SetScore(TestPurpose, FilterType, Score , MinThresholdValue, MaxThresholdValue);
	}
}



