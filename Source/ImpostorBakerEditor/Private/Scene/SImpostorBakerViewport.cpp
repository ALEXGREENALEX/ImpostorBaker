// Fill out your copyright notice in the Description page of Project Settings.

#include "SImpostorBakerViewport.h"

#include "ImpostorData.h"
#include "ImpostorBakerViewportClient.h"
#include "Managers/ImpostorBakerManager.h"
#include "SImpostorBakerViewportToolBar.h"

#include "AdvancedPreviewScene.h"
#include "ComponentReregisterContext.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "BufferVisualizationMenuCommands.h"
#include "Components/DirectionalLightComponent.h"

void SImpostorBakerViewport::Construct(const FArguments& InArgs)
{
	Object = InArgs._Object;

	AdvancedPreviewScene = MakeShared<FAdvancedPreviewScene>(FPreviewScene::ConstructionValues());
	AdvancedPreviewScene->SetFloorVisibility(false);

	SEditorViewport::Construct(SEditorViewport::FArguments());

	BakerManager = NewObject<UImpostorBakerManager>(GetTransientPackage());
	BakerManager->AssignData(
		Object,
		AdvancedPreviewScene->GetWorld(),
		FImpostorManageComponent::CreateSP(this, &SImpostorBakerViewport::OnAddComponent),
		FImpostorManageComponent::CreateSP(this, &SImpostorBakerViewport::OnDestroyComponent),
		FImpostorForceTick::CreateSP(this, &SImpostorBakerViewport::OnAskForForcedTick),
		FImpostorPopulateOverlay::CreateSP(this, &SImpostorBakerViewport::PopulateOverlayText));
	BakerManager->AssignSkyLight(AdvancedPreviewScene->SkyLight);
	BakerManager->Initialize();

	if (AdvancedPreviewScene->DirectionalLight)
	{
		AdvancedPreviewScene->DirectionalLight->SetVisibility(false);
	}

	Object->OnPropertyChange.FindOrAdd(GET_MEMBER_NAME_CHECKED(UImpostorData, ReferencedMesh), {}).AddSP(this, &SImpostorBakerViewport::UpdateCamera);
}

SImpostorBakerViewport::~SImpostorBakerViewport()
{
	if (SystemViewportClient)
	{
		SystemViewportClient->Viewport = nullptr;
	}

	Object->OnPropertyInteractiveChange = {};
	Object->OnPropertyChange = {};
	Object->OnSettingsChange = {};

	BakerManager->Cleanup();
	BakerManager = nullptr;

	Object = nullptr;
	SystemViewportClient = nullptr;
	AdvancedPreviewScene = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SImpostorBakerViewport::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(Object);
	Collector.AddReferencedObject(BakerManager);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SImpostorBakerViewport::PopulateOverlayText(const TMap<FName, FString>& TextItems) const
{
	FTextBuilder FinalText;

	for (const auto& It : TextItems)
	{
		FinalText.AppendLine(FText::FromString(It.Value));
	}

	FinalText.AppendLine(INVTEXT("<TextBlock.ShadowedText>Changing any parameter not in </><TextBlock.ShadowedTextWarning>Material</><TextBlock.ShadowedText> category, will require rebaking</>"));

	OverlayText->SetText(FinalText.ToText());
	OverlayText->SetVisibility(TextItems.IsEmpty() ? EVisibility::Collapsed : EVisibility::SelfHitTestInvisible);
}

void SImpostorBakerViewport::UpdateCamera() const
{
	SystemViewportClient->UpdateCamera(Object->ReferencedMesh->GetBounds());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<FEditorViewportClient> SImpostorBakerViewport::MakeEditorViewportClient()
{
	SystemViewportClient = MakeShared<FImpostorBakerViewportClient>(*AdvancedPreviewScene.Get(), SharedThis(this));
	SystemViewportClient->SetRealtime(true);
	SystemViewportClient->UpdateCamera(Object->ReferencedMesh->GetBounds());

	return SystemViewportClient.ToSharedRef();
}

TSharedPtr<SWidget> SImpostorBakerViewport::MakeViewportToolbar()
{
	return SNew(SImpostorBakerViewportToolBar, SharedThis(this));
}

void SImpostorBakerViewport::PopulateViewportOverlays(const TSharedRef<SOverlay> Overlay)
{
	SEditorViewport::PopulateViewportOverlays(Overlay);

	Overlay->AddSlot()
		.VAlign(VAlign_Top)
		.HAlign(HAlign_Left)
		.Padding(FMargin(6.f, 36.f, 6.f, 6.f))
		[
			SNew(SBorder)
			.BorderImage( FAppStyle::Get().GetBrush( "FloatingBorder" ) )
			.Padding(4.f)
			[
				SAssignNew(OverlayText, SRichTextBlock)
			]
		];
}

bool SImpostorBakerViewport::IsVisible() const
{
	return
		SEditorViewport::IsVisible() ||
		bIsTickForced;
}

void SImpostorBakerViewport::BindCommands()
{
	SEditorViewport::BindCommands();

	FBufferVisualizationMenuCommands::Get().BindCommands(*CommandList, Client);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SImpostorBakerViewport::OnAddComponent(USceneComponent* SceneComponent) const
{
	if (!ensure(AdvancedPreviewScene))
	{
		return;
	}

	FComponentReregisterContext ReregisterContext(SceneComponent);;
	AdvancedPreviewScene->AddComponent(SceneComponent, FTransform::Identity);
}

void SImpostorBakerViewport::OnDestroyComponent(USceneComponent* SceneComponent) const
{
	if (!ensure(AdvancedPreviewScene))
	{
		return;
	}

	AdvancedPreviewScene->RemoveComponent(SceneComponent);
}

void SImpostorBakerViewport::OnAskForForcedTick(const bool bForceTick)
{
	bIsTickForced = bForceTick;
}