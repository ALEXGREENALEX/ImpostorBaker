#include "ImpostorBakerEditorModule.h"

#include "ImpostorDataThumbnailRenderer.h"
#include "Managers/ImpostorBakerManager.h"
#include "AssetTypeActions_ImpostorSettings.h"
#include "Customizations/ImpostorDataCustomization.h"
#include "Customizations/ImpostorBakerManagerCustomization.h"

#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"

void FImpostorBakerEditorModule::StartupModule()
{
	if (FModuleManager::Get().IsModuleLoaded("ContentBrowser"))
	{
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
		TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();

		CBMenuExtenderDelegates.Add(FContentBrowserMenuExtender_SelectedAssets::CreateStatic(&FAssetTypeActions_ImpostorSettings::OnExtendContentBrowserAssetSelectionMenu));
		ContentBrowserAssetsExtenderDelegateHandle = CBMenuExtenderDelegates.Last().GetHandle();
	}

	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		AssetTypeAction = MakeShared<FAssetTypeActions_ImpostorSettings>();
		AssetTools.RegisterAssetTypeActions(AssetTypeAction.ToSharedRef());
	}

	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.RegisterCustomClassLayout(UImpostorBakerManager::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FImpostorBakerManagerCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UImpostorData::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FImpostorDataCustomization::MakeInstance));
	}

	UThumbnailManager::Get().RegisterCustomRenderer(UImpostorData::StaticClass(), UImpostorDataThumbnailRenderer::StaticClass());
}

void FImpostorBakerEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("ContentBrowser"))
	{
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
		TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();

		CBMenuExtenderDelegates.RemoveAll([&](const FContentBrowserMenuExtender_SelectedAssets& Delegate)
		{
			return Delegate.GetHandle() == ContentBrowserAssetsExtenderDelegateHandle;
		});
	}

	if (FModuleManager::Get().IsModuleLoaded("AssetTools") &&
		AssetTypeAction)
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		AssetTools.UnregisterAssetTypeActions(AssetTypeAction.ToSharedRef());
	}

	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomClassLayout(UImpostorBakerManager::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(UImpostorData::StaticClass()->GetFName());
	}

	if (UObjectInitialized())
	{
		UThumbnailManager::Get().UnregisterCustomRenderer(UImpostorData::StaticClass());
	}
}

IMPLEMENT_MODULE(FImpostorBakerEditorModule, ImpostorBakerEditor)