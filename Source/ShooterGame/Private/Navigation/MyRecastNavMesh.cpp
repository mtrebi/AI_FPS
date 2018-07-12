// Fill out your copyright notice in the Description page of Project Settings.
#include "ShooterGame.h"

// recast includes
#include "AI/Navigation/NavFilters/NavigationQueryFilter.h"
#include "Runtime/Navmesh/Public/Detour/DetourNavMeshQuery.h"
#include "Runtime/Navmesh/Public/Detour/DetourNavMesh.h"
#include "Runtime/Navmesh/Public/Detour/DetourCommon.h"
#include "Public/Navigation/MyRecastNavMesh.h"
#include "Public/Navigation/CubeComponent.h"

//----------------------------------------------------------------------//
// dtQueryFilter_Example();
//----------------------------------------------------------------------//
TArray<Triangle> dtQueryFilter_Example::PlayerVisibleAreas;

bool dtQueryFilter_Example::SetPlayerVisibility(const TArray<Triangle> VisibleAreas) {
	PlayerVisibleAreas = VisibleAreas;
	return true;;
}

/// Returns cost to move from the beginning to the end of a line segment
/// that is fully contained within a polygon.
///  @param[in]		pa			The start position on the edge of the previous and current polygon. [(x, y, z)]
///  @param[in]		pb			The end position on the edge of the current and next polygon. [(x, y, z)]
///  @param[in]		prevRef		The reference id of the previous polygon. [opt]
///  @param[in]		prevTile	The tile containing the previous polygon. [opt]
///  @param[in]		prevPoly	The previous polygon. [opt]
///  @param[in]		curRef		The reference id of the current polygon.
///  @param[in]		curTile		The tile containing the current polygon.
///  @param[in]		curPoly		The current polygon.
///  @param[in]		nextRef		The reference id of the next polygon. [opt]
///  @param[in]		nextTile	The tile containing the next polygon. [opt]
///  @param[in]		nextPoly	The next polygon. [opt]

float dtQueryFilter_Example::getVirtualCost(const float * pa, const float * pb, const dtPolyRef prevRef, const dtMeshTile * prevTile, const dtPoly * prevPoly, const dtPolyRef curRef, const dtMeshTile * curTile, const dtPoly * curPoly, const dtPolyRef nextRef, const dtMeshTile * nextTile, const dtPoly * nextPoly) const
{
	const float cost = GetCustomCost(FVector2D(-pa[0], -pa[2]), FVector2D(-pb[0], -pb[2]));
	//UE_LOG(LogTemp, Warning, TEXT("Cost from P(%f,%f) to P(%f,%f) - Cost: %f"), -pa[0], -pa[2], -pb[0], -pb[2], cost);
	return cost; 
}

float dtQueryFilter_Example::GetCustomCost(const FVector2D StartPosition, const FVector2D EndPosition) const {
	float DefaultCost = 1; 	// Default pathfinding cost
	float Cost = 0;
	float ContinuousCostInsideDangerArea = 0;

	FVector2D PreviousPosition = StartPosition;
	FVector2D CurrentPosition = StartPosition;
	while (FVector2D::Distance(CurrentPosition, EndPosition) > 1) {		
		CurrentPosition = PreviousPosition + GetMinimumVector(StartPosition, EndPosition);
		const float Distance = FVector2D::DistSquared(PreviousPosition, CurrentPosition);
		if (PositionIsVisibleByPlayer(PreviousPosition)) {
			ContinuousCostInsideDangerArea += Distance * DefaultCost;
		}
		else {
			if (ContinuousCostInsideDangerArea > 0) {
				Cost += ContinuousCostInsideDangerArea * 1.1;
				//Cost += FMath::Pow(ContinuousCostInsideDangerArea, 2);
				ContinuousCostInsideDangerArea = 0;
			}
			Cost += Distance * DefaultCost;
		}
		PreviousPosition = CurrentPosition;
	}

	if (ContinuousCostInsideDangerArea > 0) {
		Cost += ContinuousCostInsideDangerArea * 1.1;
		//Cost += FMath::Pow(ContinuousCostInsideDangerArea, 2);
	}

	return Cost;
}

FVector2D dtQueryFilter_Example::GetMinimumVector(const FVector2D StartPosition, const FVector2D EndPosition) const {
	const FVector2D OriginalVector = EndPosition - StartPosition;
	float size = OriginalVector.Size();
	return OriginalVector / 10;
}

bool dtQueryFilter_Example::PositionIsVisibleByPlayer(const FVector2D Position) const {
	for (auto It = PlayerVisibleAreas.CreateConstIterator(); It; ++It) {
		Triangle Triangle = *It;
		if (Triangle.PointInsideTriangle(Position)) {
			return true;
		}
	}

	return false;
}

float dtQueryFilter_Example::GetCostOfPosition(const FVector2D Position) const {
	float Cost = 1;
	for (auto It = PlayerVisibleAreas.CreateConstIterator(); It; ++It) {
		Triangle Triangle = *It;
		if (Triangle.PointInsideTriangle(Position)) {
			const float DistanceToPlayer = FVector2D::Distance(Position, FVector2D(Triangle.V2.X, Triangle.V2.Y));
			
			Cost = 20;
			//Cost = 100;
			break;
		}
	}

	return Cost;
}


/*
void dtQueryFilter_Example::abc2()  const {
	UWorld* World = NULL;
	APlayerController* PlayerController = NULL;

	TIndirectArray<FWorldContext> WorldContexts = GEngine->GetWorldContexts();
	for (int i = 0; i < WorldContexts.Num(); ++i) {
		World = WorldContexts[i].World();
		PlayerController = World->GetFirstPlayerController();
		if (PlayerController != NULL) {

			break;
		}
	}

	if (World && PlayerController && PlayerController->GetPawn()) {
		const TArray<FVector> PlayerVisibleLocations = DeterminePlayerVisibility2(World, PlayerController->GetPawn());
	}
}*/

//----------------------------------------------------------------------//
// FRecastQueryFilter_Example();
//----------------------------------------------------------------------//

FRecastQueryFilter_Example::FRecastQueryFilter_Example(bool bIsVirtual)
	: dtQueryFilter_Example(bIsVirtual)
{
	SetExcludedArea(RECAST_NULL_AREA);
}

INavigationQueryFilterInterface* FRecastQueryFilter_Example::CreateCopy() const
{
	return new FRecastQueryFilter_Example(*this);
}

void FRecastQueryFilter_Example::SetIsVirtual(bool bIsVirtual)
{
	dtQueryFilter* Filter = static_cast<dtQueryFilter*>(this);
	Filter = new(Filter)dtQueryFilter(bIsVirtual);
	SetExcludedArea(RECAST_NULL_AREA);
}

void FRecastQueryFilter_Example::Reset()
{
	dtQueryFilter* Filter = static_cast<dtQueryFilter*>(this);
	Filter = new(Filter) dtQueryFilter(isVirtual);
	SetExcludedArea(RECAST_NULL_AREA);
}

void FRecastQueryFilter_Example::SetAreaCost(uint8 AreaType, float Cost)
{
	setAreaCost(AreaType, Cost);
}

void FRecastQueryFilter_Example::SetFixedAreaEnteringCost(uint8 AreaType, float Cost)
{
#if WITH_FIXED_AREA_ENTERING_COST
	setAreaFixedCost(AreaType, Cost);
#endif // WITH_FIXED_AREA_ENTERING_COST
}

void FRecastQueryFilter_Example::SetExcludedArea(uint8 AreaType)
{
	setAreaCost(AreaType, DT_UNWALKABLE_POLY_COST);
}

void FRecastQueryFilter_Example::SetAllAreaCosts(const float* CostArray, const int32 Count)
{
	// @todo could get away with memcopying to m_areaCost, but it's private and would require a little hack
	// need to consider if it's wort a try (not sure there'll be any perf gain)
	if (Count > RECAST_MAX_AREAS)
	{
		UE_LOG(LogNavigation, Warning, TEXT("FRecastQueryFilter_Example: Trying to set cost to more areas than allowed! Discarding redundant values."));
	}

	const int32 ElementsCount = FPlatformMath::Min(Count, RECAST_MAX_AREAS);
	for (int32 i = 0; i < ElementsCount; ++i)
	{
		setAreaCost(i, CostArray[i]);
	}
}

void FRecastQueryFilter_Example::GetAllAreaCosts(float* CostArray, float* FixedCostArray, const int32 Count) const
{
	const float* DetourCosts = getAllAreaCosts();
	const float* DetourFixedCosts = getAllFixedAreaCosts();

	FMemory::Memcpy(CostArray, DetourCosts, sizeof(float) * FMath::Min(Count, RECAST_MAX_AREAS));
	FMemory::Memcpy(FixedCostArray, DetourFixedCosts, sizeof(float) * FMath::Min(Count, RECAST_MAX_AREAS));
}

void FRecastQueryFilter_Example::SetBacktrackingEnabled(const bool bBacktracking)
{
	setIsBacktracking(bBacktracking);
}

bool FRecastQueryFilter_Example::IsBacktrackingEnabled() const
{
	return getIsBacktracking();
}

bool FRecastQueryFilter_Example::IsEqual(const INavigationQueryFilterInterface* Other) const
{
	// @NOTE: not type safe, should be changed when another filter type is introduced
	return FMemory::Memcmp(this, Other, sizeof(FRecastQueryFilter)) == 0;
}

void FRecastQueryFilter_Example::SetIncludeFlags(uint16 Flags)
{

	setIncludeFlags(Flags);

}

uint16 FRecastQueryFilter_Example::GetIncludeFlags() const
{
	return getIncludeFlags();
}

void FRecastQueryFilter_Example::SetExcludeFlags(uint16 Flags)
{
	setExcludeFlags(Flags);
}

uint16 FRecastQueryFilter_Example::GetExcludeFlags() const
{
	return getExcludeFlags();
}

//----------------------------------------------------------------------//
// AMyRecastNavMesh
//----------------------------------------------------------------------//

AMyRecastNavMesh::AMyRecastNavMesh(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMyRecastNavMesh::BeginPlay() {
	Super::BeginPlay();
	Timer = 0;
	SetupCustomNavFilter();
}

void AMyRecastNavMesh::Tick(float deltaTime)
{
	Super::Tick(deltaTime); 
	/*
	if (Timer <= dtQueryFilter_Example::UPDATE_FREQ) {
		Timer += deltaTime;
	}
	else {
		this->GetCustomFilter()->GetInfluenceMap()->UpdateTextureWithInfluences();
		Timer = 0;
	}*/
}

void AMyRecastNavMesh::SetupCustomNavFilter() {
	if (DefaultQueryFilter.IsValid())
	{
		DefaultQueryFilter->SetFilterImplementation(dynamic_cast<const INavigationQueryFilterInterface*>(&DefaultNavFilter));
	}
}

FRecastQueryFilter_Example* AMyRecastNavMesh::GetCustomFilter() const  {
	FRecastQueryFilter_Example* MyFRecastQueryFilter = reinterpret_cast<FRecastQueryFilter_Example*>(DefaultQueryFilter.Get()->GetImplementation());
	return MyFRecastQueryFilter;
}

