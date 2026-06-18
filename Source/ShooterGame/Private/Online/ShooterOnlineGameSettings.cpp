// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "ShooterOnlineGameSettings.h"
#include "ShooterGame.h"

#define SEARCH_PRESENCE               FName(TEXT("PRESENCESEARCH"))
#define SEARCH_DEDICATED_ONLY         FName(TEXT("DEDICATEDONLY"))
#define SEARCH_EMPTY_SERVERS_ONLY     FName(TEXT("EMPTYSERVERSONLY"))
#define SEARCH_NONEMPTY_SERVERS_ONLY  FName(TEXT("NONEMPTYSERVERSONLY"))
#define SEARCH_USER                   FName(TEXT("USERSEARCH"))
#define SEARCH_KEYWORDS               FName(TEXT("KEYWORDS"))
#define SEARCH_MATCHMAKING_QUEUE      FName(TEXT("MATCHMAKINGQUEUE"))

FShooterOnlineSessionSettings::FShooterOnlineSessionSettings(bool bIsLAN, bool bIsPresence, int32 MaxNumPlayers)
{
	NumPublicConnections = MaxNumPlayers;
	if (NumPublicConnections < 0)
	{
		NumPublicConnections = 0;
	}
	NumPrivateConnections = 0;
	bIsLANMatch = bIsLAN;
	bShouldAdvertise = true;
	bAllowJoinInProgress = true;
	bAllowInvites = true;
	bUsesPresence = bIsPresence;
	bAllowJoinViaPresence = true;
	bAllowJoinViaPresenceFriendsOnly = false;
}

FShooterOnlineSearchSettings::FShooterOnlineSearchSettings(bool bSearchingLAN, bool bSearchingPresence)
{
	bIsLanQuery = bSearchingLAN;
	MaxSearchResults = 10;
	PingBucketSize = 50;

	if (bSearchingPresence)
	{
		QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	}
}

FShooterOnlineSearchSettingsEmptyDedicated::FShooterOnlineSearchSettingsEmptyDedicated(bool bSearchingLAN, bool bSearchingPresence) :
	FShooterOnlineSearchSettings(bSearchingLAN, bSearchingPresence)
{
	QuerySettings.Set(SEARCH_DEDICATED_ONLY, true, EOnlineComparisonOp::Equals);
	QuerySettings.Set(SEARCH_EMPTY_SERVERS_ONLY, true, EOnlineComparisonOp::Equals);
}
