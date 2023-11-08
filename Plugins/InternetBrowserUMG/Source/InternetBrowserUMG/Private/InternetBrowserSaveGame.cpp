// Copyright 2020 YetiTech Studios, Pvt Ltd. All Rights Reserved.


#include "InternetBrowserSaveGame.h"
#include "InternetBrowser.h"
#include "Kismet/GameplayStatics.h"

UInternetBrowserSaveGame::UInternetBrowserSaveGame()
{
	SaveSlotName = "InternetBrowserSave";
	UserIndex = 0;
}

bool UInternetBrowserSaveGame::SaveBrowser(const UInternetBrowser* InInternetBrowserWidget)
{
	if (InInternetBrowserWidget->GetBrowserSaveGameClass())
	{
		UInternetBrowserSaveGame* SaveGameInstance = Cast<UInternetBrowserSaveGame>(UGameplayStatics::CreateSaveGameObject(InInternetBrowserWidget->GetBrowserSaveGameClass()));
		SaveGameInstance->Bookmarks = InInternetBrowserWidget->GetBookmarks();
		return UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->SaveSlotName, SaveGameInstance->UserIndex);
	}

	return false;
}

UInternetBrowserSaveGame* UInternetBrowserSaveGame::LoadBrowser(const UInternetBrowser* InInternetBrowserWidget)
{
	if (InInternetBrowserWidget->GetBrowserSaveGameClass())
	{
		UInternetBrowserSaveGame* LoadGameInstance = Cast<UInternetBrowserSaveGame>(UGameplayStatics::CreateSaveGameObject(InInternetBrowserWidget->GetBrowserSaveGameClass()));
		const FString Local_SlotName = LoadGameInstance->SaveSlotName;
		const int32 Local_Index = LoadGameInstance->UserIndex;
		if (UGameplayStatics::DoesSaveGameExist(Local_SlotName, Local_Index))
		{
			LoadGameInstance = Cast<UInternetBrowserSaveGame>(UGameplayStatics::LoadGameFromSlot(Local_SlotName, Local_Index));
			return LoadGameInstance;
		}
	}
	
	return nullptr;
}
