// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

[SupportedPlatforms(UnrealPlatformClass.Server)]
public class ShooterGameServerTarget : TargetRules
{
	public ShooterGameServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V5;

		// Use the latest include order version
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		// Ensure a unique build environment to avoid conflicts
		//BuildEnvironment = TargetBuildEnvironment.Unique;

		ExtraModuleNames.Add("ShooterGame");
	}
}
