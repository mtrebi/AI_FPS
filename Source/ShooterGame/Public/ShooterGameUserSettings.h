// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ShooterGameUserSettings.generated.h"

UCLASS()
class UShooterGameUserSettings : public UGameUserSettings
{
	GENERATED_UCLASS_BODY()

	/** Applies all current user settings to the game and saves to permanent storage (e.g. file), optionally checking for command line overrides. */
	virtual void ApplySettings(bool bCheckForCommandLineOverrides) override;

	int32 GetGraphicsQuality() const
	{
		return GraphicsQuality;
	}

	void SetGraphicsQuality(int32 InGraphicsQuality)
	{
		GraphicsQuality = InGraphicsQuality;
	}

	bool IsLanMatch() const
	{
		return bIsLanMatch;
	}

	void SetLanMatch(bool InbIsLanMatch)
	{
		bIsLanMatch = InbIsLanMatch;
	}
	
	bool IsDedicatedServer() const
	{
		return bIsDedicatedServer;
	}

	void SetDedicatedServer(bool InbIsDedicatedServer)
	{
		bIsDedicatedServer = InbIsDedicatedServer;
	}

	// interface UGameUserSettings
	virtual void SetToDefaults() override;

private:
	/**
	 * Graphics Quality
	 *	0 = Low
	 *	1 = High
	 */
	UPROPERTY(config)
	int32 GraphicsQuality;

	/** is lan match? */
	UPROPERTY(config)
	bool bIsLanMatch;

	/** is dedicated server? */
	UPROPERTY(config)
	bool bIsDedicatedServer;
};
