// Fill out your copyright notice in the Description page of Project Settings.

#include "ImpostorDataThumbnailRenderer.h"
#include "ImpostorData.h"

void UImpostorDataThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* RenderTarget, FCanvas* Canvas, bool bAdditionalViewFamily)
{
	const UImpostorData* ImpostorData = Cast<UImpostorData>(Object);
	if (!ImpostorData)
	{
		return;
	}

	Super::Draw(ImpostorData->ReferencedMesh, X, Y, Width, Height, RenderTarget, Canvas, bAdditionalViewFamily);
}