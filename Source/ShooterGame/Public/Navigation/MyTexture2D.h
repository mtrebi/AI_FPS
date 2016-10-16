// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "AI/Navigation/RecastNavMesh.h"
#include "Runtime/Engine/Classes/Engine/Texture2D.h"
/**
 * 
 */
class SHOOTERGAME_API MyTexture2D
{
public:
	const FColor NAVMESH_COLOR = FColor(102, 255, 102);
	const FColor OBSTACLE_COLOR = FColor(64 ,64,64);
	const FColor ENEMY_VIEW_COLOR = FColor(255, 200, 25);

	//@todo REMOVE FROM HERE
	static const int MinX = -1900;
	static const int MaxX = 1600;
	static const int MinY = -1810;
	static const int MaxY = 1690;
	static const int Z = 0;
private:
	UTexture2D* Texture;
	int TextureWidth, TextureHeight;

public:
	MyTexture2D(const FString TexturePath);

	int GetTextureWidth();
	int GetTextureHeight();

	FColor GetColorOfPixel(const uint16 X, const uint16 Y);
	void SetColorOfPixel(const uint16 X, const uint16 Y, const FColor PixelColor, const bool Update = true) const;
	
	FVector2D WorldSpaceToTexture(const FVector2D WorldPosition) const;
	FVector WorldSpaceToTexture(const FVector WorldPosition) const;
	FVector2D TextureToWorldSpace(const int TextureX, const int TextureY) const;


	void Update();
	void Reset();

	~MyTexture2D();

private:
	void PostProcess();

	int GetValueNewScale(const int OldMin, const int OldMax, const int NewMin, const int NewMax, const int OldValue) const;

};
