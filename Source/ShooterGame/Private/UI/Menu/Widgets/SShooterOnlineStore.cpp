// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "SShooterOnlineStore.h"
#include "SHeaderRow.h"
#include "ShooterStyle.h"
#include "ShooterGameLoadingScreen.h"
#include "ShooterGameInstance.h"
#include "Online/ShooterGameSession.h"
#include "Interfaces/OnlineStoreInterfaceV2.h"
#include "Interfaces/OnlinePurchaseInterface.h"

#define LOCTEXT_NAMESPACE "ShooterGame.HUD.Menu"

void SShooterOnlineStore::Construct(const FArguments& InArgs)
{
	PlayerOwner = InArgs._PlayerOwner;
	OwnerWidget = InArgs._OwnerWidget;
	State = EStoreState::Browsing;
	StatusText = FText::GetEmpty();
	BoxWidth = 125;
	
	ChildSlot
	.VAlign(VAlign_Fill)
	.HAlign(HAlign_Fill)
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBox)  
			.WidthOverride(600)
			.HeightOverride(300)
			[
				SAssignNew(OfferListWidget, SListView<TSharedPtr<FStoreEntry>>)
				.ItemHeight(20)
				.ListItemsSource(&OfferList)
				.SelectionMode(ESelectionMode::Single)
				.OnGenerateRow(this, &SShooterOnlineStore::MakeListViewWidget)
				.OnSelectionChanged(this, &SShooterOnlineStore::EntrySelectionChanged)
				.OnMouseButtonDoubleClick(this,&SShooterOnlineStore::OnListItemDoubleClicked)
				.HeaderRow(
					SNew(SHeaderRow)
					+ SHeaderRow::Column("Title").FillWidth(3).DefaultLabel(NSLOCTEXT("OfferList", "OfferTitleColumn", "Offer Title"))
					+ SHeaderRow::Column("Description").FillWidth(6).DefaultLabel(NSLOCTEXT("OfferList", "OfferDescColumn", "Description"))
					+ SHeaderRow::Column("Price").FillWidth(2).HAlignCell(HAlign_Right).DefaultLabel(NSLOCTEXT("OfferList", "OfferPriceColumn", "Price"))
					+ SHeaderRow::Column("Purchased").FillWidth(2).HAlignCell(HAlign_Right).DefaultLabel(NSLOCTEXT("OfferList", "OfferPurchaseColumn", "Purchased?"))
				)
			]
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SOverlay)
			+SOverlay::Slot()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[
				SNew(SRichTextBlock)
				.Text(this, &SShooterOnlineStore::GetBottomText)
				.TextStyle(FShooterStyle::Get(), "ShooterGame.MenuServerListTextStyle")
				.DecoratorStyleSet(&FShooterStyle::Get())
				+ SRichTextBlock::ImageDecorator()
			]
		]
		
	];
}


FText SShooterOnlineStore::GetBottomText() const
{
	return StatusText;
}

/**
 * Ticks this widget.  Override in derived classes, but always call the parent implementation.
 *
 * @param  InCurrentTime  Current absolute real time
 * @param  InDeltaTime  Real time passed since last tick
 */
void SShooterOnlineStore::Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime )
{
	SCompoundWidget::Tick( AllottedGeometry, InCurrentTime, InDeltaTime );
}

/** Returns logged in user */
TSharedPtr<const FUniqueNetId> SShooterOnlineStore::GetLoggedInUser()
{
	TSharedPtr<const FUniqueNetId> UserIdPtr;
	IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineIdentityPtr IdentityInt = OnlineSub->GetIdentityInterface();
		if (IdentityInt.IsValid())
		{
			UserIdPtr = GetFirstSignedInUser(IdentityInt);
		}
	}
	return UserIdPtr;
}


/** Starts searching for servers */
void SShooterOnlineStore::BeginGettingOffers()
{
	if (State != EStoreState::Browsing)
	{
		UE_LOG(LogOnline, Warning, TEXT("We cannot start getting the offers, the store state is not Browsing (state = %d)"), static_cast<int32>(State));
		return;
	}

	OfferList.Reset();

	IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineStoreV2Ptr StoreV2Int = OnlineSub->GetStoreV2Interface();
		IOnlinePurchasePtr PurchaseInt = OnlineSub->GetPurchaseInterface();
		if (StoreV2Int.IsValid())
		{
			TSharedPtr<const FUniqueNetId> LoggedInUser = GetLoggedInUser();
			if (!LoggedInUser.IsValid())
			{
				UE_LOG(LogOnline, Warning, TEXT("There's no logged in user"));
				return;
			}

			TSharedRef<const FUniqueNetId> UserId = LoggedInUser.ToSharedRef();

			const FOnQueryOnlineStoreOffersComplete QueryStoreOffersDelegate = FOnQueryOnlineStoreOffersComplete::CreateLambda(
				[this, StoreV2Int, PurchaseInt, UserId](bool bWasSuccessful, const TArray<FUniqueOfferId>& OfferIds, const FString& ErrorString)
			{
				UE_LOG(LogOnline, Verbose, TEXT("Query store offers completed with status bWasSuccessful=[%d] Error=[%s] OfferIds=[%d]"), bWasSuccessful, ErrorString.IsEmpty() ? TEXT("None") : *ErrorString, OfferIds.Num());

				if (bWasSuccessful)
				{
					TArray<FOnlineStoreOfferRef> StoreOffers;
					StoreV2Int->GetOffers(StoreOffers);

					UE_LOG(LogOnline, Verbose, TEXT("Found %d offers in cache"), StoreOffers.Num());

					for (const FOnlineStoreOfferRef& OfferRef : StoreOffers)
					{
						UE_LOG(LogOnline, Verbose, TEXT("  Offer=[%s] CurrencyCode=[%s] PriceInt=[%d] DisplayPrice=[%s]"), *OfferRef->OfferId, *OfferRef->CurrencyCode, OfferRef->NumericPrice, *OfferRef->GetDisplayPrice().ToString());

						TSharedPtr<FStoreEntry> NewOffer = MakeShareable(new FStoreEntry());;
						NewOffer->OnlineId = OfferRef->OfferId;
						NewOffer->Title = OfferRef->Title.IsEmptyOrWhitespace() ? NSLOCTEXT("ShooterOnlineStore", "DefaultOfferTitle", "EmptyTitle") : OfferRef->Title;
						NewOffer->Description = OfferRef->Description.IsEmptyOrWhitespace() ? NSLOCTEXT("ShooterOnlineStore", "DefaultOfferDescription", "EmptyDescription") : OfferRef->Description;
						NewOffer->Price = OfferRef->GetDisplayPrice().IsEmptyOrWhitespace() ? NSLOCTEXT("ShooterOnlineStore", "DefaultOfferDescription", "EmptyPrice") : OfferRef->GetDisplayPrice();

						OfferList.Add(NewOffer);
					}
				}

				OfferListWidget->RequestListRefresh();
				if (OfferList.Num() > 0)
				{
					OfferListWidget->UpdateSelectionSet();
					OfferListWidget->SetSelection(0, ESelectInfo::OnNavigation);
				}

				if (PurchaseInt.IsValid())
				{
					const FOnQueryReceiptsComplete QueryReceiptsDelegate = FOnQueryReceiptsComplete::CreateLambda(
						[this, PurchaseInt, UserId](const FOnlineError& Result)
					{
						if (Result.bSucceeded)
						{
							TArray<FPurchaseReceipt> PurchasedReceipts;
							PurchaseInt->GetReceipts(*UserId, PurchasedReceipts);

							TArray<FUniqueOfferId> OffersPurchased;
							for (const FPurchaseReceipt& Receipt : PurchasedReceipts)
							{
								for (const FPurchaseReceipt::FReceiptOfferEntry& ReceiptOffer : Receipt.ReceiptOffers)
								{
									OffersPurchased.Add(ReceiptOffer.OfferId);
								}
							}

							MarkAsPurchased(OffersPurchased);
						}

						SetStoreState(EStoreState::Browsing);
					});

					PurchaseInt->QueryReceipts(*UserId, true, QueryReceiptsDelegate);
				}
				else
				{
					SetStoreState(EStoreState::Browsing);
				}
			});

			SetStoreState(EStoreState::GettingOffers);
			StoreV2Int->QueryOffersByFilter(*UserId, FOnlineStoreFilter(), QueryStoreOffersDelegate);
		}
		else
		{
			// just for test
			for (int32 Idx = 0; Idx < 16; ++Idx)
			{
				TSharedPtr<FStoreEntry> NewOffer = MakeShareable(new FStoreEntry());;
				NewOffer->OnlineId = FUniqueOfferId(FString::Printf(TEXT("FakeOfferId%d"), Idx));
				NewOffer->Title = FText::FromString(FString::Printf(TEXT("Offer #%d"), Idx));
				NewOffer->Description = FText::FromString(FString::Printf(TEXT("Somewhat long description for the offer #%d"), Idx));
				NewOffer->Price = FText::FromString(FString::Printf(TEXT("$%d"), Idx * 2));

				OfferList.Add(NewOffer);
			}

			SetStoreState(EStoreState::Browsing);

			OfferListWidget->RequestListRefresh();
			if (OfferList.Num() > 0)
			{
				OfferListWidget->UpdateSelectionSet();
				OfferListWidget->SetSelection(0, ESelectInfo::OnNavigation);
			}
		}
	}
}

void SShooterOnlineStore::SetStoreState(EStoreState NewState)
{
	UE_LOG(LogOnline, Verbose, TEXT("Transitioning the store from state %d to state %d"), static_cast<int32>(State), static_cast<int32>(NewState));
	State = NewState;

	switch (State)
	{
		case EStoreState::PurchasingAnOffer:
			StatusText = FText(NSLOCTEXT("ShooterOnlineStore", "Status", "Purchasing..."));
			break;

		case EStoreState::GettingOffers:
			StatusText = FText(NSLOCTEXT("ShooterOnlineStore", "Status", "Checking what's available..."));
			break;

		case EStoreState::Browsing:
			if (OfferList.Num() == 0)
			{
				StatusText = FText(NSLOCTEXT("ShooterOnlineStore", "Status", "No offers found - press Space to refresh"));
				break;
			}
			// intended fall-through
		default:
			StatusText = FText::GetEmpty();
			break;
	}
}

void SShooterOnlineStore::PurchaseOffer()
{
	if (State != EStoreState::Browsing)
	{
		UE_LOG(LogOnline, Warning, TEXT("We cannot purchase an offer, the store state is not Browsing (state = %d)"), static_cast<int32>(State));
		return;
	}

	if (SelectedItem.IsValid())
	{
		IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();
		if (OnlineSub)
		{
			IOnlinePurchasePtr PurchaseInt = OnlineSub->GetPurchaseInterface();
			if (PurchaseInt.IsValid())
			{
				TSharedPtr<const FUniqueNetId> LoggedInUser = GetLoggedInUser();
				if (!LoggedInUser.IsValid())
				{
					UE_LOG(LogOnline, Warning, TEXT("There's no logged in user"));
					return;
				}

				TSharedRef<const FUniqueNetId> UserId = LoggedInUser.ToSharedRef();

				FPurchaseCheckoutRequest CheckoutParams;
				CheckoutParams.AddPurchaseOffer(FString(), SelectedItem->OnlineId, 1, false);


				UE_LOG(LogOnline, Verbose, TEXT("Attempting to checkout OfferId %s"), *SelectedItem->OnlineId);

				SetStoreState(EStoreState::PurchasingAnOffer);
				PurchaseInt->Checkout(*UserId, CheckoutParams, FOnPurchaseCheckoutComplete::CreateLambda(
					[this](const FOnlineError& Result, const TSharedRef<FPurchaseReceipt>& Receipt)
				{
					UE_LOG(LogOnline, Verbose, TEXT("Checkout completed with status %s"), *Result.ToLogString());

					TArray<FUniqueOfferId> OffersPurchased;
					for (const FPurchaseReceipt::FReceiptOfferEntry& ReceiptOffer : Receipt->ReceiptOffers)
					{
						OffersPurchased.Add(ReceiptOffer.OfferId);
					}
					MarkAsPurchased(OffersPurchased);

					SetStoreState(EStoreState::Browsing);
				}));
			}
			else
			{
				UE_LOG(LogOnline, Warning, TEXT("IOnlinePurchase interface not available"));
			}
		}
	}
}

void SShooterOnlineStore::MarkAsPurchased(const TArray<FUniqueOfferId> & Offers)
{
	for (TSharedPtr<FStoreEntry> StoreOffer : OfferList)
	{
		for (const FUniqueOfferId& PurchasedOffer : Offers)
		{
			if (StoreOffer->OnlineId == PurchasedOffer)
			{
				StoreOffer->bPurchased = true;
			}
		}
	}

	// otherwise the items won't be picked up
	OfferListWidget->RebuildList();
}


void SShooterOnlineStore::OnFocusLost( const FFocusEvent& InFocusEvent )
{
	if (InFocusEvent.GetCause() != EFocusCause::SetDirectly)
	{
		FSlateApplication::Get().SetKeyboardFocus(SharedThis( this ));
	}
}

FReply SShooterOnlineStore::OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent)
{
	return FReply::Handled().SetUserFocus(OfferListWidget.ToSharedRef(), EFocusCause::SetDirectly).SetUserFocus(SharedThis(this), EFocusCause::SetDirectly, true);
}

void SShooterOnlineStore::EntrySelectionChanged(TSharedPtr<FStoreEntry> InItem, ESelectInfo::Type SelectInfo)
{
	SelectedItem = InItem;
}

void SShooterOnlineStore::OnListItemDoubleClicked(TSharedPtr<FStoreEntry> InItem)
{
	SelectedItem = InItem;
	PurchaseOffer();
	FSlateApplication::Get().SetKeyboardFocus(SharedThis(this));
}

void SShooterOnlineStore::MoveSelection(int32 MoveBy)
{
	int32 SelectedItemIndex = OfferList.IndexOfByKey(SelectedItem);

	if (SelectedItemIndex+MoveBy > -1 && SelectedItemIndex+MoveBy < OfferList.Num())
	{
		OfferListWidget->SetSelection(OfferList[SelectedItemIndex+MoveBy]);
	}
}

FReply SShooterOnlineStore::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) 
{
	if (State != EStoreState::Browsing) // lock input
	{
		return FReply::Handled();
	}

	FReply Result = FReply::Unhandled();
	const FKey Key = InKeyEvent.GetKey();
	
	if (Key == EKeys::Up || Key == EKeys::Gamepad_DPad_Up || Key == EKeys::Gamepad_LeftStick_Up)
	{
		MoveSelection(-1);
		Result = FReply::Handled();
	}
	else if (Key == EKeys::Down || Key == EKeys::Gamepad_DPad_Down || Key == EKeys::Gamepad_LeftStick_Down)
	{
		MoveSelection(1);
		Result = FReply::Handled();
	}
	else if (Key == EKeys::Enter || Key == EKeys::Virtual_Accept)
	{
		PurchaseOffer();
		Result = FReply::Handled();
		FSlateApplication::Get().SetKeyboardFocus(SharedThis(this));
	}
	else if (Key == EKeys::SpaceBar || Key == EKeys::Gamepad_FaceButton_Left)
	{
		BeginGettingOffers();
	}
	return Result;
}

TSharedRef<ITableRow> SShooterOnlineStore::MakeListViewWidget(TSharedPtr<FStoreEntry> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	class SStoreEntryWidget : public SMultiColumnTableRow< TSharedPtr<FStoreEntry> >
	{
	public:
		SLATE_BEGIN_ARGS(SStoreEntryWidget){}
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable, TSharedPtr<FStoreEntry> InItem)
		{
			Item = InItem;
			SMultiColumnTableRow< TSharedPtr<FStoreEntry> >::Construct(FSuperRowType::FArguments(), InOwnerTable);
		}

		TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName)
		{
			FText ItemText = FText::GetEmpty();
			if (ColumnName == "Title")
			{
				ItemText = Item->Title;
			}
			else if (ColumnName == "Description")
			{
				ItemText = Item->Description;
			}
			else if (ColumnName == "Price")
			{
				ItemText = Item->Price;
			}
			else if (ColumnName == "Purchased")
			{
				ItemText = Item->bPurchased ? FText(NSLOCTEXT("OfferList", "PurchasedYes", "Yes")) : FText(NSLOCTEXT("OfferList", "PurchasedNo", "No"));
			}
			return SNew(STextBlock)
				.Text(ItemText)
				.TextStyle(FShooterStyle::Get(), "ShooterGame.MenuStoreListTextStyle");
		}
		TSharedPtr<FStoreEntry> Item;
	};
	return SNew(SStoreEntryWidget, OwnerTable, Item);
}

#undef LOCTEXT_NAMESPACE
