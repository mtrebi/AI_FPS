// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "Public/Bots/ShooterBot.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
#include "Public/Bots/ShooterAIController.h"
#include "Public/EQS/CoverBaseClass.h"
#include "Public/EQS/AttackPositionTest.h"


UAttackPositionTest::UAttackPositionTest(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Context = UEnvQueryContext_Querier::StaticClass();
	Cost = EEnvTestCost::Low;
	ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();
}


void UAttackPositionTest::RunTest(FEnvQueryInstance& QueryInstance) const
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

	// don't support context Item here, it doesn't make any sense
	TArray<FVector> ContextLocations;
	if (!QueryInstance.PrepareContext(Context, ContextLocations))
	{
		return;
	}

	// Get all covers annotations within radius
	UWorld * World = GEngine->GetWorldFromContextObject(QueryOwner);
	const TArray<FVector> LocationOfCoverAnnotations = GetLocationOfCoverAnnotationsWithinRadius(World, ContextLocations[0]);


	TArray<FVector> BehindCoverLocations;
	// Get all behind cover points
	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		float CurrentDistance;
		float MinDistance = 100000;
		const FVector Location = GetItemLocation(QueryInstance, *It);
		for (auto It2 = LocationOfCoverAnnotations.CreateConstIterator(); It2; ++It2) {
			const FVector CoverLocation = *It2;
			CurrentDistance = FVector::Dist(Location, CoverLocation);
			MinDistance = (CurrentDistance < MinDistance) ? CurrentDistance : MinDistance;
		}
		// Discard items that are far away of a cover annotation
		if (MinDistance > DiscardDistanceMax) {
			//It.DiscardItem();
		}
		else if (World) {
			FHitResult OutHit;
			FCollisionQueryParams CollisionParams;
			TArray<AActor*> ActorsToIgnore;

			UGameplayStatics::GetAllActorsOfClass(World, AShooterCharacter::StaticClass(), ActorsToIgnore);
			CollisionParams.AddIgnoredActors(ActorsToIgnore);

			// Check distance to nearest cover and use it to score (the closer the better)
			const bool BlockingHitFound = World->LineTraceSingleByChannel(OutHit, Location, ContextLocations[0], ECollisionChannel::ECC_Visibility, CollisionParams);
			if (BlockingHitFound) {
				const float DistanceToCoverInContextDirection = FVector::Dist(OutHit.ImpactPoint, Location);
				It.SetScore(TestPurpose, FilterType, MinDistance , MinThresholdValue, MaxThresholdValue);
				//BehindCoverLocations.Add(Location);
			}
			else {
				It.DiscardItem();
			}
		}
	}
	
	// Check visibility of points to behind cover locations
	for (FEnvQueryInstance::ItemIterator It3(this, QueryInstance); It3; ++It3)
	{
		const FVector Location = GetItemLocation(QueryInstance, *It3);

		FHitResult OutHit;
		FCollisionQueryParams CollisionParams;
		TArray<AActor*> ActorsToIgnore;

		UGameplayStatics::GetAllActorsOfClass(World, AShooterBot::StaticClass(), ActorsToIgnore);
		CollisionParams.AddIgnoredActors(ActorsToIgnore);

		// Check visibility to player
		const bool BlockingHitFound = World->LineTraceSingleByChannel(OutHit, Location, ContextLocations[0], ECollisionChannel::ECC_Visibility, CollisionParams);
		if (BlockingHitFound && OutHit.Actor->GetClass() == AShooterCharacter::StaticClass()) {
			// There is visibility. Lets check distance to nearest cover!
			float CurrentDistance;
			float MinDistance = 100000;

			for (auto It2 = BehindCoverLocations.CreateConstIterator(); It2; ++It2) {
				const FVector BehindCoverLocation = *It2;
				CurrentDistance = FVector::Dist(Location, BehindCoverLocation);
				MinDistance = (CurrentDistance < MinDistance) ? CurrentDistance : MinDistance;
			}
			if (MinDistance < DiscardDistanceMax) {
				It3.SetScore(TestPurpose, FilterType, MinDistance, MinThresholdValue, MaxThresholdValue);

			}
			else {
				// Discard items that are far away of a cover annotation
				It3.DiscardItem();
			}
		}
		else {
			// No visibility
			It3.DiscardItem();
		}
	}
}
/*
void UAttackPositionTest::RunTest(FEnvQueryInstance& QueryInstance) const
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

	// don't support context Item here, it doesn't make any sense
	TArray<FVector> ContextLocations;
	if (!QueryInstance.PrepareContext(Context, ContextLocations))
	{
		return;
	}

	// Get all covers annotations within radius
	UWorld * World = GEngine->GetWorldFromContextObject(QueryOwner);
	const TArray<FVector> LocationOfCoverAnnotations = GetLocationOfCoverAnnotationsWithinRadius(World, ContextLocations[0]);

	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		float CurrentDistance;
		float MinDistance = 100000;
		const FVector Location = GetItemLocation(QueryInstance, *It);
		for (auto It2 = LocationOfCoverAnnotations.CreateConstIterator(); It2; ++It2) {
			const FVector CoverLocation = *It2;
			CurrentDistance = FVector::Dist(Location, CoverLocation);
			MinDistance = (CurrentDistance < MinDistance) ? CurrentDistance : MinDistance;
		}
		// Discard items that are far away of a cover annotation
		if (MinDistance > DiscardDistance) {
			It.DiscardItem();
		}
		else if (World) {
			// Discard points that are not visible to the player
			FHitResult OutHit;
			FCollisionQueryParams CollisionParams;
			AShooterAIController * BotController;
			TArray<AActor*> ActorsToIgnore;
			TArray<AActor*> AIControllers;

			UGameplayStatics::GetAllActorsOfClass(World, AShooterBot::StaticClass(), ActorsToIgnore);
			CollisionParams.AddIgnoredActors(ActorsToIgnore);

			UGameplayStatics::GetAllActorsOfClass(World, AShooterAIController::StaticClass(), AIControllers);
			if (AIControllers.Num() > 0) {
				BotController = Cast<AShooterAIController>(AIControllers[0]);
				FVector PlayerLocation = FVector(0, 0, 0);
				if (BotController->GetPlayer()) {
					PlayerLocation = BotController->GetPlayerLastLocation();
				}
				else {
					//@todo prediction
				}

				if (PlayerLocation != FVector(0, 0, 0)) {
					// Check distance to nearest cover and use it to score (the closer the better)
					const bool BlockingHitFound = World->LineTraceSingleByChannel(OutHit, Location, PlayerLocation, ECollisionChannel::ECC_Visibility, CollisionParams);
					if (BlockingHitFound && OutHit.Actor->GetClass() == AShooterCharacter::StaticClass()) {
						// There is visibility
						It.SetScore(TestPurpose, FilterType, MinDistance, MinThresholdValue, MaxThresholdValue);
					}
					else {
						// No Visibility
						It.DiscardItem();
					}
				}
				else {
					// We dont know the player location

				}
			}
			else {
				//Dont have AI Controller
				It.DiscardItem();
			}
			



		}
	}
}*/

TArray<FVector> UAttackPositionTest::GetLocationOfCoverAnnotationsWithinRadius(UWorld * World, const FVector ContextLocation) const {
	TArray<FVector> LocationOfCoverAnnotations;
	TArray<AActor*> Actors;

	UGameplayStatics::GetAllActorsOfClass(World, ACoverBaseClass::StaticClass(), Actors);
	for (auto It = Actors.CreateConstIterator(); It; ++It) {
		const AActor* Cover = *It;
		const float Distance = FVector::Dist(Cover->GetActorLocation(), ContextLocation);
		if (Distance <= MaxRadius) {
			TArray<AActor*> CoverAnnotations;
			Cover->GetAttachedActors(CoverAnnotations);
			for (auto It2 = CoverAnnotations.CreateConstIterator(); It2; ++It2) {
				LocationOfCoverAnnotations.Add((*It2)->GetActorLocation());
			}
		}
	}
	return LocationOfCoverAnnotations;
}


void UAttackPositionTest::PostLoad()
{
	Super::PostLoad();
}



