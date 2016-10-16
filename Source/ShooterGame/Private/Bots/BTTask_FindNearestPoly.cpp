// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "Bots/BTTask_FindNearestPoly.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"
#include "Bots/ShooterAIController.h"
#include "Bots/ShooterBot.h"
#include "Public/Navigation/MyRecastNavMesh.h"

UBTTask_FindNearestPoly::UBTTask_FindNearestPoly(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

EBTNodeResult::Type UBTTask_FindNearestPoly::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AShooterAIController* MyController = Cast<AShooterAIController>(OwnerComp.GetAIOwner());
	AShooterBot* MyBot = MyController ? Cast<AShooterBot>(MyController->GetPawn()) : NULL;
	if (MyBot == NULL)
	{
		return EBTNodeResult::Failed;
	}

	AShooterGameMode* GameMode = MyBot->GetWorld()->GetAuthGameMode<AShooterGameMode>();
	if (GameMode == NULL)
	{
		return EBTNodeResult::Failed;
	}

	const FNavAgentProperties* AgentProperties = MyBot->GetNavAgentProperties();

	if (!AgentProperties)
	{
		return EBTNodeResult::Failed;
	}

	UNavigationSystem* NavSys = GetWorld()->GetNavigationSystem();
	if (!NavSys) {
		return EBTNodeResult::Failed;
	}

	//FNavigationSystem::ECreateIfEmpty CreateNewIfNoneFound;
	const ANavigationData* NavData = NavSys->GetMainNavData(FNavigationSystem::DontCreate);

	//const ANavigationData* NavData = (AgentProperties != NULL) ? GetNavDataForProps(*AgentProperties) : GetMainNavData(FNavigationSystem::DontCreate);
	const ARecastNavMesh* NavMesh = Cast<ARecastNavMesh>(NavData);
	const AMyRecastNavMesh* MyNavMesh = Cast<AMyRecastNavMesh>(NavMesh);

	//MyNavMesh->UpdateNavMesh();
	//OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Vector>(BlackboardKey.GetSelectedKeyID(), true);
	return EBTNodeResult::Succeeded;
}

/*
EBTNodeResult::Type UBTTask_FindNearestPoly::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AShooterAIController* MyController = Cast<AShooterAIController>(OwnerComp.GetAIOwner());
	AShooterBot* MyBot = MyController ? Cast<AShooterBot>(MyController->GetPawn()) : NULL;
	if (MyBot == NULL)
	{
		return EBTNodeResult::Failed;
	}

	AShooterGameMode* GameMode = MyBot->GetWorld()->GetAuthGameMode<AShooterGameMode>();
	if (GameMode == NULL)
	{
		return EBTNodeResult::Failed;
	}

	const FVector MyLoc = MyBot->GetActorLocation();
	
	const FNavAgentProperties* AgentProperties = MyBot->GetNavAgentProperties();

	if (!AgentProperties)
	{
		return EBTNodeResult::Failed;
	}

	UNavigationSystem* NavSys = GetWorld()->GetNavigationSystem();
	if (!NavSys) {
		return EBTNodeResult::Failed;
	}
	
	FNavigationSystem::ECreateIfEmpty CreateNewIfNoneFound;
	const ANavigationData* NavData = NavSys->GetMainNavData(CreateNewIfNoneFound);
	
	//const ANavigationData* NavData = (AgentProperties != NULL) ? GetNavDataForProps(*AgentProperties) : GetMainNavData(FNavigationSystem::DontCreate);
	const ARecastNavMesh* NavMesh = Cast<ARecastNavMesh>(NavData);

	if (!NavMesh)
	{
		return EBTNodeResult::Failed;
	}

	NavNodeRef NearestPoly = NavMesh->FindNearestPoly(MyLoc, FVector(10.0f, 10.0f, 10.f), NavData->GetDefaultQueryFilter());

	if (!NearestPoly)
	{
		return EBTNodeResult::Failed;
	}
	
	//DrawDebugPoint(GetWorld(), MyLoc, 10.0, FColor(255, 0, 0), 10.0, 0.03);

	return EBTNodeResult::Succeeded;
	

}
*/
/*
EBTNodeResult::Type UBTTask_FindNearestPoly::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	
	AShooterAIController* MyController = Cast<AShooterAIController>(OwnerComp.GetAIOwner());
	AShooterBot* MyBot = MyController ? Cast<AShooterBot>(MyController->GetPawn()) : NULL;
	if (MyBot == NULL)
	{
		return EBTNodeResult::Failed;
	}

	AShooterGameMode* GameMode = MyBot->GetWorld()->GetAuthGameMode<AShooterGameMode>();
	if (GameMode == NULL)
	{
		return EBTNodeResult::Failed;
	}

	const FVector MyLoc = MyBot->GetActorLocation();
	const FNavAgentProperties* AgentProperties = MyBot->GetNavAgentProperties();

	if (!AgentProperties)
	{
	return EBTNodeResult::Failed;
	}

	UNavigationSystem* NavSys = GetWorld()->GetNavigationSystem();
	if (!NavSys) {
	return EBTNodeResult::Failed;
	}

	FNavigationSystem::ECreateIfEmpty CreateNewIfNoneFound;
	const ANavigationData* NavData = NavSys->GetMainNavData(CreateNewIfNoneFound);

	//const ANavigationData* NavData = (AgentProperties != NULL) ? GetNavDataForProps(*AgentProperties) : GetMainNavData(FNavigationSystem::DontCreate);
	const ARecastNavMesh* NavMesh = Cast<ARecastNavMesh>(NavData);

	if (!NavMesh)
	{
	return EBTNodeResult::Failed;
	}

	NavNodeRef NearestPoly = NavMesh->FindNearestPoly(MyLoc, FVector(10.0f, 10.0f, 10.f), NavData->GetDefaultQueryFilter());

	if (!NearestPoly)
	{
	return EBTNodeResult::Failed;
	}

	//DrawDebugPoint(GetWorld(), MyLoc, 10.0, FColor(255, 0, 0), 10.0, 0.03);

	return EBTNodeResult::Succeeded;
	


}
*/