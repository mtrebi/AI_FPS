// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
#include "Public/EQS/CoverBaseClass.h"
#include "Public/Bots/ShooterBot.h"
#include "Public/Others/HelperMethods.h"
#include "Public/EQS/NearCoverAnnotationTest.h"

UNearCoverAnnotationTest::UNearCoverAnnotationTest(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Cost = EEnvTestCost::Low;
	ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();
	SetWorkOnFloatValues(false);
}

void UNearCoverAnnotationTest::RunTest(FEnvQueryInstance& QueryInstance) const
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
	UNavigationSystem* NavSys = World->GetNavigationSystem();
	

	if (!NavSys) {
		return;
	}

	ANavigationData* NavData = NavSys->GetMainNavData(FNavigationSystem::Create);
	AMyRecastNavMesh* MyNavMesh = Cast<AMyRecastNavMesh>(NavData);
	FRecastQueryFilter_Example* MyFRecastQueryFilter = MyNavMesh->GetCustomFilter();

	const FVector PlayerLocation = HelperMethods::GetPlayerPositionFromAI(World);
	const FVector PlayerForwardVector = HelperMethods::GetPlayerForwardVectorFromAI(World);
	const TArray<Triangle> VisibleTriangles = HelperMethods::CalculateVisibility(World, PlayerLocation, PlayerForwardVector);
	



	// Get all behind cover points
	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		It.SetScore(TestPurpose, FilterType, true, bWantsHit);
	}
}

void UNearCoverAnnotationTest::PostLoad()
{
	Super::PostLoad();
}


