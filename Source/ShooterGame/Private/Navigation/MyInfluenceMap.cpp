// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include <algorithm>
using namespace std;
#include "Public/Navigation/MyInfluenceMap.h"

void AMyInfluenceMap::Initialize() {
	// Setup basic influence map
	BotsVisibilities = new TMap<FString, TArray<Triangle>>();
	for (int Index = 0; Index < Width*Height; ++Index) {
		InfluenceTile * Tile = new InfluenceTile();
		Tile->Influence = 0;
		Tile->Index = Index;
		Tile->X = (Index >= Width) ? (Index % Width) : Index;
		Tile->Y = (Index >= Width) ? (Index / Width) : 0;
		Tile->IsWalkable = BaseTexture->GetColorOfPixel(Tile->X, Tile->Y).G  > 100;

		InfluenceTile * LocalTile = new InfluenceTile();
		LocalTile->Influence = 0;
		LocalTile->Index = Index;
		LocalTile->X = (Index >= Width) ? (Index % Width) : Index;
		LocalTile->Y = (Index >= Width) ? (Index / Width) : 0;
		LocalTile->IsWalkable = BaseTexture->GetColorOfPixel(LocalTile->X, LocalTile->Y).G  > 100;


		Influences.Add(Tile);
		LocalInfluences.Add(LocalTile);

		UpdatedTexture->SetColorOfPixel(Tile->X, Tile->Y, BaseTexture->GetColorOfPixel(Tile->X, Tile->Y));

	}

	UpdatedTexture->Update();
}


MyTexture2D* AMyInfluenceMap::UpdateTextureWithInfluences() {
	for (int X = 0; X < Width; ++X) {
		for (int Y = 0; Y < Height; ++Y) {
			InfluenceTile * Tile = GetInfluence(X, Y);
			if (Tile->IsWalkable) {
				const float Influence = GetInfluence(X, Y)->Influence;
				float RealInfluence = FMath::Min(255.0f, Influence);
				RealInfluence = FMath::Max(0.0f, Influence);
				//if (Influence > 1) {
					const FColor OriginalColor = UpdatedTexture->GetColorOfPixel(X, Y);
					const FColor PixelColor = FColor(RealInfluence, 0, 0);
					UpdatedTexture->SetColorOfPixel(X, Y, PixelColor, false);
				//}
			}

		}
	}
	UpdatedTexture->Update();
	return UpdatedTexture;
}

InfluenceTile * AMyInfluenceMap::GetInfluence(const int X, const int Y) const {
	int Index;
	if (X >= 0 && X < Width && Y >= 0 && Y < Height) {
		Index = (Y * Width) + X;
	}
	else {
		Index = X;
	}
	return GetInfluence(Index);
}

InfluenceTile* AMyInfluenceMap::GetInfluence(const int Index) const {
	return Influences[Index];
}

InfluenceTile* AMyInfluenceMap::GetLocalInfluence(const int Index) const {
	return LocalInfluences[Index];
}

InfluenceTile * AMyInfluenceMap::GetInfluence(const FVector WorldPosition) const {
	FVector TexturePosition = BaseTexture->WorldSpaceToTexture(WorldPosition);
	return GetInfluence(TexturePosition.X, TexturePosition.Y);
}

bool AMyInfluenceMap::SetInfluence(const int X, const int Y, const float NewInfluence, const float DeltaTime) {
	int Index;
	if (X >= 0 && X < Width && Y >= 0 && Y < Height){
		Index = (Y * Width) + X;
	}
	else {
		Index = X;
	}
	return SetInfluence(Index, NewInfluence, DeltaTime);
}

bool AMyInfluenceMap::SetInfluence(const int Index, const float NewInfluence, const float DeltaTime) {
	//@todo instant propagation
	if (Index > 0 && Index < Width * Height) {
		InfluenceTile * CurrentTile = GetInfluence(Index);
		if (CurrentTile->IsWalkable) {
			Influences[Index]->Influence = NewInfluence;
			return true;
		}
	}
	return false;
}

bool AMyInfluenceMap::SetLocalInfluence(const int Index, const float NewInfluence) {
	if (Index > 0 && Index < Width * Height) {
		InfluenceTile * CurrentTile = GetLocalInfluence(Index);
		if (CurrentTile->IsWalkable) {
			LocalInfluences[Index]->Influence = NewInfluence;
			return true;
		}
	}
	return false;
}

bool AMyInfluenceMap::SetInfluence(const FVector WorldPosition, const float NewInfluence, const float DeltaTime) {
	FVector TexturePosition = UpdatedTexture->WorldSpaceToTexture(WorldPosition);
	
	return SetInfluence(TexturePosition.X, TexturePosition.Y, NewInfluence, DeltaTime);
}


TArray<InfluenceTile*> AMyInfluenceMap::GetWalkableNeighbors(const int Index)  {
	TArray<InfluenceTile *> Neighbors;
	const InfluenceTile * Tile = GetInfluence(Index);

	int NeigborLevels = 2;
	for (int X = Tile->X - NeigborLevels; X <= Tile->X + NeigborLevels; ++X) {
		for (int Y = Tile->Y - NeigborLevels; Y <= Tile->Y + NeigborLevels; ++Y) {
			if (X >= 0 && X < Width && Y >= 0 && Y < Height) { // Inside Canvas
				if (X != Tile->X || Y != Tile->Y) { // Skip current Tile
					InfluenceTile * Neighbor = GetInfluence(X, Y);
					if (Neighbor->IsWalkable) {
						Neighbors.Add(Neighbor);
					}
				}
			}

		}
	}
	return Neighbors;
}

void AMyInfluenceMap::PropagateInfluence() {
	for (int Index = 0; Index < Width * Height; ++Index) {
		float MaxInfluence = 0.0f;
		InfluenceTile * CurrentTile = GetInfluence(Index);
		if (CurrentTile->IsWalkable && !TileIsVisible(CurrentTile)) {
			const TArray<InfluenceTile *> WalkableNeighbors = GetWalkableNeighbors(Index);
			for (auto It = WalkableNeighbors.CreateConstIterator(); It; ++It) {
				InfluenceTile * NeighborTile = *It;
				const FVector2D CurrentTileWorld = UpdatedTexture->TextureToWorldSpace(CurrentTile->X, CurrentTile->Y);
				const FVector2D NeighborTileWorld = UpdatedTexture->TextureToWorldSpace(NeighborTile->X, NeighborTile->Y);
				const float Distance = FVector2D::Distance(CurrentTileWorld, NeighborTileWorld);
				const float Influence = NeighborTile->Influence * expf(-Distance * Decay);
				MaxInfluence = FMath::Max(Influence, MaxInfluence);
			}
			const float NewInfluence = FMath::Lerp(CurrentTile->Influence, MaxInfluence, Momentum);
			SetLocalInfluence(Index, NewInfluence);
		}
	}
	UpdateWithLocal();
}

void AMyInfluenceMap::UpdateWithLocal() {
	//UE_LOG(LogTemp, Log, TEXT("F:UpdateWithLocal"));

	for (int Index = 0; Index < Width * Height; ++Index) {
		SetInfluence(Index, GetLocalInfluence(Index)->Influence, 0);
	}
}

// Sets default values
AMyInfluenceMap::AMyInfluenceMap()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bAllowTickOnDedicatedServer = true;

}

void AMyInfluenceMap::CreateInfluenceMap(const float Momentum, const float Decay, const float UpdateFreq, const FString BaseImagePath, const FString ImagePath) {
	this->Momentum = Momentum;
	this->Decay = Decay;
	this->UpdateFrequency = UpdateFreq;

	this->BaseTexture = new MyTexture2D(BaseImagePath);
	this->UpdatedTexture = new MyTexture2D(ImagePath);

	this->Width = BaseTexture->GetTextureWidth();
	this->Height = BaseTexture->GetTextureHeight();

	//this->Influences = new InfluenceTile[Width*Height];
	//this->LocalInfluences = new InfluenceTile[Width*Height];

	this->Initialize();
}

void AMyInfluenceMap::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
	if (TempTimer > UpdateFrequency) {
		PropagateInfluence();
		a();
		UpdateTextureWithInfluences();
		TempTimer = 0;
	}
	else {
		TempTimer += DeltaSeconds;
	}
}

void AMyInfluenceMap::Destroyed() {
	Super::Destroyed();
	//UpdatedTexture->Reset();
	//UpdatedTexture->Update();
}


void AMyInfluenceMap::a() {
	for (auto It = Influences.CreateConstIterator(); It; ++It) {
		InfluenceTile * Tile = *It;
		if (TileIsVisible(Tile)) {
			Tile->Influence = -100000;
		}
	}
}

void AMyInfluenceMap::SetBotVisibility(FString BotName, TArray<Triangle> Visibility) {
	
	BotsVisibilities->Add(BotName, Visibility);
}

bool AMyInfluenceMap::TileIsVisible(InfluenceTile * Tile) {
	for (auto ItBots = BotsVisibilities->CreateConstIterator(); ItBots; ++ItBots) {
		TArray<Triangle> BotVisibility = ItBots.Value();
		for (auto ItTriangles = BotVisibility.CreateConstIterator(); ItTriangles; ++ItTriangles) {
			Triangle triangle = *ItTriangles;
			if (triangle.PointInsideTriangle(BaseTexture->TextureToWorldSpace(Tile->X, Tile->Y))) {
				return true;
			}
		}
	}
	return false;
}