// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SEditorViewport.h"
#include "SCommonEditorViewportToolbarBase.h"

class UImpostorData;
class SRichTextBlock;
class UImpostorBakerManager;
class FAdvancedPreviewScene;
class FImpostorBakerViewportClient;

class IMPOSTORBAKEREDITOR_API SImpostorBakerViewport
	: public SEditorViewport
	, public FGCObject
	, public ICommonEditorViewportToolbarInfoProvider
{
public:
	SLATE_BEGIN_ARGS(SImpostorBakerViewport) {}
		SLATE_ARGUMENT(UImpostorData*, Object)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SImpostorBakerViewport() override;

	//~ Begin FGCObject Interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override
	{
		return TEXT("SImpostorBakerViewport");
	}
	//~ End FGCObject Interface

	//~ Begin ICommonEditorViewportToolbarInfoProvider Interface
	virtual TSharedRef<SEditorViewport> GetViewportWidget() override
	{
		return SharedThis(this);
	}
	virtual TSharedPtr<FExtender> GetExtenders() const override
	{
		return MakeShared<FExtender>();
	}
	virtual void OnFloatingButtonClicked() override {}
	//~ End ICommonEditorViewportToolbarInfoProvider Interface

public:
	UImpostorBakerManager* GetManager() const
	{
		return BakerManager;
	}

	void PopulateOverlayText(const TMap<FName, FString>& TextItems) const;
	void UpdateCamera() const;

protected:
	//~ Begin SEditorViewport Interface
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
	virtual TSharedPtr<SWidget> MakeViewportToolbar() override;
	virtual void PopulateViewportOverlays(TSharedRef<SOverlay> Overlay) override;
	virtual bool IsVisible() const override;
	//~ End SEditorViewport Interface

private:
	void OnAddComponent(USceneComponent* SceneComponent) const;
	void OnDestroyComponent(USceneComponent* SceneComponent) const;
	void OnAskForForcedTick(bool bForceTick);

private:
	TSharedPtr<FAdvancedPreviewScene> AdvancedPreviewScene;
	TSharedPtr<FImpostorBakerViewportClient> SystemViewportClient;
	TSharedPtr<SRichTextBlock> OverlayText;

	UImpostorBakerManager* BakerManager = nullptr;
	UImpostorData* Object = nullptr;
	bool bIsTickForced = false;
};
