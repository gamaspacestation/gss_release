// Copyright 2022 UNAmedia. All Rights Reserved.

#include "MixamoToolkitStyle.h"
#include "MixamoToolkitPrivatePCH.h"
#include "Slate/SlateGameResources.h"
#include "Styling/SlateStyleRegistry.h"
#include "Interfaces/IPluginManager.h"
#include "Framework/Application/SlateApplication.h"


#define PLUGIN_NAME "MixamoAnimationRetargeting"



TSharedPtr< FSlateStyleSet > FMixamoToolkitStyle::StyleInstance = NULL;



void FMixamoToolkitStyle::Initialize()
{
	if (! StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}



void FMixamoToolkitStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}



FName FMixamoToolkitStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT(PLUGIN_NAME "Style"));
	return StyleSetName;
}



#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define TTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".ttf") ), __VA_ARGS__ )
#define OTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".otf") ), __VA_ARGS__ )

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);


TSharedRef< FSlateStyleSet > FMixamoToolkitStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet(PLUGIN_NAME "Style"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin(PLUGIN_NAME)->GetBaseDir() / TEXT("Resources"));

	// Define the styles for the module's actions.
	// For commands: the command name/id must match the style's property name.
	Style->Set(PLUGIN_NAME ".RetargetMixamoSkeleton", new IMAGE_BRUSH(TEXT("ButtonIcon_40x"), Icon40x40));
	Style->Set(PLUGIN_NAME ".ExtractRootMotion", new IMAGE_BRUSH(TEXT("ButtonIcon_40x"), Icon40x40));
	
	Style->Set("ContentBrowser.AssetActions", new IMAGE_BRUSH(TEXT("ButtonIcon_40x"), Icon40x40));
	
	return Style;
}


#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef TTF_FONT
#undef OTF_FONT



void FMixamoToolkitStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}



const ISlateStyle& FMixamoToolkitStyle::Get()
{
	return *StyleInstance;
}
