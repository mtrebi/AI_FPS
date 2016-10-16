// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once
#include "AIController.h"
#include "Perception/AISense.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Public/Navigation/MyRecastNavMesh.h"
#include "Public/Others/HelperMethods.h"
#include "Public/Others/AttackPositions.h"
#include "Public/Others/SearchLocations.h"
#include "Public/Navigation/MyInfluenceMap.h"
#include "ShooterAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class State : uint8
{
	VE_Idle 	UMETA(DisplayName = "Idle"),
	VE_Patrol 	UMETA(DisplayName = "Patrol"),
	VE_Search	UMETA(DisplayName = "Search"),
	VE_Fight	UMETA(DisplayName = "Fight")
};


UCLASS(config=Game)
class AShooterAIController : public AAIController
{
	GENERATED_UCLASS_BODY()

private:
	//----------------------------------------------------------------------//
	// Behavior Tree
	//----------------------------------------------------------------------//

	UPROPERTY(transient)
	UBlackboardComponent* BlackboardComp;
	UPROPERTY(transient)
	UBehaviorTreeComponent* BehaviorComp;

	//----------------------------------------------------------------------//
	// AI Perception
	//----------------------------------------------------------------------//

	UPROPERTY(transient)
	UAIPerceptionComponent* AIPerceptionComp;
	UPROPERTY(transient)
	UAISenseConfig_Sight* AIPerceptionSightConfig;
	UPROPERTY(transient)
	UAISenseConfig_Hearing* AIPerceptionHearingConfig;

	virtual void GetActorEyesViewPoint(FVector & OutLocation, FRotator & OutRotation) const override;
//----------------------------------------------------------------------//
// A Shooter Game
//----------------------------------------------------------------------//

public:
	// Begin AController interface
	virtual void GameHasEnded(class AActor* EndGameFocus = NULL, bool bIsWinner = false) override;
	virtual void Possess(class APawn* InPawn) override;
	virtual void BeginInactiveState() override;
	// End APlayerController interface

	void Respawn();

	void CheckAmmo(const class AShooterWeapon* CurrentWeapon);

	bool HasWeaponLOSToEnemy(AActor* InEnemyActor, const bool bAnyEnemy) const;

	// Begin AAIController interface
	/** Update direction AI is looking based on FocalPoint */
	virtual void UpdateControlRotation(float DeltaTime, bool bUpdatePawn = true) override;
	// End AAIController interface
protected:
	// Check of we have LOS to a character
	bool LOSTrace(AShooterCharacter* InEnemyChar) const;

	int32 EnemyKeyID;
	int32 NeedAmmoKeyID;

	/** Handle for efficient management of Respawn timer */
	FTimerHandle TimerHandle_Respawn;

public:
	/** Returns BlackboardComp subobject **/
	FORCEINLINE UBlackboardComponent* GetBlackboardComp() const { return BlackboardComp; }
	/** Returns BehaviorComp subobject **/
	FORCEINLINE UBehaviorTreeComponent* GetBehaviorComp() const { return BehaviorComp; }

//----------------------------------------------------------------------//
// Artificial Intelligence @ Mariano Trebino
//----------------------------------------------------------------------//
private:
	// Influence map config
	const FString IM_IMAGE_PATH = "/Game/Environment/Images/navmap_green_31";
	const float IM_UPDATE_FREQ = 0.5;
	const float IM_MOMENTUM = 0.6;
	const float IM_DECAY = 0.0001;

	// Temp variables
	int CurrentPatrolPointIndex = 0;
	bool NeverSawPlayer = true;
	
	float Temp_LookAroundTimer = 0;
	FVector Temp_PlayerLastLocation;
	bool Temp_LookAroundRight = false;

	// Health Updates
	float Health_lastValue = 0;
	float Health_timer = 0;

public:
	/************************* GENERAL **************************/

	State GetAI_State() const;
	void SetAI_State(const State AI_State);
	

	/************************* PATROL **************************/

	UFUNCTION(BlueprintCallable, Category = "Patrol")
	void SetAI_pNextLocation();

	/************************* SEARCH **************************/

	TMap<FString, FVector> * GetAI_SearchLocations();
	void SetAI_SearchLocations(ASearchLocations * SearchLocations);

	AMyInfluenceMap * GetAI_PredictionMap();
	void SetAI_PredictionMap(AMyInfluenceMap * PredictionMap);

	UFUNCTION(BlueprintCallable, Category = "Search")
	void SetAI_NextSearchLocation(const FVector SearchLocation);

	UFUNCTION(BlueprintCallable, Category = "Search")
	void LookAround(const float RotationAngle, const float RotationTime);

	/************************* FIGHT **************************/

	UFUNCTION(BlueprintCallable, Category = "Fight")
	APawn* GetPL_fPlayer() const;
	void SetPL_fPlayer(APawn* Player);

	bool GetAI_fTakingDamage() const;
	void SetAI_fTakingDamage(const bool TakingDamage);

	FVector GetAI_fNextCoverLocation() const;
	UFUNCTION(BlueprintCallable, Category = "Fight")
	void SetAI_fNextCoverLocation(const FVector NextCoverLocation);

	bool GetAI_fNextCoverLocationIsSafe() const;
	void SetAI_fNextCoverLocationIsSafe(const bool IsSafe);

	void SetAI_fCurrentLocationIsSafe(const bool CurrentLocationIsSafe);

	UFUNCTION(BlueprintCallable, Category = "Fight")
	bool GetAI_fNeedReloadNow() const;
	void SetAI_fNeedReloadNow(const bool NeedReloadNow);

	UFUNCTION(BlueprintCallable, Category = "Fight")
	bool GetAI_fNeedReloadSoon() const;
	void SetAI_fNeedReloadSoon(const bool NeedReloadSoon);

	bool GetPL_fIamVisible() const;
	void SetPL_fIamVisible(const bool PlayerCanSeeMe);

	UFUNCTION(BlueprintCallable, Category = "Fight")
	bool GetPL_fIsVisible() const;
	void SetPL_fIsVisible(const bool IsVisible);

	bool GetPL_fLost() const;
	void SetPL_fLost(const bool PlayerLost);

	UFUNCTION(BlueprintCallable, Category = "Fight")
	FVector GetPL_fLocation();
	void SetPL_fLocation(const FVector LastLocation);

	FVector GetPL_fForwardVector();
	void SetPL_fForwardVector(const FVector ForwardVector);

	FVector GetAI_fNextAttackLocation() const;
	UFUNCTION(BlueprintCallable, Category = "Fight")
	void SetAI_fNextAttackLocation(const FVector AttackLocation);

	void SetAI_fNextAttackLocationIsOK(const bool NextLocationIsAttack);
	void SetAI_fCurrentLocationIsAttack(const bool CurrentLocationIsAttack);

	TMap<FString, FVector> * GetAI_fAttackLocations();
	void SetAI_fAttackLocations(AAttackPositions * AttackPositions);

	UFUNCTION(BlueprintCallable, Category = "Fight")
	bool IsLineOfSightClearOfBots() const;

	void SetAI_fIsClose(const bool PlayerIsClose);


	/************************* WEAPON **************************/

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StartWeaponFire();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StopWeaponFire();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void ReloadWeapon();

	/************************* RUNTIME UPDATES **************************/
	UFUNCTION(BlueprintCallable, Category = "Update")
	void OnPerceptionUpdated(TArray<AActor*> testActors);
	
	UFUNCTION(BlueprintCallable, Category = "Update")
	void UpdateData(const float DeltaSeconds);

	void UpdateOwnData(const float DeltaSeconds);
	void UpdatePlayerRelatedData(const float DeltaSeconds);
	void UpdateEnvironmentRelatedData(const float DeltaSeconds);

	void UpdateMyVisibility();
	void UpdateBotState();
	void UpdateHealthSituation(const float DeltaSeconds);
	void UpdateWeaponStats();

	void UpdatePlayerVisibility();
	void UpdatePlayerIsClose();
	void UpdatePlayerLocationAndRotation(const float DeltaSeconds);

	void UpdateTacticalCoverSituation();
	void UpdateTacticalAttackSituation();

private:
	bool PositionIsSafeCover(const FVector CoverPosition, const FVector PlayerPosition) const;
	bool PositionIsGoodAttack(const FVector AttackPosition, const FVector PlayerPosition) const;
};