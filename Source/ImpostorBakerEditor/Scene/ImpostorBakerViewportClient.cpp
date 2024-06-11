#include "ImpostorBakerViewportClient.h"
#include <AdvancedPreviewScene.h>
#include <CanvasItem.h>
#include <CanvasTypes.h>
#include <Engine/Font.h>
#include <ProceduralMeshComponent.h>
#include <SceneView.h>
#include "ImpostorData/ImpostorData.h"
#include "SImpostorBakerViewport.h"
#include "Managers/ImpostorBakerManager.h"
#include "Managers/ImpostorProceduralMeshManager.h"

#define LOCTEXT_NAMESPACE "ImpostorBakerViewportClient"

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

	SetRealtime(true);
	if(GEditor->PlayWorld)
	{
		AddRealtimeOverride(false, LOCTEXT("ImpostorBakerViewport_RealTimeDisableOnPie", "Disable ImpostorBaker Viewport Realtime for PIE"));
	}

	FEditorViewportClient::SetViewMode(VMI_Lit);

	EngineShowFlags.Game = 0;
	EngineShowFlags.ScreenSpaceReflections = 1;
	EngineShowFlags.AmbientOcclusion = 1;
	EngineShowFlags.SetSnap(false);
	EngineShowFlags.Grid = false;
	EngineShowFlags.EnableAdvancedFeatures();

	OverrideNearClipPlane(1.0f);
	bUsingOrbitCamera = true;
}

void FImpostorBakerViewportClient::DrawCanvas(FViewport& InViewport, FSceneView& View, FCanvas& Canvas)
{
	FEditorViewportClient::DrawCanvas(InViewport, View, Canvas);

	const TSharedPtr<SEditorViewport> PinnedViewport = EditorViewportWidget.Pin();
	if (!PinnedViewport)
	{
		return;
	}

	const TSharedPtr<SImpostorBakerViewport> ImpostorBakerViewport = StaticCastSharedPtr<SImpostorBakerViewport>(PinnedViewport);
	if (!ImpostorBakerViewport)
	{
		return;
	}

	const UImpostorData* Data = ImpostorBakerViewport->GetObjectData();
	if (!Data || !Data->bDisplayVertices)
	{
		return;
	}

	const UImpostorBakerManager* Manager = ImpostorBakerViewport->GetManager();
	if (!Manager)
	{
		return;
	}

	UImpostorProceduralMeshManager* MeshManager = Manager->GetManager<UImpostorProceduralMeshManager>();
	if (!MeshManager)
	{
		return;
	}

	const int32 HalfX = Viewport->GetSizeXY().X / 2 / GetDPIScale();
	const int32 HalfY = Viewport->GetSizeXY().Y / 2 / GetDPIScale();

	TSet<FVector> UsedVertices;
	for (const auto& It : MeshManager->Points)
	{
		for (int32 Index = 0; Index < It.Value.Points.Num(); Index++)
		{
			const FVector2D& Point = It.Value.Points[Index];
			const FVector* VertexCoords = It.Value.PointToVertex.Find(Point);
			if (!VertexCoords)
			{
				continue;
			}
			int32 VertexIndex = MeshManager->Vertices.IndexOfByKey(*VertexCoords);
			UsedVertices.Add(*VertexCoords);

			FString Text = LexToString(VertexIndex) + " [" + LexToString(Index) + "] " + Point.ToString();
			const FPlane Projection = View.Project(MeshManager->MeshComponent->GetComponentLocation() + *VertexCoords);
			if (Projection.W > 0.f)
			{
				const int32 XPos = HalfX + (HalfX * Projection.X);
				const int32 YPos = HalfY + (HalfY * (Projection.Y * -1));

				const int32 StringWidth = GEngine->GetSmallFont()->GetStringSize(*Text) / 2;
				const int32 StringHeight = GEngine->GetSmallFont()->GetStringHeightSize(*Text);
				FCanvasTextItem TextItem(FVector2D(XPos - StringWidth, YPos - StringHeight), FText::FromString(Text), GEngine->GetSmallFont(), FLinearColor::White);
				TextItem.EnableShadow(FLinearColor::Black);
				Canvas.DrawItem(TextItem);
			}
		}
	}
	for (int32 Index = 0; Index < MeshManager->Vertices.Num(); Index++)
	{
		const FVector& Vertex = MeshManager->Vertices[Index];
		if (UsedVertices.Contains(Vertex))
		{
			continue;
		}

		FString Text = LexToString(Index);
		const FPlane Projection = View.Project(MeshManager->MeshComponent->GetComponentLocation() + Vertex);
		if (Projection.W > 0.f)
		{
			const int32 XPos = HalfX + (HalfX * Projection.X);
			const int32 YPos = HalfY + (HalfY * (Projection.Y * -1));

			const int32 StringWidth = GEngine->GetSmallFont()->GetStringSize(*Text) / 2;
			const int32 StringHeight = GEngine->GetSmallFont()->GetStringHeightSize(*Text);
			FCanvasTextItem TextItem(FVector2D(XPos - StringWidth, YPos - StringHeight), FText::FromString(Text), GEngine->GetSmallFont(), FLinearColor::White);
			TextItem.EnableShadow(FLinearColor::Black);
			Canvas.DrawItem(TextItem);
		}
	}
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

void FImpostorBakerViewportClient::UpdateCamera(const FBoxSphereBounds& Bounds)
{
	static FRotator CustomOrbitRotation(-33.75f, -0.f, 0.f);
	const FVector CustomOrbitZoom(0.f, Bounds.GetSphere().W / (75.f * PI / 360.f) * 3.f, 0.f);

	bUsingOrbitCamera = true;
	SetCameraSetup(FVector::ZeroVector, CustomOrbitRotation, CustomOrbitZoom, FVector::ZeroVector, FVector::ZeroVector, FRotator::ZeroRotator);
}

#undef LOCTEXT_NAMESPACE
