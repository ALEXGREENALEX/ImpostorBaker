// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Misc/NotifyHook.h"
#include "EditorUndoClient.h"

class UImpostorData;
class SImpostorBakerViewport;

class FImpostorBakerEditorToolkit final
	: public FAssetEditorToolkit
	, public FGCObject
	, public FNotifyHook
	, public FEditorUndoClient
{
private:
	struct FTab
	{
		FName TabId;
		FText DisplayName;
		FName Icon;
		TSharedPtr<SWidget> Widget;
	};

public:
	const FString ToolkitName = "ImpostorBakerEditor";

public:
	//~ Begin IToolkit Interface
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual FText GetBaseToolkitName() const override;
	virtual FName GetToolkitFName() const override { return *ToolkitName; }
	virtual FString GetWorldCentricTabPrefix() const override { return ToolkitName; }
	virtual FLinearColor GetWorldCentricTabColorScale() const override { return FLinearColor(0.3f, 0.2f, 0.5f, 0.5f); }
	//~ End IToolkit Interface

	//~ Begin FGCObject Interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override { return ToolkitName; }
	//~ End FGCObject Interface

	void InitImpostorAssetEditor(EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UObject* ObjectToEdit);

private:
	void CreateInternalWidgets();
	static TSharedRef<FTabManager::FLayout> GetLayout();
	void RegisterTabs(TArray<FTab>& OutTabs) const;

	void BuildToolbar(FToolBarBuilder& ToolBarBuilder) const;

private:
	static constexpr const TCHAR* ViewportTabId = TEXT("FBuildingObjectEditorToolkit_Viewport");
	static constexpr const TCHAR* DetailsTabId = TEXT("FBuildingObjectEditorToolkit_Details");
	static constexpr const TCHAR* TransientDetailsTabId = TEXT("FBuildingObjectEditorToolkit_TransientDetails");

	UImpostorData* ObjectBeingEdited = nullptr;
	TArray<FName> RegisteredTabIds;

	TSharedPtr<IDetailsView> DetailsView;
	TSharedPtr<IDetailsView> TransientDetailsView;
	TSharedPtr<SImpostorBakerViewport> Viewport;
};
