// Fill out your copyright notice in the Description page of Project Settings.

#include "AssetTypeActions_ImpostorSettings.h"
#include "ImpostorBakerEditorToolkit.h"
#include "AssetRegistry/AssetRegistryModule.h"

void FAssetTypeActions_ImpostorSettings::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	for (UObject* Object : InObjects)
	{
		const TSharedPtr<FImpostorBakerEditorToolkit> NewEditor = MakeShared<FImpostorBakerEditorToolkit>();
		if (!NewEditor)
		{
			FAssetTypeActions_Base::OpenAssetEditor(InObjects, EditWithinLevelEditor);
			return;
		}

		if (ensure(Object) &&
			ensure(NewEditor))
		{
			const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;
			NewEditor->InitImpostorAssetEditor(Mode, EditWithinLevelEditor, Object);
		}
		else
		{
			FAssetTypeActions_Base::OpenAssetEditor(InObjects, EditWithinLevelEditor);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<FExtender> FAssetTypeActions_ImpostorSettings::OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& Assets)
{
	TSharedRef<FExtender> Extender = MakeShared<FExtender>();

	if (Assets.Num() != 1)
	{
		return Extender;
	}

	for (const FAssetData& Asset : Assets)
	{
		if (!Asset.GetClass()->IsChildOf<UStaticMesh>())
		{
			return Extender;
		}
	}

	Extender->AddMenuExtension(
		"GetAssetActions",
		EExtensionHook::After,
		nullptr,
		FMenuExtensionDelegate::CreateLambda([=](FMenuBuilder& MenuBuilder)
		{
			MenuBuilder.AddMenuEntry(
				INVTEXT("Create Impostor Data"),
				INVTEXT("Creates impostor data asset for impostor baker"),
				FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.ComposureCompositing"),
				FUIAction(FExecuteAction::CreateStatic(&FAssetTypeActions_ImpostorSettings::OpenImpostorBaking, Assets[0])));
		})
	);

	return Extender;
}

void FAssetTypeActions_ImpostorSettings::OpenImpostorBaking(FAssetData Asset)
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	FString PackageName = FPackageName::GetLongPackagePath(Asset.GetPackage()->GetName());
	FString AssetName = FPackageName::GetShortName(Asset.GetPackage()->GetName());
	AssetName.RemoveFromStart("SM_");
	AssetName = "ID_" + AssetName;

	PackageName = FPaths::Combine(PackageName, AssetName);
	AssetTools.CreateUniqueAssetName(PackageName, "", PackageName, AssetName);

	UImpostorData* NewImpostorData = NewObject<UImpostorData>(CreatePackage(*PackageName), *AssetName, RF_Public | RF_Standalone);

	NewImpostorData->MarkPackageDirty();
	NewImpostorData->AssignMesh(Asset);
	NewImpostorData->PostEditChange();

	FAssetRegistryModule::AssetCreated(NewImpostorData);
	TArray<UObject*> ObjectsToSync{NewImpostorData};
	GEditor->SyncBrowserToObjects(ObjectsToSync);

	GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(NewImpostorData);
}