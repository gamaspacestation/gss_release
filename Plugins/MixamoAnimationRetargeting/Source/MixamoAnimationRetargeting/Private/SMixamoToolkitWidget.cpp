// Copyright 2022 UNAmedia. All Rights Reserved.

#include "SMixamoToolkitWidget.h"
#include "MixamoToolkitPrivatePCH.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SUniformGridPanel.h"

#include "SAssetView.h"

#include "Features/IModularFeatures.h"
#include "IContentBrowserDataModule.h"
#include "ContentBrowserModule.h"
#include "ContentBrowserDataSource.h"
#include "ContentBrowserAssetDataSource.h"
#include "ContentBrowserAssetDataCore.h"
#include "IContentBrowserSingleton.h"	// FAssetPickerConfig
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Animation/Rig.h"



#define LOCTEXT_NAMESPACE "FMixamoAnimationRetargetingModule"



SRiggedSkeletonPicker::SRiggedSkeletonPicker()
	: ActiveSkeleton(nullptr),
	  SelectedSkeleton(nullptr)
{}



void SRiggedSkeletonPicker::Construct(const FArguments& InArgs)
{
	checkf(!InArgs._Title.IsEmpty(), TEXT("A title must be specified."));
	checkf(!InArgs._Description.IsEmpty(), TEXT("A description must be specified."));

	ActiveSkeleton = nullptr;
	SelectedSkeleton = nullptr;

	// Configure the Asset Picker.
	FAssetPickerConfig AssetPickerConfig;
	AssetPickerConfig.Filter.ClassNames.Add(USkeleton::StaticClass()->GetFName());
	AssetPickerConfig.Filter.bRecursiveClasses = true;
	AssetPickerConfig.SelectionMode = ESelectionMode::Single;
	AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateSP(this, &SRiggedSkeletonPicker::OnAssetSelected);
	AssetPickerConfig.AssetShowWarningText = LOCTEXT("SRiggedSkeletonPicker_NoAssets", "No Skeleton asset for the UE Mannequin found!");
	AssetPickerConfig.OnShouldFilterAsset = InArgs._OnShouldFilterAsset;
	// Aesthetic settings.
	AssetPickerConfig.OnAssetDoubleClicked = FOnAssetDoubleClicked::CreateSP(this, &SRiggedSkeletonPicker::OnAssetDoubleClicked);
	AssetPickerConfig.InitialAssetViewType = EAssetViewType::Column;
	AssetPickerConfig.bShowPathInColumnView = true;
	AssetPickerConfig.bShowTypeInColumnView = false;
	// Hide all asset registry columns by default (we only really want the name and path)
	TArray<UObject::FAssetRegistryTag> AssetRegistryTags;
	USkeleton::StaticClass()->GetDefaultObject()->GetAssetRegistryTags(AssetRegistryTags);
	for (UObject::FAssetRegistryTag & AssetRegistryTag : AssetRegistryTags)
	{
		AssetPickerConfig.HiddenColumnNames.Add(AssetRegistryTag.Name.ToString());
	}

	FContentBrowserModule & ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TSharedRef<SWidget> AssetPicker = ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig);

	ChildSlot[
		SNew(SVerticalBox)

			// Title text
			+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2)
				.HAlign(HAlign_Fill)
				[
					SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
							[
								SNew(STextBlock)
									.Text(InArgs._Title)
									.Font(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 16))
									.AutoWrapText(true)
							]
				]

			// Help description text
			+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2)
				.HAlign(HAlign_Fill)
				[
					SNew(STextBlock)
						.Text(InArgs._Description)
						.AutoWrapText(true)
				]

			+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(5)
				[
					SNew(SSeparator)
				]

			// Asset picker.
			+ SVerticalBox::Slot()
				.MaxHeight(500)
				[
					AssetPicker
				]

			+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(5)
				[
					SNew(SSeparator)
				]

			// Buttons
			+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Bottom)
				[
					SNew(SUniformGridPanel)
						+ SUniformGridPanel::Slot(0, 0)
						[
							SNew(SButton)
								.HAlign(HAlign_Center)
								.Text(LOCTEXT("SRiggedSkeletonPicker_Ok", "Select"))
								.IsEnabled(this, &SRiggedSkeletonPicker::CanSelect)
								.OnClicked(this, &SRiggedSkeletonPicker::OnSelect)
						]
					+ SUniformGridPanel::Slot(1, 0)
						[
							SNew(SButton)
								.HAlign(HAlign_Center)
								.Text(LOCTEXT("SRiggedSkeletonPicker_Cancel", "Cancel"))
								.OnClicked(this, &SRiggedSkeletonPicker::OnCancel)
						]
				]
	];
}



USkeleton * SRiggedSkeletonPicker::GetSelectedSkeleton()
{
	return SelectedSkeleton;
}



void SRiggedSkeletonPicker::OnAssetSelected(const FAssetData & AssetData)
{
	ActiveSkeleton = Cast<USkeleton>(AssetData.GetAsset());
}



void SRiggedSkeletonPicker::OnAssetDoubleClicked(const FAssetData & AssetData)
{
	OnAssetSelected(AssetData);
	OnSelect();
}



bool SRiggedSkeletonPicker::CanSelect() const
{
	return ActiveSkeleton != nullptr;
}



FReply SRiggedSkeletonPicker::OnSelect()
{
	SelectedSkeleton = ActiveSkeleton;
	CloseWindow();
	return FReply::Handled();
}



FReply SRiggedSkeletonPicker::OnCancel()
{
	SelectedSkeleton = nullptr;
	CloseWindow();
	return FReply::Handled();
}



void SRiggedSkeletonPicker::CloseWindow()
{
	TSharedPtr<SWindow> window = FSlateApplication::Get().FindWidgetWindow(AsShared());
	if (window.IsValid())
	{
		window->RequestDestroyWindow();
	}
}



SRootMotionExtractionWidget::SRootMotionExtractionWidget()
    : ActiveAnimationSequence(nullptr),
    ActiveInPlaceAnimationSequence(nullptr),
    SelectedAnimationSequence(nullptr),
    SelectedInPlaceAnimationSequence(nullptr)
{}



void SRootMotionExtractionWidget::Construct(const FArguments& InArgs)
{
    const USkeleton * ReferenceSkeleton = InArgs._ReferenceSkeleton;

    checkf(ReferenceSkeleton != nullptr, TEXT("A reference skeleton must be specified."));

    FText Title = LOCTEXT("SRootMotionExtractionWidget_Title", "Generate Root Motion Animation");
    FText Description = LOCTEXT("SRootMotionExtractionWidget_Description", "You can generate a Root Motion animation from an ordinary Mixamo animation and its in-place version. A new asset will be created.");
	FText NormalAnimPickerDesc = LOCTEXT("SRootMotionExtractionWidget_NormalAnimPickerDescription", "ORDINARY animation.");
	FText InPlaceAnimPickerDesc = LOCTEXT("SRootMotionExtractionWidget_InPlaceAnimPickerDescription", "IN-PLACE animation.");

    ActiveAnimationSequence = nullptr;
    ActiveInPlaceAnimationSequence = nullptr;
    SelectedAnimationSequence = nullptr;
    SelectedInPlaceAnimationSequence = nullptr;

    ChildSlot[
        SNew(SVerticalBox)

			// Title text
			+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2)
				[
					SNew(STextBlock)
						.Text(Title)
						.Font(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 16))
						.AutoWrapText(true)
				]

			// Help description text
			+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2)
				[
					SNew(STextBlock)
						.Text(Description)
						.AutoWrapText(true)
				]

			+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(5)
				[
					SNew(SSeparator)
				]

			// Asset pickers.
			+ SVerticalBox::Slot()
				.FillHeight(1)
				.Padding(2)
				.MaxHeight(500)
				[
					SNew(SHorizontalBox)

						// Picker for "normal" animation
						+ SHorizontalBox::Slot()
							.FillWidth(1)
							[
								SNew(SVerticalBox)

									+ SVerticalBox::Slot()
										.AutoHeight()
										.HAlign(HAlign_Center)
										.Padding(5)
										[
											SNew(STextBlock)
												.Text(NormalAnimPickerDesc)
												.AutoWrapText(true)
										]

									+ SVerticalBox::Slot()
										.FillHeight(1)
										[
											CreateAnimationSequencePicker(ReferenceSkeleton, false)
										]
							]

						+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(5)
							[
								SNew(SSeparator)
							]
						
						// Picker for "in-place" animation
						+ SHorizontalBox::Slot()
							.FillWidth(1)
							[
								SNew(SVerticalBox)

									+ SVerticalBox::Slot()
										.AutoHeight()
										.HAlign(HAlign_Center)
										.Padding(5)
										[
											SNew(STextBlock)
												.Text(InPlaceAnimPickerDesc)
												.AutoWrapText(true)
										]

									+ SVerticalBox::Slot()
										.FillHeight(1)
										[
											CreateAnimationSequencePicker(ReferenceSkeleton, true)
										]
							]
			]

			+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(5)
				[
					SNew(SSeparator)
				]

			// Buttons
			+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Bottom)
				[
					SNew(SUniformGridPanel)

						+ SUniformGridPanel::Slot(0, 0)
							[
								SNew(SButton)
									.HAlign(HAlign_Center)
									.Text(LOCTEXT("SRootMotionExtractionWidget_Ok", "Select"))
									.IsEnabled(this, &SRootMotionExtractionWidget::CanSelect)
									.OnClicked(this, &SRootMotionExtractionWidget::OnSelect)
							]
						
						+ SUniformGridPanel::Slot(1, 0)
							[
								SNew(SButton)
									.HAlign(HAlign_Center)
									.Text(LOCTEXT("SRootMotionExtractionWidget_Cancel", "Cancel"))
									.OnClicked(this, &SRootMotionExtractionWidget::OnCancel)
							]
				]
	];
}



TSharedRef<SWidget> SRootMotionExtractionWidget::CreateAnimationSequencePicker(const USkeleton * ReferenceSkeleton, bool InPlaceAnimation)
{
    auto OnClickDelegate = [this, InPlaceAnimation](const FAssetData & AssetData)
    {
        UAnimSequence* anim = Cast<UAnimSequence>(AssetData.GetAsset());
        if (InPlaceAnimation)
            ActiveInPlaceAnimationSequence = anim;
        else
            ActiveAnimationSequence = anim;
    };

    // Configure the Asset Picker.
    FAssetPickerConfig AssetPickerConfig;
    AssetPickerConfig.Filter.ClassNames.Add(UAnimSequence::StaticClass()->GetFName());
    AssetPickerConfig.Filter.bRecursiveClasses = true;
    if (ReferenceSkeleton != nullptr)
    {
        FString SkeletonString = FAssetData(ReferenceSkeleton).GetExportTextName();
        AssetPickerConfig.Filter.TagsAndValues.Add(FName(TEXT("Skeleton")), SkeletonString);
    }
    AssetPickerConfig.SelectionMode = ESelectionMode::Single;
    AssetPickerConfig.OnAssetSelected.BindLambda(OnClickDelegate);
    AssetPickerConfig.AssetShowWarningText = LOCTEXT("SRootMotionExtractionWidget_NoAnimations", "No Animation asset for the selected Skeleton found!");
    // Aesthetic settings.
    AssetPickerConfig.OnAssetDoubleClicked.BindLambda(OnClickDelegate);
    AssetPickerConfig.InitialAssetViewType = EAssetViewType::Column;
    AssetPickerConfig.bShowPathInColumnView = true;
    AssetPickerConfig.bShowTypeInColumnView = false;
    // Hide all asset registry columns by default (we only really want the name and path)
    TArray<UObject::FAssetRegistryTag> AssetRegistryTags;
    UAnimSequence::StaticClass()->GetDefaultObject()->GetAssetRegistryTags(AssetRegistryTags);
    for (UObject::FAssetRegistryTag & AssetRegistryTag : AssetRegistryTags)
    {
        AssetPickerConfig.HiddenColumnNames.Add(AssetRegistryTag.Name.ToString());
    }

    FContentBrowserModule & ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
    return ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig);
}



bool SRootMotionExtractionWidget::CanSelect() const
{
    return ActiveAnimationSequence != nullptr
        && ActiveInPlaceAnimationSequence != nullptr
        && ActiveAnimationSequence != ActiveInPlaceAnimationSequence;
}



FReply SRootMotionExtractionWidget::OnSelect()
{
    SelectedAnimationSequence = ActiveAnimationSequence;
    SelectedInPlaceAnimationSequence = ActiveInPlaceAnimationSequence;
    CloseWindow();
    return FReply::Handled();
}



FReply SRootMotionExtractionWidget::OnCancel()
{
    SelectedAnimationSequence = nullptr;
    SelectedInPlaceAnimationSequence = nullptr;
    CloseWindow();
    return FReply::Handled();
}



void SRootMotionExtractionWidget::CloseWindow()
{
    TSharedPtr<SWindow> window = FSlateApplication::Get().FindWidgetWindow(AsShared());
    if (window.IsValid())
    {
        window->RequestDestroyWindow();
    }
}

SOverridingAssetsConfirmationDialog::SOverridingAssetsConfirmationDialog()
	: bConfirmed(false)
{
}

FReply SOverridingAssetsConfirmationDialog::OnConfirm()
{
	bConfirmed = true;
	CloseWindow();
	return FReply::Handled();
}

FReply SOverridingAssetsConfirmationDialog::OnCancel()
{
	bConfirmed = false;
	CloseWindow();
	return FReply::Handled();
}

void SOverridingAssetsConfirmationDialog::CloseWindow()
{
	TSharedPtr<SWindow> window = FSlateApplication::Get().FindWidgetWindow(AsShared());
	if (window.IsValid())
	{
		window->RequestDestroyWindow();
	}
}

bool SOverridingAssetsConfirmationDialog::EnumerateCustomSourceItemDatas(TFunctionRef<bool(FContentBrowserItemData&&)> InCallback)
{
	UContentBrowserDataSubsystem* ContentBrowserDataSubsystem = IContentBrowserDataModule::Get().GetSubsystem();

#if 0
	TArray<FContentBrowserItemPath> SourceItemPaths;
	for (auto Asset : AssetsToOverwrite)
	{
		SourceItemPaths.Add(FContentBrowserItemPath(FAssetData(Asset).PackageName, EContentBrowserPathType::Internal));
	}

	//BUG: assets in memory are not enumerated.
	return ContentBrowserDataSubsystem->EnumerateItemsAtPaths(SourceItemPaths, EContentBrowserItemTypeFilter::IncludeFiles, InCallback);
#else

	IModularFeatures& ModularFeatures = IModularFeatures::Get();
	static const FName BrowserDataSourceTypeName = UContentBrowserDataSource::GetModularFeatureTypeName();

	const int32 NumExtensions = ModularFeatures.GetModularFeatureImplementationCount(BrowserDataSourceTypeName);
	for (int32 ExtensionIndex = 0; ExtensionIndex < NumExtensions; ++ExtensionIndex)
	{
		UContentBrowserDataSource* DataSource = static_cast<UContentBrowserDataSource*>(ModularFeatures.GetModularFeatureImplementation(BrowserDataSourceTypeName, ExtensionIndex));
		if (DataSource->IsA<UContentBrowserAssetDataSource>())
		{
			for (auto Asset : AssetsToOverwrite)
			{
				FAssetData AssetData(Asset);

				FName VirtualizedPath;
				DataSource->TryConvertInternalPathToVirtual(AssetData.ObjectPath, VirtualizedPath);

				InCallback(ContentBrowserAssetData::CreateAssetFileItem(DataSource, VirtualizedPath, AssetData));
			}

			break;
		}
	}

	return true;
#endif
}

void SOverridingAssetsConfirmationDialog::Construct(const FArguments& InArgs)
{
	FText Title = LOCTEXT("SOverridingAssetsConfirmationDialog_Title", "Warning");
	FText Description = LOCTEXT("SOverridingAssetsConfirmationDialog_Description", "Files listed below will be overwritten! Please confirm to continue or cancel to abort the procedure.");

	AssetsToOverwrite = InArgs._AssetsToOverwrite;

	auto LibrarySourceData = MakeShared<FSourcesData>();
	// Provide a dummy invalid virtual path to make sure nothing tries to enumerate root "/"
	LibrarySourceData->VirtualPaths.Add(FName(TEXT("/UMGWidgetTemplateListViewModel")));
	// Disable any enumerate of virtual path folders
	LibrarySourceData->bIncludeVirtualPaths = false;
	// Supply a custom list of source items to display
	LibrarySourceData->OnEnumerateCustomSourceItemDatas.BindSP(this, &SOverridingAssetsConfirmationDialog::EnumerateCustomSourceItemDatas);

	TSharedRef<SAssetView> AssetView = SNew(SAssetView)
		.InitialCategoryFilter(EContentBrowserItemCategoryFilter::IncludeAll)
		.InitialSourcesData(*LibrarySourceData)
		.InitialViewType(EAssetViewType::List)
		//.InitialThumbnailPoolSize(AssetsToOverwrite.Num())
		//.InitialThumbnailSize(EThumbnailSize::Large)
		.ForceShowEngineContent(true)
		.ForceShowPluginContent(true)
		.ShowTypeInTileView(false)
		.ShowViewOptions(false);

	ChildSlot[
		SNew(SVerticalBox)

			// Title text
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2)
			.HAlign(HAlign_Fill)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
			[
				SNew(STextBlock)
				.Text(Title)
				.Font(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 16))
				.AutoWrapText(true)
			]
			]

		// Help description text
		+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2)
			.HAlign(HAlign_Fill)
			[
				SNew(STextBlock)
				.Text(Description)
				.AutoWrapText(true)
			]

		+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(5)
			[
				SNew(SSeparator)
			]

		// Asset viewer.
		+ SVerticalBox::Slot()
			.MaxHeight(500)
			[
				AssetView
			]

		+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(5)
			[
				SNew(SSeparator)
			]

		// Buttons
		+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			[
				SNew(SUniformGridPanel)
				+ SUniformGridPanel::Slot(0, 0)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.Text(LOCTEXT("SRiggedSkeletonPicker_Ok", "Confirm"))
				.OnClicked(this, &SOverridingAssetsConfirmationDialog::OnConfirm)
			]
		+ SUniformGridPanel::Slot(1, 0)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.Text(LOCTEXT("SRiggedSkeletonPicker_Cancel", "Cancel"))
				.OnClicked(this, &SOverridingAssetsConfirmationDialog::OnCancel)
			]
			]
	];

	AssetView->RequestSlowFullListRefresh();
}



#undef LOCTEXT_NAMESPACE
