// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "ShooterFriends.h"
#include "ShooterTypes.h"
#include "ShooterStyle.h"
#include "ShooterOptionsWidgetStyle.h"
#include "Player/ShooterPersistentUser.h"
#include "ShooterGameUserSettings.h"
#include "ShooterLocalPlayer.h"

#define LOCTEXT_NAMESPACE "ShooterGame.HUD.Menu"

void FShooterFriends::Construct(ULocalPlayer* _PlayerOwner, int32 LocalUserNum_)
{
	FriendsStyle = &FShooterStyle::Get().GetWidgetStyle<FShooterOptionsStyle>("DefaultShooterOptionsStyle");

	PlayerOwner = _PlayerOwner;
	LocalUserNum = LocalUserNum_;
	CurrFriendIndex = 0;
	MinFriendIndex = 0;
	MaxFriendIndex = 0; //initialized after the friends list is read in

	/** Friends menu root item */
	TSharedPtr<FShooterMenuItem> FriendsRoot = FShooterMenuItem::CreateRoot();

	//Populate the friends list
	FriendsItem = MenuHelper::AddMenuItem(FriendsRoot, LOCTEXT("Friends", "FRIENDS"));
	OnlineSub = IOnlineSubsystem::Get();
	OnlineFriendsPtr = OnlineSub->GetFriendsInterface();

	UpdateFriends(LocalUserNum);

	UserSettings = CastChecked<UShooterGameUserSettings>(GEngine->GetGameUserSettings());
}

void FShooterFriends::OnApplySettings()
{
	ApplySettings();
}

void FShooterFriends::ApplySettings()
{
	UShooterPersistentUser* PersistentUser = GetPersistentUser();
	if(PersistentUser)
	{
		PersistentUser->TellInputAboutKeybindings();

		PersistentUser->SaveIfDirty();
	}

	UserSettings->ApplySettings(false);

	OnApplyChanges.ExecuteIfBound();
}

void FShooterFriends::TellInputAboutKeybindings()
{
	UShooterPersistentUser* PersistentUser = GetPersistentUser();
	if(PersistentUser)
	{
		PersistentUser->TellInputAboutKeybindings();
	}
}

UShooterPersistentUser* FShooterFriends::GetPersistentUser() const
{
	UShooterLocalPlayer* const ShooterLocalPlayer = Cast<UShooterLocalPlayer>(PlayerOwner);
	return ShooterLocalPlayer ? ShooterLocalPlayer->GetPersistentUser() : nullptr;
	//// Main Menu
	//AShooterPlayerController_Menu* ShooterPCM = Cast<AShooterPlayerController_Menu>(PCOwner);
	//if(ShooterPCM)
	//{
	//	return ShooterPCM->GetPersistentUser();
	//}

	//// In-game Menu
	//AShooterPlayerController* ShooterPC = Cast<AShooterPlayerController>(PCOwner);
	//if(ShooterPC)
	//{
	//	return ShooterPC->GetPersistentUser();
	//}

	//return nullptr;
}

void FShooterFriends::UpdateFriends(int32 NewOwnerIndex)
{
	if (!OnlineFriendsPtr.IsValid())
	{
		return;
	}

	LocalUserNum = NewOwnerIndex;
	OnlineFriendsPtr->ReadFriendsList(LocalUserNum, EFriendsLists::ToString(EFriendsLists::OnlinePlayers), FOnReadFriendsListComplete::CreateSP(this, &FShooterFriends::OnFriendsUpdated));
}

void FShooterFriends::OnFriendsUpdated(int32 /*unused*/, bool bWasSuccessful, const FString& FriendListName, const FString& ErrorString)
{
	if (!bWasSuccessful)
	{
		UE_LOG(LogOnline, Warning, TEXT("Unable to update friendslist %s due to error=[%s]"), *FriendListName, *ErrorString);
		return;
	}

	MenuHelper::ClearSubMenu(FriendsItem);

	Friends.Reset();
	if (OnlineFriendsPtr->GetFriendsList(LocalUserNum, EFriendsLists::ToString(EFriendsLists::OnlinePlayers), Friends))
	{
		for (const TSharedRef<FOnlineFriend> Friend : Friends)
		{
			TSharedRef<FShooterMenuItem> FriendItem = MenuHelper::AddMenuItem(FriendsItem, FText::FromString(Friend->GetDisplayName()));
			FriendItem->OnControllerFacebuttonDownPressed.BindSP(this, &FShooterFriends::ViewSelectedFriendProfile);
			FriendItem->OnControllerDownInputPressed.BindSP(this, &FShooterFriends::IncrementFriendsCounter);
			FriendItem->OnControllerUpInputPressed.BindSP(this, &FShooterFriends::DecrementFriendsCounter);
		}

		MaxFriendIndex = Friends.Num() - 1;
	}

	MenuHelper::AddMenuItemSP(FriendsItem, LOCTEXT("Close", "CLOSE"), this, &FShooterFriends::OnApplySettings);
}

void FShooterFriends::IncrementFriendsCounter()
{
	if (CurrFriendIndex + 1 <= MaxFriendIndex)
	{
		++CurrFriendIndex;
	}
}
void FShooterFriends::DecrementFriendsCounter()
{
	if (CurrFriendIndex - 1 >= MinFriendIndex)
	{
		--CurrFriendIndex;
	}
}
void FShooterFriends::ViewSelectedFriendProfile()
{
	auto Identity = Online::GetIdentityInterface();
	if (Identity.IsValid() && Friends.IsValidIndex(CurrFriendIndex))
	{
		TSharedPtr<const FUniqueNetId> Requestor = Identity->GetUniquePlayerId(LocalUserNum);
		TSharedPtr<const FUniqueNetId> Requestee = Friends[CurrFriendIndex]->GetUserId();
		auto ExternalUI = Online::GetExternalUIInterface();
		if (ExternalUI.IsValid() && Requestor.IsValid() && Requestee.IsValid())
		{
			ExternalUI->ShowProfileUI(*Requestor, *Requestee, FOnProfileUIClosedDelegate());
		}
	}
}
void FShooterFriends::InviteSelectedFriendToGame()
{
	// invite the user to the current gamesession

	IOnlineSessionPtr OnlineSessionInterface = OnlineSub->GetSessionInterface();
	if (OnlineSessionInterface.IsValid())
	{
		OnlineSessionInterface->SendSessionInviteToFriend(LocalUserNum, NAME_GameSession, *Friends[CurrFriendIndex]->GetUserId());
	}
}


#undef LOCTEXT_NAMESPACE

