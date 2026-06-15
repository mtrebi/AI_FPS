// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "ShooterMenuItemWidgetStyle.h"
#include "ShooterGame.h"

FShooterMenuItemStyle::FShooterMenuItemStyle()
{
}

FShooterMenuItemStyle::~FShooterMenuItemStyle()
{
}

const FName FShooterMenuItemStyle::TypeName(TEXT("FShooterMenuItemStyle"));

const FShooterMenuItemStyle& FShooterMenuItemStyle::GetDefault()
{
	static FShooterMenuItemStyle Default;
	return Default;
}

void FShooterMenuItemStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const
{
	OutBrushes.Add(&BackgroundBrush);
	OutBrushes.Add(&LeftArrowImage);
	OutBrushes.Add(&RightArrowImage);
}


UShooterMenuItemWidgetStyle::UShooterMenuItemWidgetStyle( const FObjectInitializer& ObjectInitializer )
	: Super(ObjectInitializer)
{
	
}
