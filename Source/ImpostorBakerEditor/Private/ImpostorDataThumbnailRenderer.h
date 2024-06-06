// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <CoreMinimal.h>
#include <ThumbnailRendering/StaticMeshThumbnailRenderer.h>
#include "ImpostorDataThumbnailRenderer.generated.h"

class FStaticMeshThumbnailScene;

UCLASS()
class IMPOSTORBAKEREDITOR_API UImpostorDataThumbnailRenderer : public UStaticMeshThumbnailRenderer
{
	GENERATED_BODY()

	//~ Begin UStaticMeshThumbnailRenderer Interface
	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* RenderTarget, FCanvas* Canvas, bool bAdditionalViewFamily) override;
	//~ End UStaticMeshThumbnailRenderer Interface
};