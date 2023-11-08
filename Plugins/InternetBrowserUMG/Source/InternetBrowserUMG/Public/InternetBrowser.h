// Copyright 2020 YetiTech Studios, Pvt Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "InternetBrowser.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWebBrowserLoadStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWebBrowserLoadCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWebBrowserLoadError);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadBrowserURL);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAccessDenied);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUrlChanged, const FText&, Text);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBeforePopup, FString, URL, FString, Frame);

USTRUCT(BlueprintType)
struct FInternetBrowserHistory
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Web History")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Web History")
	FText URL;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Web History")
	FDateTime DateAndTime;

	bool IsValid() const;

	FORCEINLINE bool operator==(const FInternetBrowserHistory& Other) const
	{
		return Title.EqualToCaseIgnored(Other.Title) && URL.EqualToCaseIgnored(Other.URL) && DateAndTime == Other.DateAndTime;
	}

	FORCEINLINE FInternetBrowserHistory()
	{
		Title = FText::GetEmpty();
		URL = FText::GetEmpty();
		DateAndTime = FDateTime::Now();
	}

	FORCEINLINE FInternetBrowserHistory(const FText& InTitle, const FString& InURL)
	{
		Title = InTitle;
		URL = FText::FromString(InURL);
		DateAndTime = FDateTime::Now();
	}
};

// We use struct because TMaps don't support TArrays.
USTRUCT(BlueprintType)
struct FCustomMaskedDomains
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Masked Domain")
	TArray<FString> CustomDomainNames;

	bool operator==(const FCustomMaskedDomains& Other)
	{
		return Other.CustomDomainNames == CustomDomainNames;
	}

	friend uint32 GetTypeHash(const FCustomMaskedDomains& Other)
	{
		return GetTypeHash(1);
	}

	FCustomMaskedDomains() {}
};

USTRUCT(BlueprintType)
struct FBrowserCookie
{
	GENERATED_USTRUCT_BODY();

	/* Cookie name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Browser Cookie")
	FString Name;

	/* Cookie value */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Browser Cookie")
	FString Value;

	/* If empty a host cookie will be created instead of a domain cookie.
	* Domain cookies are stored with a leading "." and are visible to sub-domains whereas host cookies are not.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Browser Cookie")
	FString Domain;

	/* If non-empty only URLS at or below the path will get the cookie value. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Browser Cookie")
	FString Path;

	/* If true, cookie will only be sent for HTTPS requests. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Browser Cookie")
	uint8 bHttpsRequestsOnly = 1;

	/* If true, cookie will only be sent for HTTP requests. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Browser Cookie")
	uint8 bHttpRequestOnly = 1;

	/* If true, expires at specific time. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Browser Cookie")
	uint8 bExpires = 1;

	/* Expiration date. Only valid if bExpires is true. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Browser Cookie")
	FDateTime ExpireTime;
};

USTRUCT(BlueprintType)
struct FBrowserBookmark
{
	GENERATED_USTRUCT_BODY();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Browser Bookmark")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Browser Bookmark")
	FText URL;

	FORCEINLINE bool operator==(const FBrowserBookmark& Other) const
	{
		return Other.Title.EqualToCaseIgnored(Title) && Other.URL.EqualToCaseIgnored(URL);
	}

	FBrowserBookmark() {}
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnCookieSetComplete, bool, bSuccess);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnCookieDeleteComplete, int, bNumberOfCookiesDeleted);

UCLASS()
class INTERNETBROWSERUMG_API UInternetBrowser : public UWidget
{
	GENERATED_BODY()

private:

	/** URL that the browser will initially navigate to. The URL should include the protocol, eg http:// */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Internet Browser", meta = (AllowPrivateAccess = "true"))
	FString InitialURL;

	/** Should the browser window support transparency. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Internet Browser", meta = (AllowPrivateAccess = "true"))
	uint8 bSupportsTransparency : 1;

	/* If enabled, web browser widget will only load websites provided in WhitelistWebsites array. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Internet Browser", meta = (AllowPrivateAccess = "true"))
	uint8 bShowWhitelistOnly : 1;

	/* If enabled, you can mask real domain names with custom names. @See Masked Domains property. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Internet Browser", meta = (AllowPrivateAccess = "true"))
	uint8 bAllowURLMasking : 1;

	/* If enabled, real web site name is always masked out even when you browse sub pages. @See Masked Domains property. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Internet Browser", meta = (EditCondition = "bAllowURLMasking", AllowPrivateAccess = "true"))
	uint8 bUrlMaskIsPersistent : 1;

	/* If enabled, web browser will add visited websites to History. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Internet Browser", meta = (AllowPrivateAccess = "true"))
	uint8 bEnableHistory : 1;

	/* If enabled, web browser will convert http to https. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Internet Browser", meta = (AllowPrivateAccess = "true"), AdvancedDisplay)
	uint8 bOnlyHTTPS : 1;

	/* If enabled, then support browser protocol link similar to chrome://. @See BrowserIdentifier */
	UPROPERTY(EditAnywhere, Category = "Internet Browser", meta = (AllowPrivateAccess = "true"), AdvancedDisplay)
	uint8 bSupportBrowserURLs : 1;

	/** Support localhost and 127.0.0.1 */
	UPROPERTY(EditAnywhere, Category = "Internet Browser", meta = (AllowPrivateAccess = "true"), AdvancedDisplay)
	uint8 bSupportLocalhost : 1;

	/* [NOT YET IMPLEMENTED] An internal identifier used for browser urls. @See UInternetBrowser::GetBrowserProtocolLink() */
	UPROPERTY(VisibleAnywhere, Category = "Internet Browser", meta = (EditCondition = "bSupportBrowserURLs", AllowPrivateAccess = "true"), AdvancedDisplay)
	FName BrowserIdentifier;

	UPROPERTY(EditAnywhere, Category = "Internet Browser", AdvancedDisplay)
	TSubclassOf<class UInternetBrowserSaveGame> InternetBrowserSaveGameClass;

	/* If "Show Whitelist Only" is enabled then Web Browser will only allow to load web pages defined in this array. Accessing any web page not defined in this array will show error page. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Internet Browser", meta = (EditCondition = "bShowWhitelistOnly", AllowPrivateAccess = "true"))
	TArray<FString> WhitelistWebsites;

	/* Allows masking real website urls with custom urls.
	* For example, in FCustomMaskedDomains you can add multiple custom domains like myworldnews.com, somenewsname.com etc. and for value you can set https://www.google.com/search?q=google+news
	* This will make sure that if you navigate to myworldnews.com or somenewsname.com you will end up in https://www.google.com/search?q=google+news
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Internet Browser", meta = (EditCondition = "bAllowURLMasking", AllowPrivateAccess = "true"))
	TMap<FCustomMaskedDomains, FString> MaskedDomains;
	
	/** Browser bookmarks. */
	UPROPERTY(EditAnywhere, Category = "Internet Browser")
	TArray<FBrowserBookmark> Bookmarks;

	UPROPERTY(BlueprintReadWrite, Category = "Internet Browser", meta = (AllowPrivateAccess = "true"))
	class UButton* BackButton;

	UPROPERTY(BlueprintReadWrite, Category = "Internet Browser", meta = (AllowPrivateAccess = "true"))
	class UButton* ForwardButton;

	UPROPERTY(BlueprintReadWrite, Category = "Internet Browser", meta = (AllowPrivateAccess = "true"))
	class UButton* ReloadButton;

	UPROPERTY(BlueprintReadWrite, Category = "Internet Browser", meta = (AllowPrivateAccess = "true"))
	class UEditableTextBox* Addressbar;

	TSharedPtr<class SWebBrowser> WebBrowserWidget;

	UPROPERTY(VisibleInstanceOnly, Category = Debug)
	FString LastLoadedURL;

	UPROPERTY(VisibleInstanceOnly, Category = Debug)
	FBrowserCookie Cookie;

	UPROPERTY()
	TMap<FString, FString> SavedBookmarks;

public:

	UPROPERTY(BlueprintAssignable, Category = "Internet Browser|Events")
	FOnAccessDenied OnAccessDenied;

	/* Called when a new tab needs to spawn. */
	UPROPERTY(BlueprintAssignable, Category = "Internet Browser|Events")
	FOnBeforePopup OnBeforePopup;

	/* Called when URL is changed. */
	UPROPERTY(BlueprintAssignable, Category = "Internet Browser|Events")
	FOnUrlChanged OnUrlChanged;

	/* Called when web browser starts loading a web page. */
	UPROPERTY(BlueprintAssignable, Category = "Internet Browser|Events")
	FOnWebBrowserLoadStarted OnLoadStarted;

	/* Called when web browser finishes loading a web page. */
	UPROPERTY(BlueprintAssignable, Category = "Internet Browser|Events")
	FOnWebBrowserLoadCompleted OnLoadCompleted;

	/* Called when web browser fails to load a web page. */
	UPROPERTY(BlueprintAssignable, Category = "Internet Browser|Events")
	FOnWebBrowserLoadError OnLoadError;

	/* Called when web browser tries to load internal browser URLs. */
	UPROPERTY(BlueprintAssignable, Category = "Internet Browser|Events")
	FOnLoadBrowserURL OnLoadBrowserURL;

	UInternetBrowser(const FObjectInitializer& ObjectInitializer);

	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	/**
	* public static UInternetBrowser::IsHistoryValid
	* Checks if the given history is a valid entry.
	* @param InHistory [const FInternetBrowserHistory&] History struct to check. 
	**/
	UFUNCTION(BlueprintPure, Category = "Internet Browser")
	static bool IsHistoryValid(const FInternetBrowserHistory& InHistory) { return InHistory.IsValid(); }

	/**
	* public UInternetBrowser::InitializeInternetBrowser
	* Assign delegates to back, forward, reload buttons and address bar. Make sure you assign them first.
	* Load initial URL.
	* @param InOverrideURL [const FText] 
	**/
	UFUNCTION(BlueprintCallable, Category = "Internet Browser")
	void InitializeInternetBrowser(const FString InOverrideURL = "");
	
	/**
	* public UInternetBrowser::LoadURL
	* Loads the given URL with support for internal browser URLs.
	* Automatically adds https:// protocol if not present. So if you pass google.com to URL, it will be converted to https://google.com
	* @param URL [const FText &] URL to load. Adds https:// protocol if not present.
	* @return [bool] True if URL is loaded.
	**/
	UFUNCTION(BlueprintCallable, Category = "Internet Browser")
	bool LoadURL(const FText& URL);

	/**
	* public UInternetBrowser::LoadString
	* Load a string as data to create a web page.
	* @param Contents [FString] HTML string to load.
	* @param DummyURL [FString] Dummy URL for the page.
	**/
	UFUNCTION(BlueprintCallable, Category = "Internet Browser")
	void LoadString(FString Contents, FString DummyURL);

	/**
	* public UInternetBrowser::ExecuteJavascript
	* Execute javascript on the current window
	* @param ScriptText [const FString&] Javascript to execute.
	**/
	UFUNCTION(BlueprintCallable, Category = "Internet Browser")
	void ExecuteJavascript(const FString& ScriptText);

	/**
	* public UInternetBrowser::AddBookmark
	* Adds the given bookmark to bookmarks array.
	* @param InBookmark [const FBrowserBookmark&] Bookmark struct.
	**/
	UFUNCTION(BlueprintCallable, Category = "Internet Browser|Bookmark")
    void AddBookmark(const FBrowserBookmark& InBookmark);

	/**
	* public UInternetBrowser::RemoveBookmark
	* Removes the given bookmark.
	* @param InBookmark [const FBrowserBookmark&] Bookmark struct.
	**/
	UFUNCTION(BlueprintCallable, Category = "Internet Browser|Bookmark")
    void RemoveBookmark(const FBrowserBookmark& InBookmark);

	/**
	* public UInternetBrowser::GetBookmarks const
	* Returns all the bookmarks.
	* @return [TArray<FBrowserBookmark>] Bookmarks array.
	**/
	UFUNCTION(BlueprintPure, Category = "Internet Browser|Bookmark")
    inline TArray<FBrowserBookmark> GetBookmarks() const { return Bookmarks; }

	/**
	* public UInternetBrowser::SetCookie
	* Sets a cookie for given URL.
	* This function expects each attribute to be well-formed.
	* It will check for disallowed characters (e.g. the ';' character is disallowed
	* within the cookie Value field) and fail without setting the cookie if such characters are found.
	* @param URL [const FString&] URL to match when searching for cookie to set.
	* @param InCookie [const FBrowserCookie &] Cookie to set.
	* @param Delegate [FOnCookieSetComplete] A callback delegate that will be invoked when the set is complete passing success bool.
	**/
	UFUNCTION(BlueprintCallable, Category = "Internet Browser|Cookies")
	void SetCookie(const FString& URL, const FBrowserCookie& InCookie, UPARAM(DisplayName = "On Cookie Set") FOnCookieSetComplete Delegate);

	/**
	* public UInternetBrowser::SetCookieForAll
	* Sets a cookie for all URLs.
	* This function expects each attribute to be well-formed.
	* It will check for disallowed characters (e.g. the ';' character is disallowed
	* within the cookie Value field) and fail without setting the cookie if such characters are found.
	* @param InCookie [const FBrowserCookie&] Cookie to set.
	* @param Delegate [FOnCookieSetComplete] A callback delegate that will be invoked when the set is complete passing success bool.
	**/
	UFUNCTION(BlueprintCallable, Category = "Internet Browser|Cookies", DisplayName = "Set Cookie (All URLs)")
	void SetCookieForAll(const FBrowserCookie& InCookie, UPARAM(DisplayName = "On Cookie Set") FOnCookieSetComplete Delegate);

	/**
	* public UInternetBrowser::DeleteCookie
	* Deletes the given cookie for the given URL.
	* @param URL [const FString&] URL to match when searching for cookie to remove.
	* @param CookieName [const FString&] The name of the cookie to delete. If left unspecified, all cookies will be removed.
	* @param Delegate [FOnCookieDeleteComplete] A callback delegate that will be invoked when the deletion is complete passing number of deleted cookies.
	**/
	UFUNCTION(BlueprintCallable, Category = "Internet Browser|Cookies")
	void DeleteCookie(const FString& URL, const FString& CookieName, UPARAM(DisplayName = "On Cookies Deleted") FOnCookieDeleteComplete Delegate);

	/**
	* public UInternetBrowser::DeleteAllCookies
	* Delete the entire cookie database.
	* @param Delegate [FOnCookieDeleteComplete] A callback delegate that will be invoked when the deletion is complete passing number of deleted cookies.
	**/
	UFUNCTION(BlueprintCallable, Category = "Internet Browser|Cookies", DisplayName = "Delete Cookie Database")
	void DeleteAllCookies(UPARAM(DisplayName = "On Cookies Deleted") FOnCookieDeleteComplete Delegate);
	
	/**
	* public UInternetBrowser::GetCookieName const
	* Gets the name of the given cookie.
	* @param InCookie [const FBrowserCookie&] Cookie to get the name from.
	* @return [FString] Name of the cookie.
	**/
	UFUNCTION(BlueprintPure, Category = "Internet Browser|Cookies")
	FString GetCookieName(const FBrowserCookie& InCookie) const { return InCookie.Name; }

	/**
	* public UInternetBrowser::GetTitleText const
	* Gets the title of the loaded URL.
	* @return [FText] Title for the loaded URL.
	**/
	UFUNCTION(BlueprintPure, Category = "Internet Browser")
	FText GetTitleText() const;

	/**
	* public UInternetBrowser::GetUrl const
	* Returns the currently loaded URL.
	* @return [FString] Currently loaded URL.
	**/
	UFUNCTION(BlueprintPure, Category = "Internet Browser")
	FString GetUrl() const;

	/**
	* public UInternetBrowser::GetAddressbarUrl const
	* Gets the URL that appears in the address bar, this may not be the URL that is currently loaded in the frame.
	* @return [FText] URL that appears in the address bar
	**/
	UFUNCTION(BlueprintPure, Category = "Internet Browser")
	FText GetAddressbarUrl() const;

	/**
	* public static UInternetBrowser::GetCleanDomainName
	* Returns a clean domain name from given url. For example: www.google.com if you pass https://www.google.com/search?q=test
	* @param InURL [const FText&] URL to get
	* @return [FText] Proper domain name.
	**/
	UFUNCTION(BlueprintPure, Category = "Internet Browser")
	static FText GetCleanDomainName(const FText& InURL);

	/**
	* public UInternetBrowser::GetBrowserProtocolLink const
	* Gets the protocol link for this browser. Similar to chrome://
	* @See BrowserIdentifier variable.
	* @return [FString] Returns browser protocol link.
	**/
	UFUNCTION(BlueprintPure, Category = "Internet Browser")
	FString GetBrowserProtocolLink() const;

	/**
	* public UInternetBrowser::GetHistory const
	* Returns array of FWebHistory. This array contains all the web pages the user has visited if History is enabled.
	* @return [TArray<FWebHistory>] History array.
	**/
	UFUNCTION(BlueprintPure, Category = "Internet Browser")
	TArray<FInternetBrowserHistory> GetHistory() const;

protected:

	UFUNCTION()
	void ReloadWebPage();

	void HandleOnUrlChanged(const FText& Text);
	void HandleOnLoadStart();
	void HandleOnLoadComplete();
	void HandleOnLoadError();

	bool HandleOnBeforePopup(FString URL, FString Frame);

private:

	UFUNCTION()
	void Internal_GoBack();

	UFUNCTION()
	void Internal_GoForward();

	const bool Internal_IsURLValid(const FString& InURL) const;
	const bool Internal_IsBrowserURL(const FString& InURL) const;

	const bool Internal_FindMaskedURL(const FText& InURL, FString& OutMaskedURL) const;

public:

	FORCEINLINE TSubclassOf<class UInternetBrowserSaveGame> GetBrowserSaveGameClass() const { return InternetBrowserSaveGameClass; }
	FORCEINLINE bool IsLocalhostURL(const FString& InURL) const { return bSupportLocalhost && (InURL.Contains("localhost") || InURL.Contains("127.0.0.1")); }
};
