// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
#include "Public/EQS/CoverBaseClass.h"
#include "Public/Bots/ShooterBot.h"
#include "Public/Others/HelperMethods.h"
#include "Public/EQS/PlayerVisibilityTest.h"

UPlayerVisibilityTest::UPlayerVisibilityTest(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Cost = EEnvTestCost::Low;
	ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();
	SetWorkOnFloatValues(false);
}

void UPlayerVisibilityTest::RunTest(FEnvQueryInstance& QueryInstance) const
{
	//UE_LOG(LogTemp, Log, TEXT("F:RunTest"));

	UObject* QueryOwner = QueryInstance.Owner.Get();
	BoolValue.BindData(QueryOwner, QueryInstance.QueryID);

	bool bWantsHit = BoolValue.GetValue();

	if (QueryOwner == nullptr)
	{
		return;
	}

	UWorld * World = GEngine->GetWorldFromContextObject(QueryOwner);
	const FVector PlayerLocation = HelperMethods::GetPlayerPositionFromAI(World);
	const FVector EyesLocation = FVector(PlayerLocation.X, PlayerLocation.Y, HelperMethods::EYES_POS_Z);
	const FVector PlayerForwardVector = HelperMethods::GetPlayerForwardVectorFromAI(World);

	if (!PanoramicView) {
		// Custom method
		// Get all covers annotations within radius

		const TArray<Triangle> PlayerVisibility = HelperMethods::CalculateVisibility(World, PlayerLocation, PlayerForwardVector);
		for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
		{
			bool IsVisible = false;
			const FVector Location = GetItemLocation(QueryInstance, *It);
			for (auto It2 = PlayerVisibility.CreateConstIterator(); It2; ++It2) {
				const Triangle Triangle = *It2;
				if (Triangle.PointInsideTriangle(FVector2D(Location.X, Location.Y))) {
					IsVisible = true;
					break;
				}
			}
			It.SetScore(TestPurpose, FilterType, IsVisible, bWantsHit);
		}
	}
	else {
		
		// Ray Trace
		FHitResult OutHit;
		FCollisionQueryParams CollisionParams;
		TArray<AActor*> ActorsToIgnore;

		UGameplayStatics::GetAllActorsOfClass(World, AShooterCharacter::StaticClass(), ActorsToIgnore);
		CollisionParams.AddIgnoredActors(ActorsToIgnore);

		for (FEnvQueryInstance::ItemIterator It2(this, QueryInstance); It2; ++It2)
		{
			const FVector Location = GetItemLocation(QueryInstance, *It2);
			const FVector UpLocation = FVector(Location.X, Location.Y, 150);
			const bool BlockingHitFound = World->LineTraceSingleByChannel(OutHit, EyesLocation, UpLocation, ECollisionChannel::ECC_Visibility, CollisionParams);

			It2.SetScore(TestPurpose, FilterType, !BlockingHitFound, bWantsHit);
		}
	}

	
}