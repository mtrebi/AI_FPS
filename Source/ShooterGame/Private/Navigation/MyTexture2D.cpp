// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "Public/Navigation/MyTexture2D.h"


//----------------------------------------------------------------------//
// MyTexture2D
//----------------------------------------------------------------------//
MyTexture2D::MyTexture2D(const FString TexturePath) {
	// Load Basic Image
	// https://answers.unrealengine.com/questions/23440/loading-textures-at-runtime.html

	Texture = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), NULL, *(TexturePath)));

	FColor* FormatedImageData = static_cast<FColor*>(Texture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_ONLY));

	TextureWidth = Texture->PlatformData->Mips[0].SizeX,
		TextureHeight = Texture->PlatformData->Mips[0].SizeY;

	Texture->PlatformData->Mips[0].BulkData.Unlock();
	//this->PostProcess();

}

int MyTexture2D::GetTextureWidth() {
	return TextureWidth;
}

int MyTexture2D::GetTextureHeight() {
	return TextureHeight;
}

void MyTexture2D::PostProcess() {
	for (int X = 0; X < TextureWidth; ++X) {
		for (int Y = 0; Y < TextureHeight; ++Y) {
			const FColor PixelColor = GetColorOfPixel(X, Y);
			if (PixelColor != NAVMESH_COLOR && (PixelColor == OBSTACLE_COLOR || PixelColor.R > 85 && PixelColor.G > 90 && PixelColor.B > 90)) {
				SetColorOfPixel(X, Y, OBSTACLE_COLOR, false);
			}
			else {
				SetColorOfPixel(X, Y, NAVMESH_COLOR, false);
			}
		}
	}
	Update();
}

void MyTexture2D::SetColorOfPixel(const uint16 X, const uint16 Y, const FColor PixelColor, const bool Update) const {
	FColor* FormatedImageData = static_cast<FColor*>(Texture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE));

	if (X >= 0 && X < TextureWidth && Y >= 0 && Y < TextureHeight)
	{
		FormatedImageData[Y * TextureWidth + X] = PixelColor;
	}
	else {
		FormatedImageData[X] = PixelColor;
	}
	Texture->PlatformData->Mips[0].BulkData.Unlock();

	if (Update) Texture->UpdateResource();
}

FColor MyTexture2D::GetColorOfPixel(const uint16 X, const uint16 Y) {
	// Access the original source image of the texture
	// https://answers.unrealengine.com/questions/25594/accessing-pixel-values-of-texture2d.html

	FColor PixelColor;
	FColor* FormatedImageData = static_cast<FColor*>(Texture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_ONLY));

	if (X >= 0 && X < TextureWidth && Y >= 0 && Y < TextureHeight)
	{
		PixelColor = FormatedImageData[Y * TextureWidth + X];

	}
	else {
		PixelColor = FormatedImageData[X];
	}

	Texture->PlatformData->Mips[0].BulkData.Unlock();

	return PixelColor;
}

FVector MyTexture2D::WorldSpaceToTexture(const FVector WorldPosition) const {
	const int XAsPixel = GetValueNewScale(MyTexture2D::MinX, MyTexture2D::MaxX, 0, TextureWidth, WorldPosition.X);
	const int YAsPixel = GetValueNewScale(MyTexture2D::MinY, MyTexture2D::MaxY, 0, TextureHeight, WorldPosition.Y);

	return FVector(XAsPixel, YAsPixel, 0);
}

FVector2D MyTexture2D::TextureToWorldSpace(const int TextureX, const int TextureY) const {
	const int XWorld = GetValueNewScale(0, TextureWidth, MyTexture2D::MinX, MyTexture2D::MaxX, TextureX);
	const int YWorld = GetValueNewScale(0, TextureHeight, MyTexture2D::MinY, MyTexture2D::MaxY, TextureY);

	return FVector2D (XWorld, YWorld);
}

FVector2D MyTexture2D::WorldSpaceToTexture(const FVector2D WorldPosition) const {
	return FVector2D(WorldSpaceToTexture(FVector(WorldPosition.X, WorldPosition.Y, 0)));
}

int MyTexture2D::GetValueNewScale(const int OldMin, const int OldMax, const int NewMin, const int NewMax, const int OldValue) const {
	const int OldRange = (OldMax - OldMin);
	const int NewRange = (NewMax - NewMin);
	const int NewValue = (((OldValue - OldMin) * NewRange) / OldRange) + NewMin;

	return NewValue;
}

void MyTexture2D::Update() {
	Texture->UpdateResource();
}

void MyTexture2D::Reset() {
	Texture->bChromaKeyTexture = !Texture->bChromaKeyTexture;
	Texture->bChromaKeyTexture = !Texture->bChromaKeyTexture;

	Texture->UpdateResource();
}
