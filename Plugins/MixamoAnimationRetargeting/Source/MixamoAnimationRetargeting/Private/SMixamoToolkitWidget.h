// Copyright 2022 UNAmedia. All Rights Reserved.

#pragma once

#include "Widgets/SCompoundWidget.h"

class FContentBrowserItemData;

class SRiggedSkeletonPicker : public SCompoundWidget
{
public:
	DECLARE_DELEGATE_RetVal_OneParam(bool, FOnShouldFilterAsset, const FAssetData& /*AssetData*/);

	SLATE_BEGIN_ARGS(SRiggedSkeletonPicker)
		{}

		SLATE_ARGUMENT(FText, Title)
		SLATE_ARGUMENT(FText, Description)
		/** Called to check if an asset is valid to use */
		SLATE_EVENT(FOnShouldFilterAsset, OnShouldFilterAsset)
	SLATE_END_ARGS()

public:
	SRiggedSkeletonPicker();
	void Construct(const FArguments& InArgs);

	USkeleton * GetSelectedSkeleton();

private:
	void OnAssetSelected(const FAssetData & AssetData);
	void OnAssetDoubleClicked(const FAssetData & AssetData);
	bool CanSelect() const;
	FReply OnSelect();
	FReply OnCancel();

	void CloseWindow();

private:
	// Track in ActiveSkeleton the temporary selected asset,
	// only after the user confirms set SelectedSkeleton. So if
	// the widget is externally closed we don't report an un-selected
	// asset.
	USkeleton * ActiveSkeleton;
	USkeleton * SelectedSkeleton;
};



class SRootMotionExtractionWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SRootMotionExtractionWidget)
		: _ReferenceSkeleton(nullptr)
		{}

		SLATE_ARGUMENT(USkeleton*, ReferenceSkeleton)
	SLATE_END_ARGS()

public:
    SRootMotionExtractionWidget();
    void Construct(const FArguments& InArgs);

    UAnimSequence * GetSelectedAnimation() { return SelectedAnimationSequence; }
    UAnimSequence * GetSelectedInPlaceAnimation() { return SelectedInPlaceAnimationSequence; }

private:
    bool CanSelect() const;
    FReply OnSelect();
    FReply OnCancel();

    void CloseWindow();

    TSharedRef<SWidget> CreateAnimationSequencePicker(const USkeleton * ReferenceSkeleton, bool InPlaceAnimation);

private:
    // Track in ActiveXYZ the temporary selected assets, only after the user confirms
	// set SelectedXYZ properties.
	// So if the widget is externally closed we don't report errors for un-selected assets.
    UAnimSequence * ActiveAnimationSequence;
    UAnimSequence * ActiveInPlaceAnimationSequence;

    UAnimSequence * SelectedAnimationSequence;
    UAnimSequence * SelectedInPlaceAnimationSequence;
};

class SOverridingAssetsConfirmationDialog : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SOverridingAssetsConfirmationDialog)
	{}
	SLATE_ARGUMENT(TArray<UObject*>, AssetsToOverwrite)
	SLATE_END_ARGS()

public:
	SOverridingAssetsConfirmationDialog();
	void Construct(const FArguments& InArgs);

	bool HasConfirmed() const { return bConfirmed; }

private:

	FReply OnConfirm();
	FReply OnCancel();

	void CloseWindow();

	bool EnumerateCustomSourceItemDatas(TFunctionRef<bool(FContentBrowserItemData&&)> InCallback);

private:

	TArray<UObject*> AssetsToOverwrite;
	bool bConfirmed;
};
