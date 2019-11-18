// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ReplicationGraph.h"
#include "ShooterReplicationGraph.generated.h"

class AShooterCharacter;
class AShooterWeapon;
class UReplicationGraphNode_GridSpatialization2D;
class AGameplayDebuggerCategoryReplicator;

DECLARE_LOG_CATEGORY_EXTERN( LogShooterReplicationGraph, Display, All );

// This is the main enum we use to route actors to the right replication node. Each class maps to one enum.
UENUM()
enum class EClassRepNodeMapping : uint32
{
	NotRouted,						// Doesn't map to any node. Used for special case actors that handled by special case nodes (UShooterReplicationGraphNode_PlayerStateFrequencyLimiter)
	RelevantAllConnections,			// Routes to an AlwaysRelevantNode or AlwaysRelevantStreamingLevelNode node
	
	// ONLY SPATIALIZED Enums below here! See UShooterReplicationGraph::IsSpatialized

	Spatialize_Static,				// Routes to GridNode: these actors don't move and don't need to be updated every frame.
	Spatialize_Dynamic,				// Routes to GridNode: these actors mode frequently and are updated once per frame.
	Spatialize_Dormancy,			// Routes to GridNode: While dormant we treat as static. When flushed/not dormant dynamic. Note this is for things that "move while not dormant".
};

/** ShooterGame Replication Graph implementation. See additional notes in ShooterReplicationGraph.cpp! */
UCLASS(transient, config=Engine)
class UShooterReplicationGraph :public UReplicationGraph
{
	GENERATED_BODY()

public:

	UShooterReplicationGraph();

	virtual void ResetGameWorldState() override;

	virtual void InitGlobalActorClassSettings() override;
	virtual void InitGlobalGraphNodes() override;
	virtual void InitConnectionGraphNodes(UNetReplicationGraphConnection* RepGraphConnection) override;
	virtual void RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo) override;
	virtual void RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo) override;
	
	UPROPERTY()
	TArray<UClass*>	SpatializedClasses;

	UPROPERTY()
	TArray<UClass*> NonSpatializedChildClasses;

	UPROPERTY()
	TArray<UClass*>	AlwaysRelevantClasses;
	
	UPROPERTY()
	UReplicationGraphNode_GridSpatialization2D* GridNode;

	UPROPERTY()
	UReplicationGraphNode_ActorList* AlwaysRelevantNode;

	TMap<FName, FActorRepListRefView> AlwaysRelevantStreamingLevelActors;

	void OnCharacterEquipWeapon(AShooterCharacter* Character, AShooterWeapon* NewWeapon);
	void OnCharacterUnEquipWeapon(AShooterCharacter* Character, AShooterWeapon* OldWeapon);

#if WITH_GAMEPLAY_DEBUGGER
	void OnGameplayDebuggerOwnerChange(AGameplayDebuggerCategoryReplicator* Debugger, APlayerController* OldOwner);
#endif

	void PrintRepNodePolicies();

private:

	EClassRepNodeMapping GetMappingPolicy(UClass* Class);

	bool IsSpatialized(EClassRepNodeMapping Mapping) const { return Mapping >= EClassRepNodeMapping::Spatialize_Static; }

	TClassMap<EClassRepNodeMapping> ClassRepNodePolicies;
};

UCLASS()
class UShooterReplicationGraphNode_AlwaysRelevant_ForConnection : public UReplicationGraphNode
{
	GENERATED_BODY()

public:

	virtual void NotifyAddNetworkActor(const FNewReplicatedActorInfo& Actor) override { }
	virtual bool NotifyRemoveNetworkActor(const FNewReplicatedActorInfo& ActorInfo, bool bWarnIfNotFound=true) override { return false; }
	virtual void NotifyResetAllNetworkActors() override { }

	virtual void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params) override;

	virtual void LogNode(FReplicationGraphDebugInfo& DebugInfo, const FString& NodeName) const override;

	void OnClientLevelVisibilityAdd(FName LevelName, UWorld* StreamingWorld);
	void OnClientLevelVisibilityRemove(FName LevelName);

	void ResetGameWorldState();

#if WITH_GAMEPLAY_DEBUGGER
	AGameplayDebuggerCategoryReplicator* GameplayDebugger = nullptr;
#endif

private:

	TArray<FName, TInlineAllocator<64> > AlwaysRelevantStreamingLevelsNeedingReplication;

	FActorRepListRefView ReplicationActorList;

	UPROPERTY()
	AActor* LastPawn = nullptr;

	bool bInitializedPlayerState = false;
};

/** This is a specialized node for handling PlayerState replication in a frequency limited fashion. It tracks all player states but only returns a subset of them to the replication driver each frame. */
UCLASS()
class UShooterReplicationGraphNode_PlayerStateFrequencyLimiter : public UReplicationGraphNode
{
	GENERATED_BODY()

	UShooterReplicationGraphNode_PlayerStateFrequencyLimiter();

	virtual void NotifyAddNetworkActor(const FNewReplicatedActorInfo& Actor) override { }
	virtual bool NotifyRemoveNetworkActor(const FNewReplicatedActorInfo& ActorInfo, bool bWarnIfNotFound=true) override { return false; }

	virtual void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params) override;

	virtual void PrepareForReplication() override;

	virtual void LogNode(FReplicationGraphDebugInfo& DebugInfo, const FString& NodeName) const override;

	/** How many actors we want to return to the replication driver per frame. Will not suppress ForceNetUpdate. */
	int32 TargetActorsPerFrame = 2;

private:
	
	TArray<FActorRepListRefView> ReplicationActorLists;
	FActorRepListRefView ForceNetUpdateReplicationActorList;
};