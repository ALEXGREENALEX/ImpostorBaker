// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"
#include "ImpostorData.h"

class FAssetTypeActions_ImpostorSettings : public FAssetTypeActions_Base
{
	//~ Begin FAssetTypeActions_Base Interface
	virtual uint32 GetCategories() override { return EAssetTypeCategories::Misc; }

	virtual bool HasActions(const TArray<UObject*>& InObjects) const override { return true; }
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor) override;

	virtual FText GetName() const override { return UImpostorData::StaticClass()->GetDisplayNameText(); }
	virtual FColor GetTypeColor() const override { return FColor(255, 140, 0); }
	virtual UClass* GetSupportedClass() const override { return UImpostorData::StaticClass(); }
	//~ End FAssetTypeActions_Base Interface

public:
	static TSharedRef<FExtender> OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& Assets);

private:
	static void OpenImpostorBaking(FAssetData Asset);
};
