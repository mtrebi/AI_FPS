// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Public/Navigation/MyRecastNavMesh.h"

/**
 * 
 */
class SHOOTERGAME_API HelperMethods
{
public:
	static const int PLAYER_FOV = 50;
	static const int PLAYER_DV = 5500;
	static const int OFFSET = 10;

	static const int EYES_POS_Z = 150;


	// 
	static const int MIN_DIST_PLAYER = 750;

	// Health diff between updates to consider as losing fight
	static const int RECEIVING_DMG = 15;

	// Time to wait till life regens
	static const int WAIT_FOR_REGEN = 5;

	// Time between look around and look around
	static const int LOOKAROUND_PERIOD = 2;
	// Time to complete look around
	static const int LOOKAROUND_COMPLETE = 1;
	// Look around angle
	static const int LOOKAROUND_ANGLE = 35;
public:
	static 	FVector GetPlayerPositionFromAI(UWorld * World);
	static 	FVector GetPlayerForwardVectorFromAI(UWorld * World);

	static TArray<Triangle> CalculateVisibility(UWorld * World, const FVector Location, const FVector ForwardVector, const float ViewAngle = PLAYER_FOV, const float ViewDistance = PLAYER_DV);
	
	//static TArray<FVector> GetLocationOfCoverAnnotationsWithinRadius(UWorld * World, const FVector ContextLocation, const float MaxRadius);
	//static TArray<FVector> GetLocationOfAttackAnnotationsWithinRadius(UWorld * World, const FVector ContextLocation, const float MaxRadius);
private:
	static TArray<Vertex> GetVisibleObstaclesVertexs(UWorld * World, const FVector EyesLocation, const FVector ForwardVector, const float ViewAngle = PLAYER_FOV, const float ViewDistance = PLAYER_DV);
	static void SortByAngle(TArray<Vertex> &FVectorArray, const FVector EyesLocation, const FVector FirstTrace);
	static TArray<Triangle> GetVisibleTriangles(const TArray<Vertex> VisibleVertexs, UWorld * World, const FVector EyesLocation, const float ViewAngle = PLAYER_FOV, const float ViewDistance = PLAYER_DV);


	FRecastQueryFilter_Example * GetCustomFilter();
	void PaintTriangle(const Triangle Triangle);

};
