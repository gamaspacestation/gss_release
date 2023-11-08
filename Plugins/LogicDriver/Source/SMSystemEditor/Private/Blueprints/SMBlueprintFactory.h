// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#pragma once

#include "UObject/ObjectMacros.h"
#include "Templates/SubclassOf.h"
#include "Engine/Blueprint.h"
#include "Factories/Factory.h"

#include "SMBlueprintFactory.generated.h"

class USMNodeBlueprint;
class USMBlueprint;
class USMInstance;
class USMNodeInstance;
class FSMNewAssetDialogOption;
class SSMAssetPickerList;

UCLASS(HideCategories = Object, MinimalAPI)
class USMBlueprintFactory : public UFactory
{
	GENERATED_UCLASS_BODY()

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnGetNewAssetDialogOptions, TArray<FSMNewAssetDialogOption>& /* OutOptions */)

	// UFactory
	virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	                                  FFeedbackContext* Warn, FName CallingContext) override;
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	                                  FFeedbackContext* Warn) override;
	virtual bool DoesSupportClass(UClass* Class) override;
	virtual FString GetDefaultNewAssetName() const override;
	// ~UFactory

	static void CreateGraphsForBlueprintIfMissing(USMBlueprint* Blueprint);
	static void CreateGraphsForNewBlueprint(USMBlueprint* Blueprint);

	/** Change the parent class the factory should use when creating a new BP. */
	SMSYSTEMEDITOR_API void SetParentClass(TSubclassOf<USMInstance> InNewParent);

	/** Subscribers can add their own options to the new dialog wizard. */
	SMSYSTEMEDITOR_API static FOnGetNewAssetDialogOptions& OnGetNewAssetDialogOptions() { return OnGetNewAssetDialogOptionsEvent; }

	/** If the ConfigureProperties dialog should be displayed. */
	void SetDisplayDialog(bool bNewValue) { bDisplayDialog = bNewValue; }

private:
	enum class ENewAssetType : uint8
	{
		Duplicate,
		Parent
	};

	bool OnCanSelectStateMachineAsset(ENewAssetType InNewAssetType, const TSharedPtr<SSMAssetPickerList> InAssetPicker) const;
	bool OnStateMachineAssetSelectionConfirmed(ENewAssetType InNewAssetType, const TSharedPtr<SSMAssetPickerList> InAssetPicker);

private:
	/** The type of blueprint that will be created. */
	UPROPERTY(EditAnywhere, Category = StateMachineBlueprintFactory)
	TEnumAsByte<EBlueprintType> BlueprintType;

	/** The parent class of the created blueprint. */
	UPROPERTY(EditAnywhere, Category = StateMachineBlueprintFactory, meta=(AllowAbstract = ""))
	TSubclassOf<USMInstance> ParentClass;

	/** A blueprint to be duplicated. */
	UPROPERTY(Transient)
	USMBlueprint* SelectedBlueprintToCopy;

	/** A blueprint to be used as a parent. */
	UPROPERTY(Transient)
	UClass* SelectedClassForParent;

	/** New asset wizard. */
	TSharedPtr<class SSMNewAssetDialog> NewAssetDialog;

	/** Subscribers can add their own options to the new dialog wizard. */
	SMSYSTEMEDITOR_API static FOnGetNewAssetDialogOptions OnGetNewAssetDialogOptionsEvent;

	/** If the ConfigureProperties dialog should be displayed. */
	bool bDisplayDialog = true;
};

UCLASS(HideCategories = Object, MinimalAPI)
class USMNodeBlueprintFactory : public UFactory
{
	GENERATED_UCLASS_BODY()

	// UFactory
	virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	                                  FFeedbackContext* Warn, FName CallingContext) override;
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	                                  FFeedbackContext* Warn) override;
	virtual bool DoesSupportClass(UClass* Class) override;
	virtual FString GetDefaultNewAssetName() const override;
	// ~UFactory

	static SMSYSTEMEDITOR_API void SetupNewBlueprint(USMNodeBlueprint* Blueprint);

	/** Change the parent class the factory should use when creating a new BP. */
	void SMSYSTEMEDITOR_API SetParentClass(TSubclassOf<USMNodeInstance> Class);
private:
	/** The type of blueprint that will be created. */
	UPROPERTY(EditAnywhere, Category = NodeBlueprintFactory)
	TEnumAsByte<EBlueprintType> BlueprintType;

	/** The parent class of the created blueprint. */
	UPROPERTY(EditAnywhere, Category = NodeBlueprintFactory, meta=(AllowAbstract = ""))
	TSubclassOf<USMNodeInstance> ParentClass;

};
