// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "Public/Others/HelperMethods.h"
#include "Public/Navigation/MyNavigationQueryFilter.h"

UMyNavigationQueryFilter::UMyNavigationQueryFilter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

}

void UMyNavigationQueryFilter::InitializeFilter(const ANavigationData& NavData, FNavigationQueryFilter& Filter) const
{
#if WITH_RECAST

	const AMyRecastNavMesh* MyNavData = Cast<AMyRecastNavMesh>(&NavData);
	
	if (MyNavData) {
		const FRecastQueryFilter_Example* MyFRecastQueryFilter = MyNavData->GetCustomFilter();
		if (MyFRecastQueryFilter) {
			Filter.SetFilterType<FRecastQueryFilter_Example>();
			
			UWorld * World = GetWorld();

			if (World) {
				const FVector PlayerLocation = HelperMethods::GetPlayerPositionFromAI(World);
				const FVector PlayerForwardVector = HelperMethods::GetPlayerPositionFromAI(World);

				const TArray<Triangle> VisibleTriangles = HelperMethods::CalculateVisibility(World, PlayerLocation, PlayerForwardVector);

				// Update Navigation Mesh
				dtQueryFilter_Example::SetPlayerVisibility(VisibleTriangles);
			}
			



			//Filter.SetFilterImplementation(MyFRecastQueryFilter); 
		}
	}
	else {
		Filter.SetFilterImplementation(dynamic_cast<const INavigationQueryFilterInterface*>(ARecastNavMesh::GetNamedFilter(ERecastNamedFilter::FilterOutAreas)));
	}

#endif // WITH_RECAST

	Super::InitializeFilter(NavData, Filter);
}