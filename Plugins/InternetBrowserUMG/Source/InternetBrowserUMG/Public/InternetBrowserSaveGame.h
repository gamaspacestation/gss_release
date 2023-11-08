// Copyright 2020 YetiTech Studios, Pvt Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "InternetBrowserSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class INTERNETBROWSERUMG_API UInternetBrowserSaveGame : public USaveGame
{
	GENERATED_BODY()

private:

	UPROPERTY(EditAnywhere, Category = "Internet Browser Save Game", meta = (AllowPrivateAccess = "true"))
	FString SaveSlotName;

	UPROPERTY(EditAnywhere, Category = "Internet Browser Save Game", meta = (AllowPrivateAccess = "true"))
	int32 UserIndex;

	UPROPERTY()
	TArray<struct FBrowserBookmark> Bookmarks;

public:

	UInternetBrowserSaveGame();

	static bool SaveBrowser(const class UInternetBrowser* InInternetBrowserWidget);
	static UInternetBrowserSaveGame* LoadBrowser(const class UInternetBrowser* InInternetBrowserWidget);

	FORCEINLINE TArray<struct FBrowserBookmark> GetSavedBookmarks() const { return Bookmarks; }
};
