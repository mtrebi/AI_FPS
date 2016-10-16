// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
#include "Public/Bots/ShooterBot.h"
#include "Public/EQS/NearBehindObstacleTest.h"


UNearBehindObstacleTest::UNearBehindObstacleTest(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Cost = EEnvTestCost::High;
	ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();
}

void UNearBehindObstacleTest::RunTest(FEnvQueryInstance& QueryInstance) const
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
	
	// Get all behind cover points
	TArray<FVector> LocationsBehindObstacles;
	TArray<float> LocationsBehindObstacles_Distance;

	if (TraceToPlayer) {
		const FVector PlayerPositionFromAI = HelperMethods::GetPlayerPositionFromAI(World);
		if (TraceToPlayer) {
			for (int Index = 0; Index < QueryInstance.Items.Num(); ++Index) {
				const FVector ItemLocation = GetItemLocation(QueryInstance, Index);
				const FVector UpLocation = FVector(ItemLocation.X, ItemLocation.Y, 150);
				if (World) {
					FHitResult OutHit;
					FCollisionQueryParams CollisionParams;
					TArray<AActor*> ActorsToIgnore;

					UGameplayStatics::GetAllActorsOfClass(World, AShooterCharacter::StaticClass(), ActorsToIgnore);
					CollisionParams.AddIgnoredActors(ActorsToIgnore);

					// Check distance to nearest cover and use it to score (the closer the better)
					const bool BlockingHitFound = World->LineTraceSingleByChannel(OutHit, UpLocation, PlayerPositionFromAI, ECollisionChannel::ECC_Visibility, CollisionParams);
					if (BlockingHitFound) {
						LocationsBehindObstacles.Add(UpLocation);
						LocationsBehindObstacles_Distance.Add(FVector::Dist(UpLocation, OutHit.ImpactPoint));
					}

				}
			}
		}
	}
	else {
		TArray<AActor*> Bots;
		UGameplayStatics::GetAllActorsOfClass(World, AShooterBot::StaticClass(), Bots);
		TArray<FVector> CharacterPositions;

		for (auto BotIterator = Bots.CreateConstIterator(); BotIterator; ++BotIterator) {
			AShooterBot * Bot = Cast<AShooterBot>(*BotIterator);
			if (Bot && Bot->IsAlive()) {
				FVector BotPosition = Bot->GetActorLocation();
				CharacterPositions.Add(BotPosition);
			}
		}

		for (int Index = 0; Index < QueryInstance.Items.Num(); ++Index) {
			const FVector ItemLocation = GetItemLocation(QueryInstance, Index);
			const FVector UpLocation = FVector(ItemLocation.X, ItemLocation.Y, 150);
			int Hits = 0;
			if (World) {
				FHitResult OutHit;
				FCollisionQueryParams CollisionParams;
				TArray<AActor*> ActorsToIgnore;

				UGameplayStatics::GetAllActorsOfClass(World, AShooterCharacter::StaticClass(), ActorsToIgnore);
				CollisionParams.AddIgnoredActors(ActorsToIgnore);

				for (auto BotPosIterator = CharacterPositions.CreateConstIterator(); BotPosIterator; ++BotPosIterator) {
					FVector BotPosition = *BotPosIterator;

					// Check distance to nearest cover and use it to score (the closer the better)
					const bool BlockingHitFound = World->LineTraceSingleByChannel(OutHit, UpLocation, BotPosition, ECollisionChannel::ECC_Visibility, CollisionParams);
					if (BlockingHitFound) {
						LocationsBehindObstacles.Add(UpLocation);
						LocationsBehindObstacles_Distance.Add(FVector::DistSquared(UpLocation, OutHit.ImpactPoint));
					}
				}
			}
		}
	}




	
	// Score those positions that are nearer to behind obstacle points
	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It){
		const FVector ItemLocation = GetItemLocation(QueryInstance, *It);

		float CurrentDistance;
		float MinDistance = 100000;
		float MinDistanceIndex = 0;
		for (auto It2 = LocationsBehindObstacles.CreateConstIterator(); It2; ++It2) {
			const FVector BehindObstacleCover = *It2;
			CurrentDistance = FVector::Dist(ItemLocation, BehindObstacleCover);
			MinDistanceIndex = (CurrentDistance < MinDistance) ? It2.GetIndex() : MinDistanceIndex;
			MinDistance = (CurrentDistance < MinDistance || MinDistance == 0) ? CurrentDistance : MinDistance;

		}
		if (MinDistance < DiscardDistanceMax && MinDistance > DiscardDistanceMin) {
			float DistanceToObstacle = LocationsBehindObstacles_Distance[MinDistanceIndex];
			
			if (DistanceToObstacle < 250 ) {
				It.SetScore(TestPurpose, FilterType, 0.5 * FMath::Square(MinDistance) + 0.5 * DistanceToObstacle, MinThresholdValue, MaxThresholdValue);
				//It.SetScore(TestPurpose, FilterType, Distance , MinThresholdValue, MaxThresholdValue);
			}
			else {
				It.DiscardItem();
			}
		}
		else {
			It.DiscardItem();
			//It.SetScore(TestPurpose, FilterType, MaxThresholdValue + 1, MinThresholdValue, MaxThresholdValue);
		}
	}
}