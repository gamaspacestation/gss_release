// Copyright 2020 YetiTech Studios, Pvt Ltd. All Rights Reserved.


#include "InternetBrowserHistoryManager.h"

UInternetBrowserHistoryManager* UInternetBrowserHistoryManager::HistoryManager = nullptr;

UInternetBrowserHistoryManager* UInternetBrowserHistoryManager::GetHistoryManager()
{
	if (HistoryManager == nullptr)
	{
		HistoryManager = NewObject<UInternetBrowserHistoryManager>();
		HistoryManager->AddToRoot();
	}

	return HistoryManager;
}

void UInternetBrowserHistoryManager::DestroyHistoryManager()
{
	if (HistoryManager)
	{
		HistoryManager->RemoveFromRoot();
		HistoryManager->ConditionalBeginDestroy();
		HistoryManager = nullptr;
	}
}

void UInternetBrowserHistoryManager::DeleteHistory(const FInternetBrowserHistory& InHistory)
{
	History.RemoveSingle(InHistory);
}
