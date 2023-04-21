// Fill out your copyright notice in the Description page of Project Settings.

#include "ImpostorBakerViewportClient.h"
#include "AdvancedPreviewScene.h"
#include "SImpostorBakerViewport.h"
#include "Managers/ImpostorBakerManager.h"

FImpostorBakerViewportClient::FImpostorBakerViewportClient(FAdvancedPreviewScene& InPreviewScene, const TSharedRef<SImpostorBakerViewport>& InViewport)
	: FEditorViewportClient(nullptr, &InPreviewScene, InViewport)
{
	DrawHelper.bDrawPivot = false;
	DrawHelper.bDrawWorldBox = false;
	DrawHelper.bDrawKillZ = false;
	DrawHelper.bDrawGrid = true;
	DrawHelper.GridColorAxis = FColor(80,80,80);
	DrawHelper.GridColorMajor = FColor(72,72,72);
	DrawHelper.GridColorMinor = FColor(64,64,64);
	DrawHelper.PerspectiveGridSize = HALF_WORLD_MAX1;
	ShowWidget(false);

	FEditorViewportClient::SetViewMode(VMI_Lit);

	EngineShowFlags.Game = 0;
	EngineShowFlags.ScreenSpaceReflections = 1;
	EngineShowFlags.AmbientOcclusion = 1;
	EngineShowFlags.SetSnap(0);
	EngineShowFlags.Grid = false;
	EngineShowFlags.EnableAdvancedFeatures();

	OverrideNearClipPlane(1.0f);
	bUsingOrbitCamera = true;
}

void FImpostorBakerViewportClient::Tick(float DeltaSeconds)
{
	FEditorViewportClient::Tick(DeltaSeconds);

	// Tick the preview scene world.
	if (!GIntraFrameDebuggingGameThread)
	{
		PreviewScene->GetWorld()->Tick(LEVELTICK_All, DeltaSeconds);

		if (const TSharedPtr<SEditorViewport> PinnedViewport = EditorViewportWidget.Pin())
		{
			if (const TSharedPtr<SImpostorBakerViewport> ImpostorBakerViewport = StaticCastSharedPtr<SImpostorBakerViewport>(PinnedViewport))
			{
				if (UImpostorBakerManager* Manager = ImpostorBakerViewport->GetManager())
				{
					Manager->Tick();
				}
			}
		}
	}
}

UE::Widget::EWidgetMode FImpostorBakerViewportClient::GetWidgetMode() const
{
	return UE::Widget::WM_Max;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FImpostorBakerViewportClient::UpdateCamera(const FBoxSphereBounds& Bounds)
{
	static FRotator CustomOrbitRotation(-33.75f, -0.f, 0.f);
	const FVector CustomOrbitZoom(0.f, Bounds.GetSphere().W / (75.f * PI / 360.f) * 3.f, 0.f);

	bUsingOrbitCamera = true;
	SetCameraSetup(FVector::ZeroVector, CustomOrbitRotation, CustomOrbitZoom, FVector::ZeroVector, FVector::ZeroVector, FRotator::ZeroRotator);
}