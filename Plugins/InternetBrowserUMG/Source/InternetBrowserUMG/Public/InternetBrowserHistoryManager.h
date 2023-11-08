// Copyright 2020 YetiTech Studios, Pvt Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InternetBrowser.h"
#include "InternetBrowserHistoryManager.generated.h"

/**
 * 
 */
UCLASS()
class INTERNETBROWSERUMG_API UInternetBrowserHistoryManager : public UObject
{
	GENERATED_BODY()

private:

	static UInternetBrowserHistoryManager* HistoryManager;

public:
	/* An array containing all web pages the user has visited. */
	TArray<FInternetBrowserHistory> History;

	UFUNCTION(BlueprintPure, Category = "Internet Browser")
	static UInternetBrowserHistoryManager* GetHistoryManager();

	UFUNCTION(BlueprintCallable, Category = "Internet Browser")
	static void DestroyHistoryManager();

	UFUNCTION(BlueprintCallable, Category = "Internet Browser")
	void DeleteHistory(const FInternetBrowserHistory& InHistory);
};
