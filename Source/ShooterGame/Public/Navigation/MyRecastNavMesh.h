// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Runtime/Engine/Classes/AI/Navigation/NavFilters/NavigationQueryFilter.h"
#include "Runtime/Navmesh/Public/Detour/DetourNavMeshQuery.h"
#include "Runtime/Navmesh/Public/Detour/DetourNavMesh.h"
#include "AI/Navigation/PImplRecastNavMesh.h"
#include "AI/Navigation/RecastNavMesh.h"

#include "MyRecastNavMesh.generated.h"

class Triangle {
public:
	FVector V1;
	FVector V2;
	FVector V3;

public:

	Triangle() {
		V1 = FVector(0, 0, 0);
		V2 = FVector(0, 0, 0);
		V3 = FVector(0, 0, 0);
	}

	Triangle(const FVector V1, const FVector V2, const FVector V3) {
		this->V1 = V1;
		this->V2 = V2;
		this->V3 = V3;
	}

	bool PointInsideTriangle(const FVector2D Point) const {
		// spanning vectors of edge (v1,v2) and (v1,v3) //
		const FVector2D vs1 = FVector2D(V2.X - V1.X, V2.Y - V1.Y);
		const FVector2D vs2 = FVector2D(V3.X - V1.X, V3.Y - V1.Y);

		FVector2D q = FVector2D(Point.X - V1.X, Point.Y - V1.Y);

		float s = (float)FVector2D::CrossProduct(q, vs2) / FVector2D::CrossProduct(vs1, vs2);
		float t = (float)FVector2D::CrossProduct(vs1, q) / FVector2D::CrossProduct(vs1, vs2);

		if ((s >= 0) && (t >= 0) && (s + t <= 1)) { // inside triangle
			return true;
		}
		return false;
	}

	TArray<FVector> GetPointsInsideTriangle() const {
		TArray<FVector> PointsInsideTriangle;
		// Get the bounding box of the triangle
		const float maxX = FMath::Max(V1.X, FMath::Max(V2.X, V3.X));
		const float minX = FMath::Min(V1.X, FMath::Min(V2.X, V3.X));
		const float maxY = FMath::Max(V1.Y, FMath::Max(V2.Y, V3.Y));
		const float minY = FMath::Min(V1.Y, FMath::Min(V2.Y, V3.Y));

		// spanning vectors of edge (v1,v2) and (v1,v3) //
		const FVector2D vs1 = FVector2D(V2.X - V1.X, V2.Y - V1.Y);
		const FVector2D vs2 = FVector2D(V3.X - V1.X, V3.Y - V1.Y);

		for (int x = minX; x <= maxX; x += 10) // @todo slope!
		{
			for (int y = minY; y <= maxY; y += 10)
			{
				if (PointInsideTriangle(FVector2D(x, y))) {
					PointsInsideTriangle.Add(FVector(x, y, 0));
				}
			}
		}

		return PointsInsideTriangle;
	}

};

struct Vertex {
	FVector V = FVector(0, 0, 0);
	bool LeftmostVertex = false;
	bool RightmostVertex = false;
};


/**
*
*/
class SHOOTERGAME_API dtQueryFilter_Example : public dtQueryFilter
{
public:
	static const int UPDATE_FREQ = 1; // Seconds 
	static const int MAX_COST = 5000; // Cost of most dangerous area (player location)
		
	static bool SetPlayerVisibility(const TArray<Triangle> VisibleAreas);
private:
	static TArray<Triangle> PlayerVisibleAreas;

public:
	dtQueryFilter_Example(bool inIsVirtual = true) : dtQueryFilter(inIsVirtual)
	{

	}

	virtual ~dtQueryFilter_Example() {}
protected:
	
	virtual float getVirtualCost(const float* pa, const float* pb,
		const dtPolyRef prevRef, const dtMeshTile* prevTile, const dtPoly* prevPoly,
		const dtPolyRef curRef, const dtMeshTile* curTile, const dtPoly* curPoly,
		const dtPolyRef nextRef, const dtMeshTile* nextTile, const dtPoly* nextPoly) const override;
	
private:
	float GetCostOfPosition(const FVector2D Position) const;
	bool PositionIsVisibleByPlayer(const FVector2D Position) const;

	float GetCustomCost(const FVector2D StartPosition, const FVector2D EndPosition) const;

	FVector2D GetMinimumVector(const FVector2D StartPosition, const FVector2D EndPosition) const;
};

/**
*
*/
class SHOOTERGAME_API FRecastQueryFilter_Example : public INavigationQueryFilterInterface, public dtQueryFilter_Example
{
public:
	FRecastQueryFilter_Example(bool bIsVirtual = true);
	virtual ~FRecastQueryFilter_Example() {}

	virtual void Reset() override;

	virtual void SetAreaCost(uint8 AreaType, float Cost) override;
	virtual void SetFixedAreaEnteringCost(uint8 AreaType, float Cost) override;
	virtual void SetExcludedArea(uint8 AreaType) override;
	virtual void SetAllAreaCosts(const float* CostArray, const int32 Count) override;
	virtual void GetAllAreaCosts(float* CostArray, float* FixedCostArray, const int32 Count) const override;
	virtual void SetBacktrackingEnabled(const bool bBacktracking) override;
	virtual bool IsBacktrackingEnabled() const override;
	virtual bool IsEqual(const INavigationQueryFilterInterface* Other) const override;
	virtual void SetIncludeFlags(uint16 Flags) override;
	virtual uint16 GetIncludeFlags() const override;
	virtual void SetExcludeFlags(uint16 Flags) override;
	virtual uint16 GetExcludeFlags() const override;
	virtual INavigationQueryFilterInterface* CreateCopy() const override;

	const dtQueryFilter* GetAsDetourQueryFilter() const { return this; }

	/** note that it results in losing all area cost setup. Call it before setting anything else */
	void SetIsVirtual(bool bIsVirtual);
};

/**
*
*/
UCLASS()
class SHOOTERGAME_API AMyRecastNavMesh : public ARecastNavMesh
{
	GENERATED_BODY()

public:
	AMyRecastNavMesh(const FObjectInitializer& ObjectInitializer);
	FRecastQueryFilter_Example* GetCustomFilter() const;

private:
	float Timer;
	FRecastQueryFilter_Example DefaultNavFilter;

protected:
	virtual void Tick(float deltaTime) override;
	virtual void BeginPlay() override;

private:
	void SetupCustomNavFilter();
};