// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Public/Navigation/MyTexture2D.h"
#include "Public/Navigation/MyRecastNavMesh.h"
#include "MyInfluenceMap.generated.h"

struct InfluenceTile {
	bool IsWalkable;
	float Influence;
	int X, Y, Index;
};

UCLASS()
class SHOOTERGAME_API AMyInfluenceMap : public AActor
{
	GENERATED_BODY()

private:
	// How much bias the update towards the existing value compared to the new value? 1 to Historic, 0 to Prediction
	float Momentumm;
	// How quickly the value decay with distance.
	float Decayy;

	float UpdateFrequency;

	//Map size
	int Height, Width;

	// Influences
	TArray<InfluenceTile*> Influences;
	TArray<InfluenceTile*> LocalInfluences;

	// Bitmap representation of influences
	MyTexture2D* BaseTexture;
	MyTexture2D* UpdatedTexture;
	
	// Temp
	TMap<FString, TArray<Triangle>> * BotsVisibilities;

	float TempTimer = 0;
public:
	AMyInfluenceMap();

	void CreateInfluenceMap(const float Momentum, const float Decay, const float UpdateFreq, const FString BaseImagePath, const FString ImagePath);
	
	MyTexture2D* UpdateTextureWithInfluences();

	InfluenceTile* GetInfluence(const int X, const int Y) const;
	InfluenceTile* GetInfluence(const int Index) const;
	InfluenceTile* GetInfluence(const FVector) const;

	TArray<InfluenceTile*> GetWalkableNeighbors(const int Index);


	void SetBotVisibility(FString BotName, TArray<Triangle> Visibility);

	bool SetInfluence(const int X, const int Y, const float NewInfluence, const float DeltaTime = 0);
	bool SetInfluence(const int index, const float NewInfluence, const float DeltaTime = 0);
	bool SetInfluence(const FVector, const float NewInfluence, const float DeltaTime = 0);

	void PropagateInfluence();
	void Initialize();

	virtual void Tick(float DeltaSeconds) override;
	virtual void Destroyed() override;
private:

	bool TileIsVisible(InfluenceTile * Tile);


	InfluenceTile * GetLocalInfluence(const int Index) const;
	bool SetLocalInfluence(const int index, const float NewInfluence);

	void UpdateWithLocal();
	void a();
};