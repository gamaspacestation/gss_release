// Copyright 2020 YetiTech Studios, Pvt Ltd. All Rights Reserved.


#include "InternetBrowser.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Async/TaskGraphInterfaces.h"
#include "UObject/ConstructorHelpers.h"
#include "WebBrowserModule.h"
#include "WebBrowserWidget/Public/WebBrowser.h"
#include "WebBrowser/Public/SWebBrowser.h"
#include "WebBrowser/Public/IWebBrowserCookieManager.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include <regex>

#include "InternetBrowserHistoryManager.h"
#include "InternetBrowserSaveGame.h"

static const FName BROWSER_IDENTIFIER_FAILSAFE = FName("browser");
static const FString DEFAULT_URL = "about:blank";

typedef struct FWebBrowserCookie FCookie;

DEFINE_LOG_CATEGORY_STATIC(LogInternetBrowser, All, All)

#define IBROWSER_LOG(Param1)			UE_LOG(LogInternetBrowser, Log,  TEXT("%s"), *FString(Param1));
#define IBROWSER_ERR(Param1)			UE_LOG(LogInternetBrowser, Error,  TEXT("%s"), *FString(Param1));

#define LOCTEXT_NAMESPACE "InternetBrowser"

bool FInternetBrowserHistory::IsValid() const
{
	return URL.IsEmptyOrWhitespace() == false && URL.ToString().Equals(DEFAULT_URL, ESearchCase::IgnoreCase) == false;
}

UInternetBrowser::UInternetBrowser(const FObjectInitializer& ObjectInitializer)
{
	InitialURL = FString("https://google.com");
	bSupportsTransparency = false;
	bShowWhitelistOnly = false;
	bAllowURLMasking = false;
	bUrlMaskIsPersistent = true;
	bOnlyHTTPS = true;
	bEnableHistory = true;
	bSupportBrowserURLs = true;
	bSupportLocalhost = true;
	BrowserIdentifier = FName("internetbrowser");
	InternetBrowserSaveGameClass = UInternetBrowserSaveGame::StaticClass();
}

TSharedRef<SWidget> UInternetBrowser::RebuildWidget()
{
	if (IsDesignTime())
	{
		return SNew(SBox)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("InternetBrowser", "Internet Browser by YetiTech Studios"))
		];
	}

	WebBrowserWidget = SNew(SWebBrowser)
		.InitialURL(DEFAULT_URL)
		.ShowControls(false)
		.SupportsTransparency(bSupportsTransparency)
		.OnUrlChanged(BIND_UOBJECT_DELEGATE(FOnTextChanged, HandleOnUrlChanged))
		.OnBeforePopup(BIND_UOBJECT_DELEGATE(FOnBeforePopupDelegate, HandleOnBeforePopup))
		.OnLoadStarted(BIND_UOBJECT_DELEGATE(FSimpleDelegate, HandleOnLoadStart))
		.OnLoadCompleted(BIND_UOBJECT_DELEGATE(FSimpleDelegate, HandleOnLoadComplete))
		.OnLoadError(BIND_UOBJECT_DELEGATE(FSimpleDelegate, HandleOnLoadError));

	UInternetBrowserSaveGame* LoadGame = UInternetBrowserSaveGame::LoadBrowser(this);
	if (LoadGame)
	{
		Bookmarks = LoadGame->GetSavedBookmarks();
	}
	return WebBrowserWidget.ToSharedRef();
}

void UInternetBrowser::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	WebBrowserWidget.Reset();
}

void UInternetBrowser::InitializeInternetBrowser(const FString InOverrideURL /*= ""*/)
{
	if (ReloadButton)
	{
		if (ReloadButton->OnClicked.IsBound())
		{
			ReloadButton->OnClicked.Clear();
		}
		
		ReloadButton->OnClicked.AddDynamic(this, &UInternetBrowser::ReloadWebPage);
		IBROWSER_LOG("Reload button delegate assigned.");
	}
	else
	{
		IBROWSER_ERR("Reload button was not found. Make sure you have a button and it is set to ReloadButton.");
	}

	if (BackButton)
	{
		if (BackButton->OnClicked.IsBound())
		{
			BackButton->OnClicked.Clear();
		}

		BackButton->OnClicked.AddDynamic(this, &UInternetBrowser::Internal_GoBack);
		IBROWSER_LOG("Back button delegate assigned.");		
	}
	else
	{
		IBROWSER_ERR("Back button was not found. Make sure you have a button and it is set to BackButton.");
	}

	if (ForwardButton)
	{
		if (ForwardButton->OnClicked.IsBound())
		{
			ForwardButton->OnClicked.Clear();
		}

		ForwardButton->OnClicked.AddDynamic(this, &UInternetBrowser::Internal_GoForward);
		IBROWSER_LOG("Forward button delegate assigned.");
	}
	else
	{
		IBROWSER_ERR("Forward button was not found. Make sure you have a button and it is set to ForwardButton.");
	}

	if (Addressbar == nullptr)
	{
		IBROWSER_ERR("Address bar was not found. Make sure you have a text box and it is set to Addressbar.");
	}

	LoadURL(FText::FromString(InOverrideURL.IsEmpty() ? InitialURL : InOverrideURL));
}

bool UInternetBrowser::LoadURL(const FText& URL)
{
	if (URL.IsEmptyOrWhitespace() == false)
	{
		FString NewURL = URL.ToString();
		LastLoadedURL = NewURL;

		if (bShowWhitelistOnly)
		{
			bool bDenyAccess = true;
			for (const FString& It : WhitelistWebsites)
			{
				if (NewURL.Contains(It))
				{
					bDenyAccess = false;
					break;
				}
			}

			if (bDenyAccess)
			{
				OnAccessDenied.Broadcast();
				Addressbar->SetText(URL);
				return false;
			}
		}

		const bool bIsBrowserURL = Internal_IsBrowserURL(NewURL);
		if (bIsBrowserURL)
		{
			OnLoadBrowserURL.Broadcast();
			return true;
		}

		const bool bIsLocalhost = IsLocalhostURL(NewURL);		
		if (bIsLocalhost == false)
		{
			if (bOnlyHTTPS && NewURL.StartsWith("http://"))
			{
				NewURL.ReplaceInline(TEXT("http://"), TEXT("https://"));
			}

			const bool bIsValidURL = Internal_IsURLValid(NewURL);
			if (bIsValidURL)
			{
				FString MaskedURL = "";
				if (Internal_FindMaskedURL(URL, MaskedURL))
				{
					NewURL = MaskedURL;
				}

				if (NewURL.StartsWith("http") == false)
				{
					NewURL = "https://" + NewURL;
				}
			}
			else
			{
				NewURL = FString::Printf(TEXT("https://www.google.com/search?q=%s"), *URL.ToString().Replace(TEXT(" "), TEXT("+")));
			}
		}

		if (WebBrowserWidget.IsValid())
		{
			WebBrowserWidget->LoadURL(NewURL);
			return true;
		}
	}

	return false;
}

void UInternetBrowser::LoadString(FString Contents, FString DummyURL)
{
	if (WebBrowserWidget.IsValid())
	{
		WebBrowserWidget->LoadString(Contents, DummyURL);
	}
}

void UInternetBrowser::ExecuteJavascript(const FString& ScriptText)
{
	if (WebBrowserWidget.IsValid())
	{
		WebBrowserWidget->ExecuteJavascript(ScriptText);
	}
}

void UInternetBrowser::AddBookmark(const FBrowserBookmark& InBookmark)
{
	Bookmarks.Add(InBookmark);
	UInternetBrowserSaveGame::SaveBrowser(this);
}

void UInternetBrowser::RemoveBookmark(const FBrowserBookmark& InBookmark)
{
	Bookmarks.RemoveSingle(InBookmark);
	UInternetBrowserSaveGame::SaveBrowser(this);
}

void UInternetBrowser::SetCookie(const FString& URL, const FBrowserCookie& InCookie, FOnCookieSetComplete Delegate)
{
	IWebBrowserSingleton* BrowserSingleton = IWebBrowserModule::Get().GetSingleton();
	TSharedPtr<class IWebBrowserCookieManager> BrowserCookie = BrowserSingleton->GetCookieManager();

	FCookie NewCookie;
	NewCookie.bHasExpires = (bool) InCookie.bExpires;
	NewCookie.bHttpOnly = (bool) InCookie.bHttpRequestOnly;
	NewCookie.bSecure = (bool) InCookie.bHttpsRequestsOnly;
	NewCookie.Domain = InCookie.Domain;
	NewCookie.Expires = InCookie.ExpireTime;
	NewCookie.Name = InCookie.Name;
	NewCookie.Path = InCookie.Path;
	NewCookie.Value = InCookie.Value;

	TFunction<void(bool)> OnSetFunc = [&](bool bSuccess)
	{
		Delegate.ExecuteIfBound(bSuccess);
	};

	BrowserCookie->SetCookie(URL, NewCookie, OnSetFunc);
}

void UInternetBrowser::SetCookieForAll(const FBrowserCookie& InCookie, FOnCookieSetComplete Delegate)
{
	SetCookie("", InCookie, Delegate);
}

void UInternetBrowser::DeleteCookie(const FString& URL, const FString& CookieName, FOnCookieDeleteComplete Delegate)
{
	IWebBrowserSingleton* BrowserSingleton = IWebBrowserModule::Get().GetSingleton();
	TSharedPtr<class IWebBrowserCookieManager> BrowserCookie = BrowserSingleton->GetCookieManager();

	TFunction<void(int)> OnDeleteFunc = [&](int Total)
	{
		Delegate.ExecuteIfBound(Total);
	};
	BrowserCookie->DeleteCookies(URL, CookieName, OnDeleteFunc);
}

void UInternetBrowser::DeleteAllCookies(FOnCookieDeleteComplete Delegate)
{
	DeleteCookie("", "", Delegate);
}

FText UInternetBrowser::GetTitleText() const
{
	if (WebBrowserWidget.IsValid())
	{
		return WebBrowserWidget->GetTitleText();
	}

	return FText::GetEmpty();
}

FString UInternetBrowser::GetUrl() const
{
	if (WebBrowserWidget.IsValid())
	{
		return WebBrowserWidget->GetUrl();
	}

	return FString();
}

FText UInternetBrowser::GetAddressbarUrl() const
{
	if (WebBrowserWidget.IsValid())
	{
		return WebBrowserWidget->GetAddressBarUrlText();
	}

	return FText::GetEmpty();
}

FText UInternetBrowser::GetCleanDomainName(const FText& InURL)
{
	const std::string SearchString = TCHAR_TO_UTF8(*InURL.ToString());
	static const std::regex RegexPattern("(www)?[a-z0-9]+([\\-\\.]{1}[a-z0-9]+)*\\.[a-z]{2,5}(:[0-9]{1,5})?");
	std::smatch RegexMatch;
	std::regex_search(SearchString, RegexMatch, RegexPattern);
	const FString CleanDomainName = RegexMatch.str().c_str();
	return FText::FromString(CleanDomainName.Replace(TEXT("www."), TEXT("")));
}

FString UInternetBrowser::GetBrowserProtocolLink() const
{
	const FName Identifier = BrowserIdentifier.IsNone() ? BROWSER_IDENTIFIER_FAILSAFE : BrowserIdentifier;
	return FString::Printf(TEXT("%s://"), *Identifier.ToString());
}

TArray<FInternetBrowserHistory> UInternetBrowser::GetHistory() const
{
	return UInternetBrowserHistoryManager::GetHistoryManager()->History;
}

void UInternetBrowser::ReloadWebPage()
{
	if (WebBrowserWidget.IsValid())
	{
		if (WebBrowserWidget->IsLoading())
		{
			WebBrowserWidget->StopLoad();
		}
		else
		{
			WebBrowserWidget->Reload();
		}
	}
}

void UInternetBrowser::HandleOnUrlChanged(const FText& Text)
{
	OnUrlChanged.Broadcast(Text);
	if (bShowWhitelistOnly)
	{
		FString NewURL = Text.ToString();
		if (NewURL.Contains(InitialURL) || NewURL.Equals(DEFAULT_URL, ESearchCase::IgnoreCase))
		{
			return;
		}

		for (const FString& It : WhitelistWebsites)
		{
			if (NewURL.Contains(It))
			{
				return;
			}
		}

		OnAccessDenied.Broadcast();
		Addressbar->SetText(Text);
	}
}

void UInternetBrowser::HandleOnLoadStart()
{
	OnLoadStarted.Broadcast();
}

void UInternetBrowser::HandleOnLoadComplete()
{
	if (WebBrowserWidget.IsValid())
	{
		if (Addressbar)
		{
			FText AddressbarURL = WebBrowserWidget->GetAddressBarUrlText();

			FString MaskedURL = "";
			if (Internal_FindMaskedURL(FText::FromString(LastLoadedURL), MaskedURL))
			{
				FString AddressbarUrlString = AddressbarURL.ToString();
				FString MaskedDomainName = "";
				bool bDomainNameFound = false;
				for (const auto& It : MaskedDomains)
				{
					for (const auto& ItDomainName : It.Key.CustomDomainNames)
					{
						if (ItDomainName.Equals(LastLoadedURL))
						{
							bDomainNameFound = true;
							MaskedDomainName = ItDomainName;
							break;
						}
					}

					if (bDomainNameFound)
					{
						break;
					}
				}

				AddressbarURL = FText::FromString(AddressbarUrlString.Replace((TEXT("%s"), *MaskedURL), (TEXT("%s"), *MaskedDomainName)));
			}

			Addressbar->SetText(AddressbarURL);
		}

		if (bEnableHistory)
		{
			UInternetBrowserHistoryManager::GetHistoryManager()->History.Add(FInternetBrowserHistory(WebBrowserWidget->GetTitleText(), WebBrowserWidget->GetUrl()));
		}

		if (BackButton && ForwardButton)
		{
			BackButton->SetIsEnabled(WebBrowserWidget->CanGoBack());
			ForwardButton->SetIsEnabled(WebBrowserWidget->CanGoForward());
		}
	}

	OnLoadCompleted.Broadcast();
}

void UInternetBrowser::HandleOnLoadError()
{
	OnLoadError.Broadcast();
}

bool UInternetBrowser::HandleOnBeforePopup(FString URL, FString Frame)
{
	if (OnBeforePopup.IsBound())
	{
		if (IsInGameThread())
		{
			OnBeforePopup.Broadcast(URL, Frame);
		}
		else
		{
			// Retry on the GameThread.
			TWeakObjectPtr<UInternetBrowser> WeakThis = this;
			FFunctionGraphTask::CreateAndDispatchWhenReady([WeakThis, URL, Frame]()
			{
				if (WeakThis.IsValid())
				{
					WeakThis->HandleOnBeforePopup(URL, Frame);
				}
			}, TStatId(), nullptr, ENamedThreads::GameThread);
		}

		return true;
	}

	return false;
}

void UInternetBrowser::Internal_GoBack()
{
	if (WebBrowserWidget.IsValid() && ensureMsgf(WebBrowserWidget->CanGoBack(), TEXT("Tried to navigate backward but widget does not allow navigating backward.")))
	{
		WebBrowserWidget->GoBack();
	}
}

void UInternetBrowser::Internal_GoForward()
{
	if (WebBrowserWidget.IsValid() && ensureMsgf(WebBrowserWidget->CanGoForward(), TEXT("Tried to navigate forward but widget does not allow navigating forward.")))
	{
		WebBrowserWidget->GoForward();
	}
}

const bool UInternetBrowser::Internal_IsURLValid(const FString& InURL) const
{
	static const std::regex RegPattern("^((http|https):\\/\\/)?(www.)?(?!.*(http|https|www.))[a-zA-Z0-9_-]+(\\.[a-zA-Z0-9]+)+((\\/)[\\w#]+)*(\\/\\w+\\?[a-zA-Z0-9_]+=\\w+(&[a-zA-Z0-9_]+=\\w+)*)?.+$");
	return std::regex_match(TCHAR_TO_UTF8(*InURL), RegPattern);
}

const bool UInternetBrowser::Internal_IsBrowserURL(const FString& InURL) const
{
	return bSupportBrowserURLs && InURL.StartsWith(GetBrowserProtocolLink());
}

const bool UInternetBrowser::Internal_FindMaskedURL(const FText& InURL, FString& OutMaskedURL) const
{
	if (MaskedDomains.Num() > 0)
	{
		const FString UrlText = InURL.ToLower().ToString();
		bool bCustomURLFound = false;
		for (const auto& It : MaskedDomains)
		{
			for (const auto& ItString : It.Key.CustomDomainNames)
			{
				if (UrlText.Contains(ItString, ESearchCase::IgnoreCase))
				{
					bCustomURLFound = true;
					OutMaskedURL = It.Value;
					return true;
				}
			}
		}
	}

	OutMaskedURL.Empty();
	return false;
}

#undef IBROWSER_LOG
#undef IBROWSER_ERR
#undef LOCTEXT_NAMESPACE
