// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SlateBasics.h"
#include "SlateExtras.h"
#include "ShooterGame.h"
#include "SShooterMenuWidget.h"

class AShooterGameSession;

struct FStoreEntry
{
	FUniqueOfferId OnlineId;
	FText Title;
	FText Description;
	FText Price;
	bool bPurchased;
};

//class declare
class SShooterOnlineStore : public SShooterMenuWidget
{
public:
	SLATE_BEGIN_ARGS(SShooterOnlineStore)
	{}

	SLATE_ARGUMENT(TWeakObjectPtr<ULocalPlayer>, PlayerOwner)
	SLATE_ARGUMENT(TSharedPtr<SWidget>, OwnerWidget)

	SLATE_END_ARGS()

	/** needed for every widget */
	void Construct(const FArguments& InArgs);

	/** if we want to receive focus */
	virtual bool SupportsKeyboardFocus() const override { return true; }

	/** focus received handler - keep the ActionBindingsList focused */
	virtual FReply OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent) override;
	
	/** focus lost handler - keep the ActionBindingsList focused */
	virtual void OnFocusLost( const FFocusEvent& InFocusEvent ) override;

	/** key down handler */
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

	/** SListView item double clicked */
	void OnListItemDoubleClicked(TSharedPtr<FStoreEntry> InItem);

	/** creates single item widget, called for every list item */
	TSharedRef<ITableRow> MakeListViewWidget(TSharedPtr<FStoreEntry> Item, const TSharedRef<STableViewBase>& OwnerTable);

	/** selection changed handler */
	void EntrySelectionChanged(TSharedPtr<FStoreEntry> InItem, ESelectInfo::Type SelectInfo);

	/** Starts getting the offers etc */
	void BeginGettingOffers();

	/** Called when server search is finished */
	void OnGettingOffersFinished();

	/** fill/update server list, should be called before showing this control */
	void UpdateServerList();

	/** purchases the chose offer */
	void PurchaseOffer();

	/** selects item at current + MoveBy index */
	void MoveSelection(int32 MoveBy);

	/**
	 * Ticks this widget.  Override in derived classes, but always call the parent implementation.
	 *
	 * @param  AllottedGeometry The space allotted for this widget
	 * @param  InCurrentTime  Current absolute real time
	 * @param  InDeltaTime  Real time passed since last tick
	 */
	void Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime );

protected:

	enum class EStoreState
	{
		Browsing,
		GettingOffers,
		PurchasingAnOffer,
	};

	/** Store state */
	EStoreState State;

	/** Transitions to new store state */
	void SetStoreState(EStoreState NewState);

	/** Marks offers as purchased */
	void MarkAsPurchased(const TArray<FUniqueOfferId> & Offers);

	/** Returns logged in user */
	TSharedPtr<const FUniqueNetId> GetLoggedInUser();

	/** action bindings array */
	TArray< TSharedPtr<FStoreEntry> > OfferList;

	/** action bindings list slate widget */
	TSharedPtr< SListView< TSharedPtr<FStoreEntry> > > OfferListWidget; 

	/** currently selected list item */
	TSharedPtr<FStoreEntry> SelectedItem;

	/** get current status text */
	FText GetBottomText() const;

	/** current status text */
	FText StatusText;

	/** size of standard column in pixels */
	int32 BoxWidth;

	/** pointer to our owner PC */
	TWeakObjectPtr<class ULocalPlayer> PlayerOwner;

	/** pointer to our parent widget */
	TSharedPtr<class SWidget> OwnerWidget;
};


