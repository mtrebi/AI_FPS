// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "Bots/ShooterAIController.h"
#include "Bots/ShooterBot.h"
#include "Online/ShooterPlayerState.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "Weapons/ShooterWeapon.h"
#include "Navigation/CrowdFollowingComponent.h"
#include "Public/Others/HelperMethods.h"
#include "Public/EQS/CoverBaseClass.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionListenerInterface.h"
#include "Public/Navigation/MyRecastNavMesh.h"

AShooterAIController::AShooterAIController(const FObjectInitializer& ObjectInitializer) 
	//: Super(ObjectInitializer) {
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(TEXT("PathFollowingComponent"))) {

 	BlackboardComp = ObjectInitializer.CreateDefaultSubobject<UBlackboardComponent>(this, TEXT("BlackBoardComp"));
 	
	BrainComponent = BehaviorComp = ObjectInitializer.CreateDefaultSubobject<UBehaviorTreeComponent>(this, TEXT("BehaviorComp"));	
	
	// Setup AIPerception component
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception Component"));
	
	AIPerceptionSightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));
	// Configure Sight perception component
	AIPerceptionSightConfig->SightRadius = 4500;
	AIPerceptionSightConfig->LoseSightRadius = 4500;
	AIPerceptionSightConfig->PeripheralVisionAngleDegrees = 80.0;
	AIPerceptionSightConfig->DetectionByAffiliation.bDetectEnemies = true;
	AIPerceptionSightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	AIPerceptionSightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	AIPerceptionComp->ConfigureSense(*AIPerceptionSightConfig);

	// Configure Hearing perception component

	AIPerceptionHearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("Hearing Config"));
	AIPerceptionHearingConfig->HearingRange = 4500;
	AIPerceptionHearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	AIPerceptionHearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	AIPerceptionHearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
	AIPerceptionComp->ConfigureSense(*AIPerceptionHearingConfig);
	
	AIPerceptionComp->OnPerceptionUpdated.AddDynamic(this, &AShooterAIController::OnPerceptionUpdated);

	bWantsPlayerState = true;	
}


void AShooterAIController::Possess(APawn* InPawn)
{
	Super::Possess(InPawn);
	AShooterBot* Bot = Cast<AShooterBot>(InPawn);

	UAIPerceptionSystem::RegisterPerceptionStimuliSource(this, UAISense_Sight::StaticClass(), InPawn);
	UAIPerceptionSystem::RegisterPerceptionStimuliSource(this, UAISense_Hearing::StaticClass(), InPawn);

	// Start behavior tree
	if (Bot && Bot->BotBehavior)
	{
		if (Bot->BotBehavior->BlackboardAsset)
		{
			BlackboardComp->InitializeBlackboard(*Bot->BotBehavior->BlackboardAsset);
		}
		EnemyKeyID = BlackboardComp->GetKeyID("Enemy");
		NeedAmmoKeyID = BlackboardComp->GetKeyID("NeedAmmo");

		// Start behavior tree
		BehaviorComp->StartTree(*(Bot->BotBehavior));
		
		AAttackPositions * AttackLocations = (AAttackPositions*) GetWorld()->SpawnActor(AAttackPositions::StaticClass());
		ASearchLocations * SearchLocations = (ASearchLocations*) GetWorld()->SpawnActor(ASearchLocations::StaticClass());
		
		SetAI_fAttackLocations(AttackLocations);
		SetAI_SearchLocations(SearchLocations);
	}

}

void AShooterAIController::BeginInactiveState()
{
	Super::BeginInactiveState();

	AGameState* GameState = GetWorld()->GameState;

	const float MinRespawnDelay = (GameState && GameState->GameModeClass) ? GetDefault<AGameMode>(GameState->GameModeClass)->MinRespawnDelay : 1.0f;

	GetWorldTimerManager().SetTimer(TimerHandle_Respawn, this, &AShooterAIController::Respawn, MinRespawnDelay);
}

void AShooterAIController::Respawn()
{
	GetWorld()->GetAuthGameMode()->RestartPlayer(this);
}

bool AShooterAIController::HasWeaponLOSToEnemy(AActor* InEnemyActor, const bool bAnyEnemy) const
{
	static FName LosTag = FName(TEXT("AIWeaponLosTrace"));
	
	AShooterBot* MyBot = Cast<AShooterBot>(GetPawn());

	bool bHasLOS = false;
	// Perform trace to retrieve hit info
	FCollisionQueryParams TraceParams(LosTag, true, GetPawn());
	TraceParams.bTraceAsyncScene = true;

	TraceParams.bReturnPhysicalMaterial = true;	
	FVector StartLocation = MyBot->GetActorLocation();	
	StartLocation.Z += GetPawn()->BaseEyeHeight; //look from eyes
	
	FHitResult Hit(ForceInit);
	const FVector EndLocation = InEnemyActor->GetActorLocation();
	GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, COLLISION_WEAPON, TraceParams);
	if (Hit.bBlockingHit == true)
	{
		// Theres a blocking hit - check if its our enemy actor
		AActor* HitActor = Hit.GetActor();
		if (Hit.GetActor() != NULL)
		{
			if (HitActor == InEnemyActor)
			{
				bHasLOS = true;
			}
			else if (bAnyEnemy == true)
			{
				// Its not our actor, maybe its still an enemy ?
				ACharacter* HitChar = Cast<ACharacter>(HitActor);
				if (HitChar != NULL)
				{
					AShooterPlayerState* HitPlayerState = Cast<AShooterPlayerState>(HitChar->PlayerState);
					AShooterPlayerState* MyPlayerState = Cast<AShooterPlayerState>(PlayerState);
					if ((HitPlayerState != NULL) && (MyPlayerState != NULL))
					{
						if (HitPlayerState->GetTeamNum() != MyPlayerState->GetTeamNum())
						{
							bHasLOS = true;
						}
					}
				}
			}
		}
	}
	return bHasLOS;
}

void AShooterAIController::CheckAmmo(const class AShooterWeapon* CurrentWeapon)
{
	if (CurrentWeapon && BlackboardComp)
	{
		const int32 Ammo = CurrentWeapon->GetCurrentAmmo();
		const int32 MaxAmmo = CurrentWeapon->GetMaxAmmo();
		const float Ratio = (float) Ammo / (float) MaxAmmo;

		BlackboardComp->SetValue<UBlackboardKeyType_Bool>(NeedAmmoKeyID, (Ratio <= 0.1f));
	}
}

void AShooterAIController::UpdateControlRotation(float DeltaTime, bool bUpdatePawn)
{
	// Look toward focus
	FVector FocalPoint = GetFocalPoint();
	if( !FocalPoint.IsZero() && GetPawn())
	{
		FVector Direction = FocalPoint - GetPawn()->GetActorLocation();
		FRotator NewControlRotation = Direction.Rotation();
		
		NewControlRotation.Yaw = FRotator::ClampAxis(NewControlRotation.Yaw);

		SetControlRotation(NewControlRotation);

		APawn* const P = GetPawn();
		if (P && bUpdatePawn)
		{
			P->FaceRotation(NewControlRotation, DeltaTime);
		}
		
	}
}

void AShooterAIController::GetActorEyesViewPoint(FVector & OutLocation, FRotator & OutRotation) const
{
	if (this->GetPawn()) {
		OutLocation = FVector(this->GetPawn()->GetActorLocation().X, this->GetPawn()->GetActorLocation().Y, this->GetPawn()->GetActorLocation().Z + 60);
		OutRotation = this->GetPawn()->GetViewRotation();
	}
}

void AShooterAIController::GameHasEnded(AActor* EndGameFocus, bool bIsWinner)
{
	// Stop the behaviour tree/logic
	BehaviorComp->StopTree();

	// Stop any movement we already have
	StopMovement();

	// Cancel the repsawn timer
	GetWorldTimerManager().ClearTimer(TimerHandle_Respawn);

	// Clear any enemy
	SetPL_fPlayer(NULL);

	// Finally stop firing
	AShooterBot* MyBot = Cast<AShooterBot>(GetPawn());
	AShooterWeapon* MyWeapon = MyBot ? MyBot->GetWeapon() : NULL;
	if (MyWeapon == NULL)
	{
		return;
	}
	MyBot->StopWeaponFire();	
}

//----------------------------------------------------------------------//
// Artificial Intelligence @ Mariano Trebino
//----------------------------------------------------------------------//


/************************* GENERAL **************************/

State AShooterAIController::GetAI_State() const {
	return  (State) BlackboardComp->GetValueAsEnum("AI_gState");
}

void AShooterAIController::SetAI_State(const State BotState) {
	if (GetAI_State() != BotState) {
		BlackboardComp->SetValueAsEnum("AI_gState", (uint8)BotState);
	}
}


/************************* PATROL **************************/

void AShooterAIController::SetAI_pNextLocation() {
	FVector NextPatrolPoint;
	TArray<AActor*> AttachedActors;
	TArray<FVector> MyPatrolPoints;

	AShooterBot* Bot = Cast<AShooterBot>(GetPawn());
	Bot->GetAttachedActors(AttachedActors);

	for (auto ItAttachedActor = AttachedActors.CreateConstIterator(); ItAttachedActor; ++ItAttachedActor) {
		AActor * AttachedActor = *ItAttachedActor;
		UE_LOG(LogTemp, Warning, TEXT("PatrolPoint's Name is %s"), *AttachedActor->GetName());
		if (AttachedActor->GetName().Contains("Patrol")) {
			MyPatrolPoints.Add(AttachedActor->GetActorLocation());
		}
	}

	if (MyPatrolPoints.Num() > 0 && CurrentPatrolPointIndex == MyPatrolPoints.Num()) {
		CurrentPatrolPointIndex = 0;
		NextPatrolPoint = MyPatrolPoints[CurrentPatrolPointIndex];
		++CurrentPatrolPointIndex;
	}
	else {
		NextPatrolPoint = MyPatrolPoints[CurrentPatrolPointIndex];
		++CurrentPatrolPointIndex;
	}
	BlackboardComp->SetValueAsVector("AI_pNextLocation", NextPatrolPoint);

}


/************************* SEARCH **************************/

TMap<FString, FVector> * AShooterAIController::GetAI_SearchLocations() {
	TMap<FString, FVector> * SearchPositions = NULL;

	if (BlackboardComp) {
		UObject * Object = BlackboardComp->GetValueAsObject("AI_sLocations");
		if (Object) {
			SearchPositions = Cast<ASearchLocations>(Object)->SearchMap;
		}
	}
	return SearchPositions;
}

void AShooterAIController::SetAI_SearchLocations(ASearchLocations * SearchLocations) {
	BlackboardComp->SetValueAsObject("AI_sLocations", SearchLocations);
}

void AShooterAIController::SetAI_NextSearchLocation(const FVector SearchLocation) {
	TMap<FString, FVector> * NewSearchPositions = GetAI_SearchLocations();
	APawn * Pawn = this->GetPawn();

	if (!Pawn) {
		return;
	}

	NewSearchPositions->Add(Pawn->GetName(), SearchLocation);
	BlackboardComp->SetValueAsVector("AI_sNextLocation", SearchLocation);
	//SetFocalPoint(SearchLocation, EAIFocusPriority::Default);

}

AMyInfluenceMap * AShooterAIController::GetAI_PredictionMap() {
	return Cast<AMyInfluenceMap>(BlackboardComp->GetValueAsObject("AI_sPredictionMap"));

}

void AShooterAIController::SetAI_PredictionMap(AMyInfluenceMap * PredictionMap) {
	BlackboardComp->SetValueAsObject("AI_sPredictionMap", PredictionMap);
}


void AShooterAIController::LookAround(const float RotationAngle, const float RotationTime) {
	ClearFocus(EAIFocusPriority::Gameplay);
	ClearFocus(EAIFocusPriority::Move);
	ClearFocus(EAIFocusPriority::LastFocusPriority);
	ClearFocus(EAIFocusPriority::Default);

	AActor * actor = GetFocusActor();
	FVector Point = GetFocalPoint();
	APawn * Pawn = this->GetPawn();

	if (!Pawn) {
		return;
	}

	//const FRotator Rotation = Pawn->GetActorRotation() + FRotator(0, RotationAngle, 0);
	//Pawn->FaceRotation(Rotation, RotationTime);
	//Pawn->AddActorLocalRotation(FRotator(0, RotationAngle, 0));
	Pawn->SetActorRelativeRotation(FRotator(0, RotationAngle, 0));
	//this->SetControlRotation(FRotator(0, RotationAngle, 0));
	//this->ClientSetRotation(FRotator(0, RotationAngle, 0));

}

/************************* FIGHT **************************/

APawn* AShooterAIController::GetPL_fPlayer() const {
	return Cast<APawn>(BlackboardComp->GetValueAsObject("PL_fPlayer"));
}

void AShooterAIController::SetPL_fPlayer(APawn* Player) {
	BlackboardComp->SetValueAsObject("PL_fPlayer", Player);
	if (Player != NULL) {
		SetFocus(Player, EAIFocusPriority::Gameplay);
	}
	else {
		ClearFocus(EAIFocusPriority::Gameplay);
		SetFocalPoint(GetPL_fLocation(), EAIFocusPriority::Gameplay);
		SetPL_fIamVisible(false);
	}
}

bool AShooterAIController::GetAI_fTakingDamage() const {
	return BlackboardComp->GetValueAsBool("AI_fTakingDamage");
}

void AShooterAIController::SetAI_fTakingDamage(const bool ReceivingDamage) {
	BlackboardComp->SetValueAsBool("AI_fTakingDamage", ReceivingDamage);
}

FVector AShooterAIController::GetAI_fNextCoverLocation() const {
	return BlackboardComp->GetValueAsVector("AI_fNextCoverLocation");
}

void AShooterAIController::SetAI_fNextCoverLocation(const FVector NextCoverLocation) {
	BlackboardComp->SetValueAsVector("AI_fNextCoverLocation", NextCoverLocation);
}

bool AShooterAIController::GetAI_fNextCoverLocationIsSafe() const {
	return BlackboardComp->GetValueAsBool("AI_fNextCoverLocationIsSafe");
}

void AShooterAIController::SetAI_fNextCoverLocationIsSafe(const bool IsSafe) {
	BlackboardComp->SetValueAsBool("AI_fNextCoverLocationIsSafe", IsSafe);
}

void AShooterAIController::SetAI_fCurrentLocationIsSafe(const bool IsSafe) {
	BlackboardComp->SetValueAsBool("AI_fCurrentLocationIsSafe", IsSafe);
}

bool AShooterAIController::GetAI_fNeedReloadNow() const  {
	return BlackboardComp->GetValueAsBool("AI_fNeedReloadNow");
}

void AShooterAIController::SetAI_fNeedReloadNow(const bool NeedReloadNow) {
	BlackboardComp->SetValueAsBool("AI_fNeedReloadNow", NeedReloadNow);
}

bool AShooterAIController::GetAI_fNeedReloadSoon() const  {
	return BlackboardComp->GetValueAsBool("AI_fNeedReloadSoon");
}

void AShooterAIController::SetAI_fNeedReloadSoon(const bool NeedReloadSoon) {
	BlackboardComp->SetValueAsBool("AI_fNeedReloadSoon", NeedReloadSoon);
}

bool AShooterAIController::GetPL_fIsVisible() const {
	return BlackboardComp->GetValueAsBool("PL_fIsVisible");
}

void AShooterAIController::SetPL_fIsVisible(const bool IsVisible) {
	BlackboardComp->SetValueAsBool("PL_fIsVisible", IsVisible);
}

bool AShooterAIController::GetPL_fLost() const {
	return BlackboardComp->GetValueAsBool("PL_fLost");
}

void AShooterAIController::SetPL_fLost(const bool PlayerLost) {
	BlackboardComp->SetValueAsBool("PL_fLost", PlayerLost);

}

bool AShooterAIController::GetPL_fIamVisible() const {
	return BlackboardComp->GetValueAsBool("PL_fIamVisible");
}

void AShooterAIController::SetPL_fIamVisible(const bool PlayerCanSeeMe) {
	BlackboardComp->SetValueAsBool("PL_fIamVisible", PlayerCanSeeMe);
}

FVector AShooterAIController::GetPL_fLocation() {
	const FVector LastLocation = BlackboardComp->GetValueAsVector("PL_fLocation");
	//SetFocalPoint(LastLocation, EAIFocusPriority::Move);
	return LastLocation;
}

void AShooterAIController::SetPL_fLocation(const FVector LastLocation) {
	NeverSawPlayer = false;
	BlackboardComp->SetValueAsVector("PL_fLocation", LastLocation);
}

FVector AShooterAIController::GetPL_fForwardVector() {
	return BlackboardComp->GetValueAsVector("PL_fForwardVector");;
}

void AShooterAIController::SetPL_fForwardVector(const FVector ForwardVector) {
	BlackboardComp->SetValueAsVector("PL_fForwardVector", ForwardVector);
}


FVector AShooterAIController::GetAI_fNextAttackLocation() const {
	return BlackboardComp->GetValueAsVector("AI_fNextAttackLocation");
}

void AShooterAIController::SetAI_fNextAttackLocation(const FVector AttackLocation) {
	APawn * Pawn = this->GetPawn();
	if (!Pawn) {
		return ;
	}

	NeverSawPlayer = false;
	BlackboardComp->SetValueAsVector("AI_fNextAttackLocation", AttackLocation);

	TMap<FString, FVector> * AttackPositions = GetAI_fAttackLocations();
	AttackPositions->Add(Pawn->GetName(), AttackLocation);

}

void AShooterAIController::SetAI_fNextAttackLocationIsOK(const bool NextLocationIsAttack) {
	BlackboardComp->SetValueAsBool("AI_fNextAttackLocationIsOK", NextLocationIsAttack);
}

void AShooterAIController::SetAI_fCurrentLocationIsAttack(const bool CurrentLocationIsGoodAttack) {
	BlackboardComp->SetValueAsBool("AI_fCurrentLocationIsAttack", CurrentLocationIsGoodAttack);
}

TMap<FString, FVector> * AShooterAIController::GetAI_fAttackLocations() {
	TMap<FString, FVector> * AttackPositions = NULL;

	if (BlackboardComp) {
		UObject * Object = BlackboardComp->GetValueAsObject("AI_fAttackLocations");
		if (Object) {
			AttackPositions = Cast<AAttackPositions>(Object)->AttackMap;
		}
	}
	return AttackPositions;
}

void AShooterAIController::SetAI_fAttackLocations(AAttackPositions * AttackPositions) {
	BlackboardComp->SetValueAsObject("AI_fAttackLocations", AttackPositions);
}

bool AShooterAIController::IsLineOfSightClearOfBots() const {
	FHitResult OutHit;
	FCollisionQueryParams CollisionParams;

	APawn* Pawn = GetPawn();

	if (!Pawn) {
		return true;
	}

	UWorld * World = GetWorld();

	if (!World) {
		return true;
	}

	FVector ViewPoint = Pawn->GetActorLocation();

	if (ViewPoint.IsZero())
	{
		FRotator ViewRotation;
		GetActorEyesViewPoint(ViewPoint, ViewRotation);

		// if we still don't have a view point we simply fail
		if (ViewPoint.IsZero())
		{
			return true;
		}
	}

	static FName NAME_LineOfSight = FName(TEXT("LineOfSight"));
	FVector TargetLocation = GetFocalPoint();

	CollisionParams.AddIgnoredActor(Pawn);

	bool bHit = GetWorld()->LineTraceSingleByChannel(OutHit, ViewPoint, TargetLocation, COLLISION_WEAPON, CollisionParams);
	if (bHit && OutHit.Actor->GetName().Contains("Bot"))
	{
		return false;
	}
	//AActor * actor = OutHit.GetActor();
	//UE_LOG(LogTemp, Warning, TEXT("MyCharacter's Name is %s"), *actor->GetName());

	return true;
}


void AShooterAIController::SetAI_fIsClose(const bool PlayerIsClose) {
	BlackboardComp->SetValueAsBool("AI_fIsClose", PlayerIsClose);
}


/************************* WEAPON **************************/

void AShooterAIController::StartWeaponFire()
{
	AShooterBot* MyBot = Cast<AShooterBot>(GetPawn());
	AShooterWeapon* MyWeapon = MyBot ? MyBot->GetWeapon() : NULL;
	if (MyWeapon == NULL)
	{
		return;
	}

	bool bCanShoot = false;
	AShooterCharacter* Player = Cast<AShooterCharacter>(GetPL_fPlayer());
	//if (Player && (Player->IsAlive()) && (MyWeapon->GetCurrentAmmo() > 0) && (MyWeapon->CanFire() == true))
	if ((MyWeapon->GetCurrentAmmo() > 0) && (MyWeapon->CanFire() == true))
	{
		if (IsLineOfSightClearOfBots()) {
			bCanShoot = true;
		}
		//if (LineOfSightTo(Player, MyBot->GetActorLocation()))
		//{
		//	bCanShoot = true;
		//}
	}

	if (bCanShoot)
	{
		MyBot->StartWeaponFire();
	}
	else
	{
		MyBot->StopWeaponFire();
	}
}

void AShooterAIController::StopWeaponFire()
{
	AShooterBot* MyBot = Cast<AShooterBot>(GetPawn());
	AShooterWeapon* MyWeapon = MyBot ? MyBot->GetWeapon() : NULL;
	if (MyWeapon == NULL)
	{
		return;
	}

	MyBot->StopWeaponFire();
}

void AShooterAIController::ReloadWeapon()
{
	AShooterBot* MyBot = Cast<AShooterBot>(GetPawn());
	AShooterWeapon* MyWeapon = MyBot ? MyBot->GetWeapon() : NULL;
	if (MyWeapon == NULL)
	{
		return;
	}
	MyWeapon->StartReload();
}


/************************* RUNTIME UPDATES **************************/

void AShooterAIController::OnPerceptionUpdated(TArray<AActor*> updatedActors){
	APawn* Player = NULL;

	UWorld * World = GetWorld();
	if (World) {
		Player = UGameplayStatics::GetPlayerPawn(World, 0);
	}

	if (!Player) {
		return;
	}
	FVector PlayerLocation = Player->GetActorLocation();

	if (Player) {
		for (int32 i = 0; i < updatedActors.Num(); ++i) {
			if (Player == updatedActors[i]) {
				// Player has been seen
				if (!GetPL_fPlayer()) {
					SetPL_fPlayer(Player);
				}
				else {
					SetPL_fPlayer(NULL);
				}
				break;
			}
			else {
				if (updatedActors[i]) {
					AActor * Parent = updatedActors[i]->GetAttachParentActor();
					if ((Parent && Cast<AShooterCharacter>(Parent) && !Cast<AShooterBot>(Parent))) {
						// Player has been heard
						SetPL_fLocation(Player->GetActorLocation());
					}
				}
			}
		}
	}
}

void AShooterAIController::UpdateData(const float DeltaSeconds) {
	UpdateOwnData(DeltaSeconds);
	UpdatePlayerRelatedData(DeltaSeconds);
	UpdateEnvironmentRelatedData(DeltaSeconds);
}

void AShooterAIController::UpdateOwnData(const float DeltaSeconds) {
	UpdateMyVisibility();
	UpdateBotState();
	UpdateHealthSituation(DeltaSeconds);
	UpdateWeaponStats();
}

void AShooterAIController::UpdatePlayerRelatedData(const float DeltaSeconds) {
	UpdatePlayerVisibility();
	UpdatePlayerIsClose();
	UpdatePlayerLocationAndRotation(DeltaSeconds);

}

void AShooterAIController::UpdateEnvironmentRelatedData(const float DeltaSeconds) {
	UpdateTacticalCoverSituation();
	UpdateTacticalAttackSituation();
}

void AShooterAIController::UpdateBotState() {
	const AShooterBot* Bot = Cast<AShooterBot>(GetPawn());
	
	if (GetPL_fPlayer() || !NeverSawPlayer) {
		// Knows player or has seen him once at least
		if (GetPL_fPlayer()) {
			AShooterCharacter * PlayerCharacter = Cast<AShooterCharacter>(GetPL_fPlayer());
			if (PlayerCharacter->IsAlive()) {
				// Knows player and is alive
				SetAI_State(State::VE_Fight);
			}
			else {
				// Knows player and is dead
				SetAI_State(State::VE_Idle);
			}
		}
		else if (!NeverSawPlayer){
			if (this->GetAI_State() == State::VE_Patrol || (GetAI_State() == State::VE_Fight && GetPL_fLost())) {
				ClearFocus(EAIFocusPriority::Gameplay);
				SetAI_State(State::VE_Search);
			}
		}
	}
	else if (NeverSawPlayer){
		SetAI_State(State::VE_Patrol);
	}
}

void AShooterAIController::UpdatePlayerIsClose() {
	AShooterBot* Bot = Cast<AShooterBot>(GetPawn());
	bool PlayerIsClose = false;
	if (Bot) {
		const float PlayerDistance = FVector::Dist(Bot->GetActorLocation(), GetPL_fLocation());
		PlayerIsClose = (PlayerDistance < HelperMethods::MIN_DIST_PLAYER) ? true : false;
	}
	SetAI_fIsClose(PlayerIsClose);
}

void AShooterAIController::UpdatePlayerLocationAndRotation(const float DeltaSeconds) {
	const APawn * PlayerPawn = GetPL_fPlayer();
	const AShooterBot* Bot = Cast<AShooterBot>(GetPawn());
	if (Bot && PlayerPawn) {
		SetPL_fLocation(PlayerPawn->GetActorLocation());
		SetPL_fForwardVector(PlayerPawn->GetActorForwardVector());
		AMyInfluenceMap * OldInfluenceMap = this->GetAI_PredictionMap();
		if (OldInfluenceMap) {
			// We no longer need the map cuz we now exactly where is the player
			GetWorld()->DestroyActor(Cast<AActor>(OldInfluenceMap));
		}
	}
	else {
		if (Temp_PlayerLastLocation != GetPL_fLocation()){
			// We have new information, se lets create a new Map
			if (GetAI_PredictionMap()) {
				GetWorld()->DestroyActor(Cast<AActor>(GetAI_PredictionMap()));
			}

			AMyInfluenceMap * NewInfluenceMap = GetWorld()->SpawnActor<AMyInfluenceMap>();
			NewInfluenceMap->CreateInfluenceMap(IM_MOMENTUM, IM_DECAY, IM_UPDATE_FREQ, IM_IMAGE_PATH + "_base", IM_IMAGE_PATH);
			this->SetAI_PredictionMap(NewInfluenceMap);
			NewInfluenceMap->SetInfluence(GetPL_fLocation(), 255.0f);
		}

		AMyInfluenceMap * MyInfluenceMap = this->GetAI_PredictionMap();
		if (MyInfluenceMap) {
			MyInfluenceMap->SetBotVisibility(Bot->GetName(),HelperMethods::CalculateVisibility(GetWorld(), GetPawn()->GetActorLocation(), GetPawn()->GetActorForwardVector()));
		}

		//SetPL_fForwardVector(FVector(2, 2, 2));
		Temp_PlayerLastLocation = GetPL_fLocation();
	}
	/*
	if (GetAI_PredictionMap()) {
		AMyInfluenceMap * InfluenceMap = this->GetAI_PredictionMap();

		const FVector PlayerLocation = HelperMethods::GetPlayerPositionFromAI(GetWorld());
		const FVector PlayerForwardVector = HelperMethods::GetPlayerForwardVectorFromAI(GetWorld());

		const TArray<Triangle> VisibleTriangles = HelperMethods::CalculateVisibility(GetWorld(), PlayerLocation, PlayerForwardVector);


		InfluenceMap->SetBotVisibility(Bot->GetName(), VisibleTriangles);

	}
	else {
		const FVector PlayerLocation = HelperMethods::GetPlayerPositionFromAI(GetWorld());

		AMyInfluenceMap * NewInfluenceMap = GetWorld()->SpawnActor<AMyInfluenceMap>();
		NewInfluenceMap->CreateInfluenceMap(IM_MOMENTUM, IM_DECAY, IM_UPDATE_FREQ, IM_IMAGE_PATH + "_base", IM_IMAGE_PATH);

		this->SetAI_PredictionMap(NewInfluenceMap);
		NewInfluenceMap->SetInfluence(Bot->GetActorLocation(), 255.0f);
	}

	*/
}

void AShooterAIController::UpdateMyVisibility() {

	APawn * PlayerPawn = GetPL_fPlayer();
	AShooterBot* BotPawn = Cast<AShooterBot>(GetPawn());
	UWorld * World = GetWorld();
	FHitResult OutHit;
	FCollisionQueryParams CollisionParams;

	if (PlayerPawn && BotPawn && World) {
		const FVector PlayerLocation = PlayerPawn->GetActorLocation();
		const FVector UpPlayerLocation = FVector(PlayerLocation.X, PlayerLocation.Y, HelperMethods::EYES_POS_Z);
	
		const FVector MyLocation = BotPawn->GetActorLocation();
		const FVector UpMyLocation = FVector(MyLocation.X, MyLocation.Y, HelperMethods::EYES_POS_Z);
		

		CollisionParams.AddIgnoredActor(BotPawn);

		const bool BlockingHitFound = World->LineTraceSingleByChannel(OutHit, UpMyLocation, UpPlayerLocation, ECollisionChannel::ECC_Visibility, CollisionParams);
		SetPL_fIsVisible(BlockingHitFound && OutHit.Actor->GetName().Contains("Player"));
	}
	else if (BotPawn) {
		State a = GetAI_State();
		if (GetAI_State() == State::VE_Fight ) {
			SetPL_fIsVisible(false);
			TArray<AActor*> ActorsToIgnore;
			const FVector PlayerLastLoc = GetPL_fLocation();
			const FVector PlayerLastLocUp = FVector(PlayerLastLoc.X, PlayerLastLoc.Y, HelperMethods::EYES_POS_Z);

			const FVector MyLocation = BotPawn->GetActorLocation();
			const FVector UpMyLocation = FVector(MyLocation.X, MyLocation.Y, HelperMethods::EYES_POS_Z);


			FCollisionQueryParams CollisionParams;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AShooterCharacter::StaticClass(), ActorsToIgnore);
			CollisionParams.AddIgnoredActors(ActorsToIgnore);
			const bool BlockingHitFound = World->LineTraceSingleByChannel(OutHit, UpMyLocation, PlayerLastLocUp, ECollisionChannel::ECC_Visibility, CollisionParams);
			SetPL_fLost(!BlockingHitFound);
		}
	}
}

void AShooterAIController::UpdatePlayerVisibility() {
	AShooterBot* Bot = Cast<AShooterBot>(GetPawn());
	APawn * PlayerPawn = GetPL_fPlayer();
	if (PlayerPawn) {
		const FVector PlayerLocation = PlayerPawn->GetActorLocation();
		const FVector PlayerForwardVector = PlayerPawn->GetActorForwardVector();

		const TArray<Triangle> VisibleTriangles = HelperMethods::CalculateVisibility(GetWorld(), PlayerLocation, PlayerForwardVector);

		// Update Navigation Mesh
		dtQueryFilter_Example::SetPlayerVisibility(VisibleTriangles);

		// Check if I am Visible
		bool IAmVisible = false;
		FVector PlayerToBot = Bot->GetActorLocation() - PlayerPawn->GetActorLocation();
		PlayerToBot.Normalize();
		const float Angle = FMath::RadiansToDegrees(acosf(PlayerForwardVector.CosineAngle2D(PlayerToBot)));
		if (Angle <= HelperMethods::PLAYER_FOV) {
			IAmVisible = true;
		}

		SetPL_fIamVisible(IAmVisible);
	}else {
		// @todo prediction
		SetPL_fIamVisible(false);
		TArray<Triangle> VisibleTriangles;
		dtQueryFilter_Example::SetPlayerVisibility(VisibleTriangles);
		/*
		TArray<Triangle> VisibleTriangles;
		if (GetAI_State() == State::VE_Fight) {
			const TArray<Triangle> VisibleTriangles1 = HelperMethods::CalculateVisibility(GetWorld(), GetPL_fLocation(), FVector(0, -1, 0));
			const TArray<Triangle> VisibleTriangles2 = HelperMethods::CalculateVisibility(GetWorld(), GetPL_fLocation(), FVector(0, 1, 0));
			const TArray<Triangle> VisibleTriangles3 = HelperMethods::CalculateVisibility(GetWorld(), GetPL_fLocation(), FVector(-1, 0, 0));
			const TArray<Triangle> VisibleTriangles4 = HelperMethods::CalculateVisibility(GetWorld(), GetPL_fLocation(), FVector(1, 0, 0));
			
			VisibleTriangles.Append(VisibleTriangles1);
			VisibleTriangles.Append(VisibleTriangles2);
			VisibleTriangles.Append(VisibleTriangles3);
			VisibleTriangles.Append(VisibleTriangles4);
			dtQueryFilter_Example::SetPlayerVisibility(VisibleTriangles, 1.001);
		}
		else {
			dtQueryFilter_Example::SetPlayerVisibility(VisibleTriangles, 1.1);
		}*/
	}
}

void AShooterAIController::UpdateTacticalCoverSituation() {
	bool NextCoverIsSafe = true;
	bool CurrentCoverIsSafe = true;
	const AShooterBot* Bot = Cast<AShooterBot>(GetPawn());
	if (Bot) {

		const FVector BotLocation = Bot->GetActorLocation();

		if (GetPL_fPlayer()) {
			CurrentCoverIsSafe = GetPL_fIamVisible() ? false:  PositionIsSafeCover(BotLocation, GetPL_fLocation());
			NextCoverIsSafe = PositionIsSafeCover(GetAI_fNextCoverLocation(), GetPL_fLocation());
		}
		else {
			// @todo prediction
			CurrentCoverIsSafe = PositionIsSafeCover(BotLocation, GetPL_fLocation());
			NextCoverIsSafe = PositionIsSafeCover(GetAI_fNextCoverLocation(), GetPL_fLocation());
		}
	}
	SetAI_fCurrentLocationIsSafe(CurrentCoverIsSafe);
	SetAI_fNextCoverLocationIsSafe(NextCoverIsSafe);
}

void AShooterAIController::UpdateTacticalAttackSituation() {
	bool NextAttackIsGood = true;
	bool CurrentAttackIsGood = true;
	const AShooterBot* Bot = Cast<AShooterBot>(GetPawn());
	if (Bot) {
		const FVector BotLocation = Bot->GetActorLocation();
		if (GetPL_fPlayer()) {
			CurrentAttackIsGood = !GetPL_fIsVisible() ? false : PositionIsGoodAttack(BotLocation, GetPL_fLocation());
			NextAttackIsGood = PositionIsGoodAttack(GetAI_fNextAttackLocation(), GetPL_fLocation());
		}
		else {
			// @todo prediction
			CurrentAttackIsGood = PositionIsGoodAttack(BotLocation, GetPL_fLocation());
			NextAttackIsGood = PositionIsGoodAttack(GetAI_fNextAttackLocation(), GetPL_fLocation());
		}
	}

	SetAI_fCurrentLocationIsAttack(CurrentAttackIsGood);
	SetAI_fNextAttackLocationIsOK(NextAttackIsGood);
}

void AShooterAIController::UpdateWeaponStats() {
	const AShooterBot* Bot = Cast<AShooterBot>(GetPawn());
	const AShooterWeapon * Weapon = Bot->GetWeapon();

	// Weapon Checks
	SetAI_fNeedReloadNow((float) (Weapon->GetCurrentAmmoInClip() / (float) Weapon->GetAmmoPerClip()) < 0.10);
	SetAI_fNeedReloadSoon(((float) Weapon->GetCurrentAmmoInClip() / (float) Weapon->GetAmmoPerClip()) < 0.40);
}

void AShooterAIController::UpdateHealthSituation(float DeltaSeconds) {
	const AShooterBot* Bot = Cast<AShooterBot>(GetPawn());

	if (Bot) {
		if (GetAI_fTakingDamage() && Health_timer >= HelperMethods::WAIT_FOR_REGEN + FMath::RandRange(0, 7)) {
			Health_timer = 0;
			SetAI_fTakingDamage(false);
		}
		else if(GetAI_fTakingDamage()) {
			Health_timer += DeltaSeconds;
		}
		else if (Health_lastValue != 0){
			SetAI_fTakingDamage(FMath::Abs(Bot->Health - Health_lastValue) > HelperMethods::RECEIVING_DMG);

		}
		Health_lastValue = Bot->Health;
	}

}

/************************* AUX PRIVATE **************************/

bool AShooterAIController::PositionIsSafeCover(const FVector CoverPosition, const FVector PlayerPosition) const {
	bool PositionIsSafeCover = false;

	if (CoverPosition == FVector(0, 0, 0)) {
		PositionIsSafeCover;
	}

	AShooterBot* MyBot = Cast<AShooterBot>(GetPawn());

	if (!MyBot) {
		return PositionIsSafeCover;
	}

	UWorld * World = GetWorld();
	if (!World) {
		return PositionIsSafeCover;
	}

	// Check Behind obstacle <- Player cant see me
	FHitResult OutHit;
	FCollisionQueryParams CollisionParams;
	TArray<AActor*> ActorsToIgnore;

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AShooterBot::StaticClass(), ActorsToIgnore);
	CollisionParams.AddIgnoredActors(ActorsToIgnore);

	const FVector UpPlayerLocation = FVector(PlayerPosition.X, PlayerPosition.Y, HelperMethods::EYES_POS_Z);
	const FVector UpCoverPosition = FVector(CoverPosition.X, CoverPosition.Y, HelperMethods::EYES_POS_Z);

	const bool BlockingHitFound = World->LineTraceSingleByChannel(OutHit, UpCoverPosition, UpPlayerLocation, ECollisionChannel::ECC_Visibility, CollisionParams);

	if (BlockingHitFound && !OutHit.Actor->GetName().Contains("Player")) {
		// Hit some obstacle
		PositionIsSafeCover = true;
	}

	return PositionIsSafeCover;
}

bool AShooterAIController::PositionIsGoodAttack(const FVector AttackPosition, const FVector PlayerPosition) const {
	bool PositionIsGoodAttack = false;

	if (AttackPosition == FVector(0, 0, 0)) {
		return false;
	}

	AShooterBot* MyBot = Cast<AShooterBot>(GetPawn());

	if (!MyBot) {
		return PositionIsGoodAttack;
	}

	// Check Has Visibility
	UWorld * World = GetWorld();
	if (!World) {
		return PositionIsGoodAttack;
	}
	
	FHitResult OutHit;
	FCollisionQueryParams CollisionParams;
	TArray<AActor*> ActorsToIgnore;

	//const FName TraceTag("VisibilityTrace");
	//World->DebugDrawTraceTag = TraceTag;
	//CollisionParams.TraceTag = TraceTag;

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AShooterBot::StaticClass(), ActorsToIgnore);
	CollisionParams.AddIgnoredActors(ActorsToIgnore);

	const FVector UpPlayerLocation = FVector(PlayerPosition.X, PlayerPosition.Y, HelperMethods::EYES_POS_Z);
	const FVector UpAttackPosition = FVector(AttackPosition.X, AttackPosition.Y, HelperMethods::EYES_POS_Z);
	/*
	const bool BlockingHitFound = World->LineTraceSingleByChannel(OutHit, UpAttackPosition, UpPlayerLocation, ECollisionChannel::ECC_Visibility, CollisionParams);

	if (!BlockingHitFound || !OutHit.Actor->GetName().Contains("Player")) {
		// Hit some obstacle
		return PositionIsGoodAttack;
	}
	*/

	if (!GetPL_fIsVisible()) {
		return PositionIsGoodAttack;
	}
	// Has Cover Position NEAR (Behind obstacle position)
	for (float X = AttackPosition.X - 250; X < AttackPosition.X + 250; X += 10) {
		for (float Y = AttackPosition.Y - 250; Y < AttackPosition.Y + 250; Y += 10) {
			const FVector CoverPosition = FVector(X, Y, HelperMethods::EYES_POS_Z);
			//UNavigationSystem::ProjectPointToNavigation()
			
			if (PositionIsSafeCover(CoverPosition, PlayerPosition)) {
				PositionIsGoodAttack = true;
				break;
			}

		}
	}

	/*

	const float MaxRadius = 500.0f;
	const TArray<FVector> LocationOfCoverAnnotations = HelperMethods::GetLocationOfCoverAnnotationsWithinRadius(World, AttackPosition, MaxRadius);

	for (auto It = LocationOfCoverAnnotations.CreateConstIterator(); It; ++It) {
		const FVector CoverLocation = *It;
		const FVector UpCoverLocation = FVector(CoverLocation.X, CoverLocation.Y, HelperMethods::EYES_POS_Z);

		const bool BlockingHitFound = World->LineTraceSingleByChannel(OutHit, UpCoverLocation, UpPlayerLocation, ECollisionChannel::ECC_Visibility, CollisionParams);
		if (BlockingHitFound && !OutHit.Actor->GetName().Contains("Player")) {
			// Hit some obstacle
			PositionIsGoodAttack = true;
			break;
		}
	}
	*/
	return PositionIsGoodAttack;
}

