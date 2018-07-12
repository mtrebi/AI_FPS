// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "Bots/ShooterBot.h"
#include "Bots/ShooterAIController.h"
#include "Public/EQS/CoverBaseClass.h"
#include "Public/Navigation/MyRecastNavMesh.h"
#include "Public/Others/HelperMethods.h"

FVector HelperMethods::GetPlayerPositionFromAI(UWorld * World) {
	FVector PlayerPosition;
	TArray<AActor *> AIBots;

	UGameplayStatics::GetAllActorsOfClass(World, AShooterBot::StaticClass(), AIBots);

	if (AIBots.Num() == 0) {
		return PlayerPosition;
	}

	AShooterBot* AnyBot = Cast<AShooterBot>(AIBots[0]);

	AShooterAIController* BotController = Cast<AShooterAIController>(AnyBot->GetController());
	if (BotController) {
		PlayerPosition = BotController->GetPL_fLocation();
	}
	else {
		// Just to debug purposes
		TArray<AActor *> AllShooterCharacters;
		UGameplayStatics::GetAllActorsOfClass(World, AShooterCharacter::StaticClass(), AllShooterCharacters);

		AShooterCharacter* PlayerCharacter = NULL;
		for (auto It = AllShooterCharacters.CreateConstIterator(); It; ++It) {
			AActor * Actor = *It;
			if (!Cast<AShooterBot>(Actor)) {
				PlayerCharacter = Cast<AShooterCharacter>(Actor);
				break;
			}
		}
		if (!PlayerCharacter) {
			PlayerPosition = FVector(0, 0, 0);
		}
		else {
			PlayerPosition = PlayerCharacter->GetActorLocation();
		}
		
	}

	return PlayerPosition;
}

FVector HelperMethods::GetPlayerForwardVectorFromAI(UWorld * World) {
	FVector PlayerForwardVector;
	TArray<AActor *> AIBots;

	UGameplayStatics::GetAllActorsOfClass(World, AShooterBot::StaticClass(), AIBots);

	if (AIBots.Num() == 0) {
		return PlayerForwardVector;
	}

	AShooterBot* AnyBot = Cast<AShooterBot>(AIBots[0]);

	AShooterAIController* BotController = Cast<AShooterAIController>(AnyBot->GetController());
	if (BotController && BotController->GetPL_fPlayer()) {
		// @todo
		PlayerForwardVector = BotController->GetPL_fPlayer()->GetActorForwardVector();
	}
	else {
		// Just to debug purposes
		TArray<AActor *> AllShooterCharacters;
		UGameplayStatics::GetAllActorsOfClass(World, AShooterCharacter::StaticClass(), AllShooterCharacters);

		AShooterCharacter* PlayerCharacter = NULL;
		for (auto It = AllShooterCharacters.CreateConstIterator(); It; ++It) {
			AActor * Actor = *It;
			if (!Cast<AShooterBot>(Actor)) {
				PlayerCharacter = Cast<AShooterCharacter>(Actor);
				break;
			}
		}
		PlayerForwardVector = PlayerCharacter->GetActorForwardVector();
	}

	return PlayerForwardVector;
}

// http://www.redblobgames.com/articles/visibility/
// http://gamedev.stackexchange.com/questions/21897/quick-2d-sight-area-calculation-algorithm
TArray<Triangle> HelperMethods::CalculateVisibility(UWorld * World, const FVector Location, const FVector ForwardVector, const float ViewAngle, const float ViewDistance){
	TArray<Triangle> VisibleLocations;

	if (World) {
		//APawn * PlayerPawn = World->GetFirstPlayerController()->GetPawn();
		
		TArray<Triangle> VisibleTriangles;

		TArray<AActor*> CubeActors;
		TArray<Vertex> VisibleVertexs;


		const FVector EyesLocation = FVector(Location.X, Location.Y, HelperMethods::EYES_POS_Z);
		//FVector ForwardVector = UGameplayStatics::GetPlayerCameraManager(World, 0)->GetActorForwardVector();
		const FVector NewForwardVector = FVector(ForwardVector.X, ForwardVector.Y, 0);
		// Calculate max start and max end point of the player's FOV
		const FVector EndOfFirstTrace = EyesLocation + FRotator(0, -ViewAngle, 0).RotateVector(NewForwardVector) * ViewDistance;
		const FVector EndOfLastTrace = EyesLocation + FRotator(0, ViewAngle, 0).RotateVector(NewForwardVector) * ViewDistance;

		Vertex FirstTraceVertex;
		FirstTraceVertex.V = EndOfFirstTrace;
		FirstTraceVertex.LeftmostVertex = true;
		FirstTraceVertex.RightmostVertex = false;

		Vertex LastTraceVertex;
		LastTraceVertex.V = EndOfLastTrace;
		LastTraceVertex.LeftmostVertex = false;
		LastTraceVertex.RightmostVertex = true;

		// Add them to visible vertexes
		VisibleVertexs = HelperMethods::GetVisibleObstaclesVertexs(World, EyesLocation, NewForwardVector, ViewAngle, ViewDistance);
		VisibleVertexs.Add(FirstTraceVertex);
		VisibleVertexs.Add(LastTraceVertex);

		// Sort the vertexs according to the angle between them and the camera, and the camera forward vector
		HelperMethods::SortByAngle(VisibleVertexs, EyesLocation, EndOfFirstTrace);
		VisibleLocations = HelperMethods::GetVisibleTriangles(VisibleVertexs, World, EyesLocation);
	}

	return VisibleLocations;
}

TArray<Vertex> HelperMethods::GetVisibleObstaclesVertexs(UWorld * World, const FVector EyesLocation, const FVector ForwardVector, const float ViewAngle, const float ViewDistance) {
	//UE_LOG(LogTemp, Log, TEXT("F:GetVisibleObstaclesVertexs"));
	TArray<AActor*> CubeActors;
	TArray<Vertex> VisibleVertexs;

	// Get All Boxes that are inside the FOV of the player
	UGameplayStatics::GetAllActorsOfClass(World, ACoverBaseClass::StaticClass(), CubeActors);
	for (auto It = CubeActors.CreateConstIterator(); It; ++It) {
		const AActor* Cube = *It;
		// Calculate the bounding box that contains the actor (Cube)
		FVector Origin;
		FVector BoundsExtent;
		Cube->GetActorBounds(false, Origin, BoundsExtent);

		// Ignore obstacles shorter than Eye pos
		if (Origin.Z + BoundsExtent.Z >= HelperMethods::EYES_POS_Z) {
			TArray<FVector> Aux;

			// Get the four upper vertexs
			const FVector UpperVertex1 = FVector(Origin.X + BoundsExtent.X, Origin.Y + BoundsExtent.Y, HelperMethods::EYES_POS_Z);
			const FVector UpperVertex2 = FVector(Origin.X - BoundsExtent.X, Origin.Y + BoundsExtent.Y, HelperMethods::EYES_POS_Z);
			const FVector UpperVertex3 = FVector(Origin.X + BoundsExtent.X, Origin.Y - BoundsExtent.Y, HelperMethods::EYES_POS_Z);
			const FVector UpperVertex4 = FVector(Origin.X - BoundsExtent.X, Origin.Y - BoundsExtent.Y, HelperMethods::EYES_POS_Z);

			Aux.Add(UpperVertex1);
			Aux.Add(UpperVertex2);
			Aux.Add(UpperVertex3);
			Aux.Add(UpperVertex4);

			Aux.Sort([=](const FVector &Pos1, const FVector &Pos2) {
				FVector FirstTraceDirection = FRotator(0, -ViewAngle * 2, 0).RotateVector(ForwardVector);
				FVector VPos1 = Pos1 - EyesLocation; // Vertex 1 to Camera
				FVector VPos2 = Pos2 - EyesLocation; // Vertex 2 to Camera
				FirstTraceDirection.Normalize();
				VPos1.Normalize();
				VPos2.Normalize();

				const float Angle1 = FMath::RadiansToDegrees(acosf(FirstTraceDirection.CosineAngle2D(VPos1))); // Angle with First trace vector
				const float Angle2 = FMath::RadiansToDegrees(acosf(FirstTraceDirection.CosineAngle2D(VPos2))); // Angle with First trace vector

				return (Angle1 < Angle2);

			});

			int counter = 0;
			for (auto It2 = Aux.CreateConstIterator(); It2; ++It2) {
				FVector V = *It2;
				const float Angle = FMath::RadiansToDegrees(acosf((V - EyesLocation).CosineAngle2D(ForwardVector)));
				if (Angle < ViewAngle) { // Inside Player's FOV
					const float Distance = FVector::Dist(V, EyesLocation);
					if (Distance < ViewDistance) { // Inside Player's FOV Max DIstance
						Vertex vertex;
						vertex.V = V;
						vertex.LeftmostVertex = counter == 0;
						vertex.RightmostVertex = counter == 3;

						VisibleVertexs.Add(vertex);
					}
				}
				++counter;
			}
		}
	}

	return VisibleVertexs;
}

void HelperMethods::SortByAngle(TArray<Vertex> &FVectorArray, const FVector EyesLocation, const FVector EndOfFirstTrace)  {
	//UE_LOG(LogTemp, Log, TEXT("F:SortByAngle"));

	FVectorArray.Sort([=](const Vertex &Pos1, const Vertex &Pos2) {
		FVector FirstTraceDirection = EndOfFirstTrace - EyesLocation; // Leftmost point to Camera
		FVector VPos1 = Pos1.V - EyesLocation; // Vertex 1 to Camera
		FVector VPos2 = Pos2.V - EyesLocation; // Vertex 2 to Camera
		FirstTraceDirection.Normalize();
		VPos1.Normalize();
		VPos2.Normalize();

		const float Angle1 = FMath::RadiansToDegrees(acosf(FirstTraceDirection.CosineAngle2D(VPos1))); // Angle with First trace vector
		const float Angle2 = FMath::RadiansToDegrees(acosf(FirstTraceDirection.CosineAngle2D(VPos2))); // Angle with First trace vector

		return (Angle1 < Angle2);
	});
}

TArray<Triangle> HelperMethods::GetVisibleTriangles(const TArray<Vertex> VisibleVertexs, UWorld * World, const FVector EyesLocation, const float ViewAngle, const float ViewDistance) {
	//UE_LOG(LogTemp, Log, TEXT("F:CalculateVisibleTriangles"));
	TArray<Triangle> VisibleTriangles;
	FHitResult OutHit;
	FCollisionQueryParams CollisionParams;
	TArray<AActor*> ActorsToIgnore;
	//@todo debug
	//const FName TraceTag("VisibilityTrace");
	//World->DebugDrawTraceTag = TraceTag;
	//CollisionParams.TraceTag = TraceTag;
	UGameplayStatics::GetAllActorsOfClass(World, AShooterCharacter::StaticClass(), ActorsToIgnore);
	CollisionParams.AddIgnoredActors(ActorsToIgnore);

	FVector PreviousVertex = FVector(0, 0, 0);
	for (auto It = VisibleVertexs.CreateConstIterator(); It; ++It) {
		const Vertex Vertex = *It;
		FVector TraceDirection = Vertex.V - EyesLocation;
		TraceDirection.Normalize();

		const FVector Offset = TraceDirection * HelperMethods::OFFSET;
		const FVector MaxProjectedVertex = EyesLocation + TraceDirection * ViewDistance;
		const bool BlockingHitFound = World->LineTraceSingleByChannel(OutHit, EyesLocation, Vertex.V + Offset, ECollisionChannel::ECC_Visibility, CollisionParams);
		// Only First Trace
		if (PreviousVertex == FVector(0, 0, 0)) {
			if (BlockingHitFound) {
				PreviousVertex = OutHit.ImpactPoint;
			}
			else {
				const bool ProjectedBlockingHitFound = World->LineTraceSingleByChannel(OutHit, EyesLocation, MaxProjectedVertex, ECollisionChannel::ECC_Visibility, CollisionParams);
				if (ProjectedBlockingHitFound) {
					PreviousVertex = OutHit.ImpactPoint; // NOPE
				}
				else {
					PreviousVertex = OutHit.TraceEnd;
				}
			}
		}
		else {
			Triangle triangle = Triangle();
			triangle.V2 = EyesLocation;
			triangle.V3 = PreviousVertex;

			if (BlockingHitFound) {
				triangle.V1 = OutHit.ImpactPoint;
				PreviousVertex = triangle.V1;
			}
			else {
				const bool ProjectedBlockingHitFound = World->LineTraceSingleByChannel(OutHit, EyesLocation, MaxProjectedVertex, ECollisionChannel::ECC_Visibility, CollisionParams);
				FVector ProjectedVertex;
				if (ProjectedBlockingHitFound) {
					ProjectedVertex = OutHit.ImpactPoint;
				}
				else {
					ProjectedVertex = OutHit.TraceEnd;
				}

				if (Vertex.LeftmostVertex) {
					triangle.V1 = ProjectedVertex;
					PreviousVertex = Vertex.V;
				}
				else if (Vertex.RightmostVertex) {
					triangle.V1 = Vertex.V;
					PreviousVertex = ProjectedVertex;
				}
				else {
					triangle.V1 = Vertex.V;
					PreviousVertex = Vertex.V;
				}
			}
			VisibleTriangles.Add(triangle);
			//PaintTriangle(triangle);  
		}
	}
	return VisibleTriangles;
}
/*
TArray<FVector> HelperMethods::GetLocationOfCoverAnnotationsWithinRadius(UWorld * World, const FVector ContextLocation, const float MaxRadius) {
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
*/
/*
void HelperMethods::PaintTriangle(const Triangle Triangle) {
	//UE_LOG(LogTemp, Log, TEXT("F:PaintTriangle"));
	const TArray<FVector> PointsInsideTriangle = Triangle.GetPointsInsideTriangle();

	//@todo GetCustomFilter()->GetInfluenceMap()->Initialize();

	// Paint each point of each triangle
	for (auto It = PointsInsideTriangle.CreateConstIterator(); It; ++It) {
		const FVector CurrentPoint = *It;
		GetCustomFilter()->GetInfluenceMap()->SetInfluence(CurrentPoint, 255.0);
	}
}

FRecastQueryFilter_Example* HelperMethods::GetCustomFilter() {
	FRecastQueryFilter_Example* MyFRecastQueryFilter = NULL;
	UWorld * World = GetWorld();
	if (World) {
		UNavigationSystem* NavSys = World->GetNavigationSystem();
		if (NavSys) {
			ANavigationData* NavData = NavSys->GetMainNavData(FNavigationSystem::Create);
			AMyRecastNavMesh* MyNavMesh = Cast<AMyRecastNavMesh>(NavData);
			MyFRecastQueryFilter = MyNavMesh->GetCustomFilter();
		}
	}
	return MyFRecastQueryFilter;
}
*/