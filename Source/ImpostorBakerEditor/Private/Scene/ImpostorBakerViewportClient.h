// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class FAdvancedPreviewScene;
class SImpostorBakerViewport;

class IMPOSTORBAKEREDITOR_API FImpostorBakerViewportClient : public FEditorViewportClient
{
public:
	FImpostorBakerViewportClient(FAdvancedPreviewScene& InPreviewScene, const TSharedRef<SImpostorBakerViewport>& InViewport);

	//~ Begin FEditorViewportClient Interface
	virtual void DrawCanvas(FViewport& InViewport, FSceneView& View, FCanvas& Canvas) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual bool CanSetWidgetMode(UE::Widget::EWidgetMode NewMode) const override { return false; }
	virtual bool CanCycleWidgetMode() const override { return false; }
	virtual UE::Widget::EWidgetMode GetWidgetMode() const override;
	//~ End FEditorViewportClient Interface

	void UpdateCamera(const FBoxSphereBounds& Bounds);
};
