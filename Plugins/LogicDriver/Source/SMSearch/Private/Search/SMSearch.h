// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#pragma once

#include "ISMSearch.h"

#include "AssetRegistry/AssetData.h"
#include "Async/AsyncWork.h"

struct FActiveSearch;
class FRegexPattern;

typedef TSharedPtr<FActiveSearch, ESPMode::ThreadSafe> FActiveSearchPtr;
typedef TSharedRef<FActiveSearch, ESPMode::ThreadSafe> FActiveSearchRef;

class FSMSearch final : public ISMSearch
{
public:
	class FSearchAsyncTask : public FNonAbandonableTask
	{
	public:
		FSMSearch* SearchInstance;
		TWeakPtr<FActiveSearch, ESPMode::ThreadSafe> ActiveSearch;

		FSearchAsyncTask(FSMSearch* InSearchInstance, const FActiveSearchPtr InActiveSearch);

		TStatId GetStatId() const;
		void DoWork();
	};

	// ISMSearch
	virtual ~FSMSearch() override;
	virtual FDelegateHandle SearchAsync(const FSearchArgs& InArgs, const FOnSearchCompleted& InOnSearchCompletedDelegate,
		const FOnSearchUpdated& InOnSearchUpdatedDelegate, const FOnSearchCanceled& InOnSearchCanceledDelegate) override;
	virtual void CancelAsyncSearch(const FDelegateHandle& InDelegateHandle) override;
	virtual FReplaceSummary ReplacePropertyValues(const FReplaceArgs& InReplaceArgs, const FSearchArgs& InSearchArgs) override;
	virtual bool EnableDeferredIndexing(bool bEnable) override;
	virtual void GetIndexingStatus(FIndexingStatus& OutIndexingStatus) override;
	// ~ISMSearch

	void RunSearch(FActiveSearchRef InActiveSearch);

	void BroadcastSearchUpdated(FActiveSearchRef InActiveSearch);
	void BroadcastSearchComplete(FActiveSearchRef InActiveSearch);
	void BroadcastSearchCanceled(FActiveSearchRef InActiveSearch);

private:
	/** Destroy the indexer UE manages. */
	bool ShutdownIndexer();

	/** Make sure everything is cleaned up. */
	void FinishSearch(FActiveSearchRef InActiveSearch);

	void SearchStateMachine(USMBlueprint* InBlueprint, FActiveSearchRef InActiveSearch, TArray<TSharedPtr<FSearchResult>>& OutResults) const;
	void SearchObject(UObject* InObject, FActiveSearchRef InActiveSearch, TArray<TSharedPtr<FSearchResult>>& OutResults) const;
	TSharedPtr<FSearchResult> SearchProperty(FProperty* InProperty, UObject* InObject, FActiveSearchRef InActiveSearch, int32 InPropertyIndex = 0) const;

	/** Search a string and record any matches under a single result. */
	static TSharedPtr<FSearchResult> SearchString(const FString& InString, FActiveSearchRef InActiveSearch);

	/** Checks if the asset shouldn't be included. */
	static bool IsAssetFilteredOut(const FAssetData& InAssetData, const FSearchArgs& InArgs);

	/** Create a regex pattern from the given args. */
	static TSharedPtr<FRegexPattern> CreateRegexPattern(const FSearchArgs& InArgs);

	/** Locate the child result that contains the default value string. */
	static bool FindDefaultValueResult(const TSharedPtr<FFindInBlueprintsResult>& InResult, TArray<TSharedPtr<FFindInBlueprintsResult>>& OutValueResults);

	/** Find the parent class node. */
	static TSharedPtr<FFindInBlueprintsResult> FindParentResult(const TSharedPtr<FFindInBlueprintsResult>& InResult);

	/** Find the UEdGraphNode result. */
	static TSharedPtr<FFindInBlueprintsResult> FindNodeResult(const TSharedPtr<FFindInBlueprintsResult>& InDefaultValueResult);

	/** Locate the child result containing property information. */
	static TSharedPtr<FSearchResultFiB> CreateFiBResult(const TSharedPtr<FFindInBlueprintsResult>& InDefaultValueResult, const TSharedPtr<FFindInBlueprintsResult>& TopMostResult);

private:
	/** The default value prefix UE uses. */
	static FString DefaultValuePrefix;

	/** Default value containing LD's node data. */
	static FString NodeDataPrefix;

	/** Each registered delegate mapped to the active search. */
	TMap<FDelegateHandle, FActiveSearchPtr> ActiveSearches;

	/** If set construction scripts should be enabled after searching. */
	bool bReEnableConstructionScriptsOnLoad = false;

	/** The local deferred indexing status which may not be the same as the engine status. */
	TOptional<bool> bDeferredIndexingEnabled;
};
