// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
#include "Public/Bots/ShooterBot.h"
#include "Public/Bots/ShooterAIController.h"
#include "Public/EQS/BehindObstacleTest.h"


UBehindObstacleTest::UBehindObstacleTest(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Cost = EEnvTestCost::High;
	ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();
}

void UBehindObstacleTest::RunTest(FEnvQueryInstance& QueryInstance) const
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
	const FVector PlayerPosition = HelperMethods::GetPlayerPositionFromAI(World);
	const FVector EyesPosition = FVector(PlayerPosition.X, PlayerPosition.Y, HelperMethods::EYES_POS_Z);
	
	FHitResult OutHit, OutHit1, OutHit2, OutHit3, OutHit4;
	FCollisionQueryParams CollisionParams;
	TArray<AActor*> ActorsToIgnore;

	UGameplayStatics::GetAllActorsOfClass(World, AShooterCharacter::StaticClass(), ActorsToIgnore);
	CollisionParams.AddIgnoredActors(ActorsToIgnore);

	// Get all behind cover points
	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It){
		const FVector ItemLocation = GetItemLocation(QueryInstance, *It);
		const FVector UpItemLocation = FVector(ItemLocation.X, ItemLocation.Y, HelperMethods::EYES_POS_Z);
		
		if (World) {

			const bool BlockingHitFound = World->LineTraceSingleByChannel(OutHit, UpItemLocation, EyesPosition, ECollisionChannel::ECC_Visibility, CollisionParams);

			if (BlockingHitFound) {
				FHitResult  OutHit1, OutHit2, OutHit3, OutHit4;
				const FVector UpItemLocation1 = FVector(UpItemLocation.X + 75, UpItemLocation.Y, UpItemLocation.Z);
				const FVector UpItemLocation2 = FVector(UpItemLocation.X - 75, UpItemLocation.Y, UpItemLocation.Z);
				const FVector UpItemLocation3 = FVector(UpItemLocation.X, UpItemLocation.Y + 75, UpItemLocation.Z);
				const FVector UpItemLocation4 = FVector(UpItemLocation.X, UpItemLocation.Y - 75, UpItemLocation.Z);

				const bool BlockingHitFound1 = World->LineTraceSingleByChannel(OutHit1, UpItemLocation1, EyesPosition, ECollisionChannel::ECC_Visibility, CollisionParams);
				const bool BlockingHitFound2 = World->LineTraceSingleByChannel(OutHit2, UpItemLocation2, EyesPosition, ECollisionChannel::ECC_Visibility, CollisionParams);
				const bool BlockingHitFound3 = World->LineTraceSingleByChannel(OutHit3, UpItemLocation3, EyesPosition, ECollisionChannel::ECC_Visibility, CollisionParams);
				const bool BlockingHitFound4 = World->LineTraceSingleByChannel(OutHit4, UpItemLocation4, EyesPosition, ECollisionChannel::ECC_Visibility, CollisionParams);

				if (BlockingHitFound1 && BlockingHitFound2 & BlockingHitFound3 && BlockingHitFound4) {
					const float DistanceToCoverInEyesDirection = FVector::Dist(OutHit.ImpactPoint, UpItemLocation);
					It.SetScore(TestPurpose, FilterType, DistanceToCoverInEyesDirection, MinThresholdValue, MaxThresholdValue);
				}
				else {
					It.SetScore(TestPurpose, FilterType, MaxThresholdValue + 1, MinThresholdValue, MaxThresholdValue);
				}
	
			/*
			if (BlockingHitFound) {
				TArray<FVector> ActorVertexs;
				FVector Origin, BoundsExtent;

				OutHit.GetActor()->GetActorBounds(false, Origin, BoundsExtent);
				
				const FVector Vertex1 = FVector(Origin.X + BoundsExtent.X, Origin.Y, BoundsExtent.Z);
				const FVector Vertex2 = FVector(Origin.X - BoundsExtent.X, Origin.Y, BoundsExtent.Z);
				const FVector Vertex3 = FVector(Origin.X, Origin.Y + BoundsExtent.Y, BoundsExtent.Z);
				const FVector Vertex4 = FVector(Origin.X, Origin.Y - BoundsExtent.Y, BoundsExtent.Z);
				
				
				//const FVector Vertex1 = FVector(Origin.X + BoundsExtent.X, Origin.Y + BoundsExtent.Y, HelperMethods::EYES_POS_Z);
				//const FVector Vertex2 = FVector(Origin.X - BoundsExtent.X, Origin.Y + BoundsExtent.Y, HelperMethods::EYES_POS_Z);
				//const FVector Vertex3 = FVector(Origin.X + BoundsExtent.X, Origin.Y - BoundsExtent.Y, HelperMethods::EYES_POS_Z);
				//const FVector Vertex4 = FVector(Origin.X - BoundsExtent.X, Origin.Y - BoundsExtent.Y, HelperMethods::EYES_POS_Z);
				

				ActorVertexs.Add(Vertex1);
				ActorVertexs.Add(Vertex2);
				ActorVertexs.Add(Vertex3);
				ActorVertexs.Add(Vertex4);
				
				float MinDistance = 10000;
				for (auto ItVertex = ActorVertexs.CreateConstIterator(); ItVertex; ++ItVertex) {
					const FVector Vertex = *ItVertex;
					const float Distance = FVector::Dist(Vertex, UpItemLocation);
					MinDistance = Distance < MinDistance ? Distance : MinDistance;
				}

				const float DistanceToCoverInEyesDirection = FVector::Dist(OutHit.ImpactPoint, UpItemLocation);
				const float DistanceToObstacle = FVector::Dist(OutHit.Actor->GetActorLocation(), UpItemLocation);
				*/
				//It.SetScore(TestPurpose, FilterType, MinDistance, MinThresholdValue, MaxThresholdValue);
				//It.SetScore(TestPurpose, FilterType, DistanceToObstacle, MinThresholdValue, MaxThresholdValue);
				//It.SetScore(TestPurpose, FilterType, DistanceToCoverInEyesDirection, MinThresholdValue, MaxThresholdValue);
			}
			else {
				It.SetScore(TestPurpose, FilterType, MaxThresholdValue + 1, MinThresholdValue, MaxThresholdValue);
			}
		}
	}
}


