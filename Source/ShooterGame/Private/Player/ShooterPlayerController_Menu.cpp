// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "Player/ShooterPlayerController_Menu.h"
#include "ShooterGame.h"
#include "ShooterStyle.h"


AShooterPlayerController_Menu::AShooterPlayerController_Menu(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void AShooterPlayerController_Menu::PostInitializeComponents() 
{
	Super::PostInitializeComponents();

	FShooterStyle::Initialize();
}
