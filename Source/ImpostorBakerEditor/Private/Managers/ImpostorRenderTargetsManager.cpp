// Fill out your copyright notice in the Description page of Project Settings.

#include "ImpostorRenderTargetsManager.h"

#include "ImpostorBakerSettings.h"
#include "ImpostorBakerUtilities.h"
#include "ImpostorMaterialsManager.h"
#include "ImpostorComponentsManager.h"

#include "Engine/Canvas.h"
#include "MaterialInstanceDynamic.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/SceneCaptureComponent2D.h"

void UImpostorRenderTargetsManager::Initialize()
{
	SceneCaptureComponent2D = NewObject<USceneCaptureComponent2D>(GetTransientPackage(), NAME_None, RF_Transient);
	AddComponent(SceneCaptureComponent2D);
}

void UImpostorRenderTargetsManager::Update()
{
	AllocateRenderTargets();
	CreateRenderTargetMips();
	CreateAlphasScratchRenderTargets();
	FillMapsToSave();

	SceneCaptureSetup();
}

void UImpostorRenderTargetsManager::Tick()
{
	if (CurrentMap == EImpostorBakeMapType::None)
	{
		return;
	}

	if (FramesBeforeCapture-- > 0)
	{
		return;
	}

	CaptureImposterGrid();

	if (MapsToBake.Num() == 0)
	{
		FinalizeBaking();
		return;
	}

	PreparePostProcess(MapsToBake.Pop());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UImpostorRenderTargetsManager::AllocateRenderTargets()
{
	TSet<EImpostorBakeMapType> UnusedMaps;
	TargetMaps.GetKeys(UnusedMaps);

	for (EImpostorBakeMapType MapType : ImpostorData->MapsToRender)
	{
		UnusedMaps.Remove(MapType);

		if (const UTextureRenderTarget2D* TargetMap = TargetMaps.FindRef(MapType))
		{
			if (TargetMap->SizeX == ImpostorData->Resolution)
			{
				continue;
			}
		}

		ETextureRenderTargetFormat Format;
		switch (MapType)
		{
		default: check(false);
		case EImpostorBakeMapType::BaseColor:
		case EImpostorBakeMapType::WorldNormal: Format = RTF_RGBA16f; break;
		case EImpostorBakeMapType::Metallic:
		case EImpostorBakeMapType::Specular:
		case EImpostorBakeMapType::Roughness:
		case EImpostorBakeMapType::Opacity:
		case EImpostorBakeMapType::Subsurface:
		case EImpostorBakeMapType::Depth:
		case EImpostorBakeMapType::CustomLighting: Format = RTF_R16f; break;
		}

		UTextureRenderTarget2D* RenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(SceneWorld, ImpostorData->Resolution, ImpostorData->Resolution, Format);
		check(RenderTarget);
		TargetMaps.Add(MapType, RenderTarget);
	}

	for (const EImpostorBakeMapType UnusedMap : UnusedMaps)
	{
		TargetMaps.Remove(UnusedMap);
	}
}

void UImpostorRenderTargetsManager::CreateRenderTargetMips()
{
	for (int32 MipIndex = 0; MipIndex < 8; MipIndex++)
	{
		const int32 MipSize = ImpostorData->SceneCaptureResolution / (1 << MipIndex);
		if (SceneCaptureMipChain.IsValidIndex(MipIndex))
		{
			if (UTextureRenderTarget2D* MipRenderTarget = SceneCaptureMipChain[MipIndex])
			{
				if (MipRenderTarget->SizeX == MipSize)
				{
					continue;
				}

				UKismetRenderingLibrary::ResizeRenderTarget2D(MipRenderTarget, MipSize, MipSize);
				continue;
			}

			SceneCaptureMipChain[MipIndex] = UKismetRenderingLibrary::CreateRenderTarget2D(SceneWorld, MipSize, MipSize, RTF_RGBA16f);
			continue;
		}

		SceneCaptureMipChain.Add(UKismetRenderingLibrary::CreateRenderTarget2D(SceneWorld, MipSize, MipSize, RTF_RGBA16f));
	}
}

void UImpostorRenderTargetsManager::CreateAlphasScratchRenderTargets()
{
	const int32 CombinedAlphasSize = FImpostorBakerUtilities::GetImpostorTypeResolution(ImpostorData->ImpostorType);
	if (!CombinedAlphasRenderTarget ||
		CombinedAlphasRenderTarget->SizeX != CombinedAlphasSize)
	{
		CombinedAlphasRenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(SceneWorld, CombinedAlphasSize, CombinedAlphasSize, RTF_RGBA16f);
	}

	if (!ScratchRenderTarget ||
		ScratchRenderTarget->SizeX != ImpostorData->Resolution)
	{
		ScratchRenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(SceneWorld, ImpostorData->Resolution, ImpostorData->Resolution, RTF_RGBA16f);
	}
}

void UImpostorRenderTargetsManager::FillMapsToSave()
{
	MapsToSave = TSet<EImpostorBakeMapType>(ImpostorData->MapsToRender);

	if (ImpostorData->bCombineLightingAndColor)
	{
		MapsToSave.Remove(EImpostorBakeMapType::CustomLighting);
	}

	if (ImpostorData->bCombineNormalAndDepth)
	{
		MapsToSave.Remove(EImpostorBakeMapType::Depth);
	}
}

void UImpostorRenderTargetsManager::SceneCaptureSetup() const
{
	if (!ensure(SceneCaptureMipChain.Num() != 0))
	{
		return;
	}

	UImpostorComponentsManager* ComponentsManager = GetManager<UImpostorComponentsManager>();
	SceneCaptureComponent2D->ShowOnlyComponents = {ComponentsManager->ReferencedMeshComponent};

	TArray<FEngineShowFlagsSetting> ShowFlagsSettings;
	ShowFlagsSettings.Add({"AmbientOcclusion", false});
	ShowFlagsSettings.Add({"Fog", false});
	ShowFlagsSettings.Add({"AtmosphericFog", false});

	SceneCaptureComponent2D->ShowFlagSettings = ShowFlagsSettings;

	SceneCaptureComponent2D->bCaptureEveryFrame = true;
	SceneCaptureComponent2D->CaptureSource = SCS_FinalColorHDR;
	SceneCaptureComponent2D->ProjectionType = ImpostorData->ProjectionType;
	SceneCaptureComponent2D->OrthoWidth = ComponentsManager->ObjectRadius * 2.f;
	SceneCaptureComponent2D->FOVAngle = ((180.f / UE_DOUBLE_PI) * FMath::Atan(GetManager<UImpostorComponentsManager>()->ObjectRadius / ImpostorData->CameraDistance)) * 2.f;
	SceneCaptureComponent2D->bCaptureOnMovement = false;
	SceneCaptureComponent2D->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	SceneCaptureComponent2D->PostProcessBlendWeight = 1.f;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UImpostorRenderTargetsManager::ClearRenderTargets()
{
	UKismetRenderingLibrary::ClearRenderTarget2D(SceneWorld, CombinedAlphasRenderTarget, FLinearColor::Black);
	UKismetRenderingLibrary::ClearRenderTarget2D(SceneWorld, ScratchRenderTarget, FLinearColor::Black);

	for (UTextureRenderTarget2D* MipRenderTarget : SceneCaptureMipChain)
	{
		UKismetRenderingLibrary::ClearRenderTarget2D(SceneWorld, MipRenderTarget, FLinearColor::Black);
	}

	for (const auto& It : TargetMaps)
	{
		UKismetRenderingLibrary::ClearRenderTarget2D(SceneWorld, It.Value, FLinearColor::Black);
	}
}

void UImpostorRenderTargetsManager::ClearRenderTarget(UTextureRenderTarget2D* RenderTarget) const
{
	UKismetRenderingLibrary::ClearRenderTarget2D(SceneWorld, RenderTarget, FLinearColor::Black);
}

void UImpostorRenderTargetsManager::ResampleRenderTarget(UTextureRenderTarget2D* Source, UTextureRenderTarget2D* Dest) const
{
	const UImpostorMaterialsManager* MaterialsManager = GetManager<UImpostorMaterialsManager>();

	MaterialsManager->ResampleMaterial->SetTextureParameterValue(GetDefault<UImpostorBakerSettings>()->ResampleRenderTarget, Source);

	UKismetRenderingLibrary::ClearRenderTarget2D(SceneWorld, Dest, FLinearColor::Black);
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(SceneWorld, Dest, MaterialsManager->ResampleMaterial);
}

void UImpostorRenderTargetsManager::BakeRenderTargets()
{
	SceneCaptureComponent2D->TextureTarget = SceneCaptureMipChain[0];
	ClearRenderTargets();

	MapsToBake = {};
	MapsToBake.Reserve(ImpostorData->MapsToRender.Num());

	const UImpostorMaterialsManager* MaterialManager = GetManager<UImpostorMaterialsManager>();
	for (const EImpostorBakeMapType MapType : ImpostorData->MapsToRender)
	{
		if (TargetMaps.Contains(MapType) &&
			MaterialManager->HasRenderTypeMaterial(MapType))
		{
			MapsToBake.Add(MapType);
		}
	}

	if (MapsToBake.Num() == 0)
	{
		return;
	}

	NumMapsToBake = MapsToBake.Num();

	ForceTick(true);

	StartSlowTask(NumMapsToBake * GetManager<UImpostorComponentsManager>()->ViewCaptureVectors.Num(), "Capturing impostor map textures...");

	PreparePostProcess(MapsToBake.Pop());
}

TMap<EImpostorBakeMapType, UTexture2D*> UImpostorRenderTargetsManager::SaveTextures()
{
	TMap<EImpostorBakeMapType, UTexture2D*> NewTextures;
	for (const EImpostorBakeMapType TargetMap : MapsToSave)
	{
		UTextureRenderTarget2D* RenderTarget = TargetMaps.FindRef(TargetMap);
		if (!ensure(RenderTarget) ||
			!ensure(RenderTarget->GetResource()))
		{
			continue;
		}

		FString AssetName = ImpostorData->NewMeshName + "_" + GetDefault<UImpostorBakerSettings>()->ImpostorPreviewMapNames[TargetMap].ToString();
		FString PackageName = ImpostorData->GetPackage(AssetName);

		UTexture2D* NewTexture = FindObject<UTexture2D>(CreatePackage(*PackageName), *AssetName);
		if (NewTexture)
		{
			RenderTarget->UpdateTexture2D(NewTexture, RenderTarget->GetTextureFormatForConversionToTexture2D(), CTF_Default | CTF_AllowMips, nullptr);
		}
		else
		{
			NewTexture = RenderTarget->ConstructTexture2D(CreatePackage(*PackageName), AssetName, RenderTarget->GetMaskedFlags() | RF_Public | RF_Standalone, CTF_Default | CTF_AllowMips, nullptr);
		}

		if (!ensure(NewTexture))
		{
			continue;
		}

		NewTexture->MarkPackageDirty();

		NewTexture->CompressionSettings = TC_Default;
		NewTexture->MipGenSettings = TMGS_FromTextureGroup;
		NewTexture->SRGB = true;
		NewTexture->PostEditChange();

		FAssetRegistryModule::AssetCreated(NewTexture);

		NewTextures.Add(TargetMap, NewTexture);
	}

	return NewTextures;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UImpostorRenderTargetsManager::PreparePostProcess(const EImpostorBakeMapType TargetMap)
{
	switch (TargetMap)
	{
	default: check(false);
	case EImpostorBakeMapType::BaseColor:
	{
		SceneCaptureComponent2D->CaptureSource = SCS_SceneColorHDR;
		SceneCaptureComponent2D->PostProcessSettings.WeightedBlendables.Array = {};
		SceneCaptureComponent2D->SceneViewExtensions = {};
		Extension = nullptr;
		break;
	}
	case EImpostorBakeMapType::CustomLighting:
	{
		SceneCaptureComponent2D->CaptureSource = SCS_SceneColorHDR;
		SceneCaptureComponent2D->PostProcessSettings.WeightedBlendables.Array = {};
		Extension = FSceneViewExtensions::NewExtension<FLightingViewExtension>(SceneCaptureComponent2D->GetScene());
		SceneCaptureComponent2D->SceneViewExtensions.Add(Extension);
		break;
	}
	case EImpostorBakeMapType::Metallic:
	case EImpostorBakeMapType::Specular:
	case EImpostorBakeMapType::Roughness:
	case EImpostorBakeMapType::Opacity:
	case EImpostorBakeMapType::WorldNormal:
	case EImpostorBakeMapType::Subsurface:
	case EImpostorBakeMapType::Depth:
	{
		SceneCaptureComponent2D->CaptureSource = SCS_FinalColorLDR;
		if (UMaterialInterface* Material = GetManager<UImpostorMaterialsManager>()->GetRenderTypeMaterial(TargetMap))
		{
			SceneCaptureComponent2D->PostProcessSettings.WeightedBlendables.Array = { FWeightedBlendable(1.f, Material) };
		}
		else
		{
			SceneCaptureComponent2D->PostProcessSettings.WeightedBlendables.Array = {};
		}
		SceneCaptureComponent2D->SceneViewExtensions = {};
		Extension = nullptr;
		break;
	}
	}

	CurrentMap = TargetMap;
	FramesBeforeCapture = 5;
}

void UImpostorRenderTargetsManager::CaptureImposterGrid()
{
	const FString Message = "Baking impostor " + GetDefault<UImpostorBakerSettings>()->ImpostorPreviewMapNames[CurrentMap].ToString() + " map... [" + LexToString(NumMapsToBake - MapsToBake.Num()) + " / " + LexToString(NumMapsToBake) + "]";

	const UImpostorComponentsManager* ComponentsManager = GetManager<UImpostorComponentsManager>();
	const TArray<FVector>& ViewCaptureVectors = ComponentsManager->ViewCaptureVectors;

	const int32 ForceEvery = FMath::RoundFromZero(NumMapsToBake * ViewCaptureVectors.Num() / 100.f);

	for (int32 Index = 0; Index < ViewCaptureVectors.Num(); Index++)
	{
		const FVector& Vector = ViewCaptureVectors[Index];

		ProgressSlowTask(Message, Index % ForceEvery == 0);

		FVector WorldLocation = ComponentsManager->ReferencedMeshComponent->Bounds.Origin;
		WorldLocation += Vector * (ImpostorData->ProjectionType == ECameraProjectionMode::Type::Orthographic ? (ComponentsManager->ReferencedMeshComponent->Bounds.SphereRadius * 1.25f) : ImpostorData->CameraDistance);
		SceneCaptureComponent2D->SetWorldLocation(WorldLocation);
		SceneCaptureComponent2D->SetWorldRotation((Vector * -1.f).ToOrientationRotator());

		FVector X, Y, Z;
		FImpostorBakerUtilities::DeriveAxes(Vector * -1.f, X, Y, Z);

		SceneWorld->SendAllEndOfFrameUpdates();
		SceneCaptureComponent2D->UpdateSceneCaptureContents(SceneWorld->Scene);

		if (ImpostorData->bUseDistanceFieldAlpha &&
			CurrentMap == EImpostorBakeMapType::BaseColor)
		{
			for (int32 MipIndex = 1; MipIndex < SceneCaptureMipChain.Num(); MipIndex++)
			{
				UKismetRenderingLibrary::ClearRenderTarget2D(SceneWorld, SceneCaptureMipChain[MipIndex], FLinearColor::Black);
				ResampleRenderTarget(SceneCaptureMipChain[MipIndex - 1], SceneCaptureMipChain[MipIndex]);
			}
		}

		DrawSingleFrame(Index);
	}
}

void UImpostorRenderTargetsManager::DrawSingleFrame(const int32 VectorIndex)
{
	const int32 NumFrames = GetManager<UImpostorComponentsManager>()->NumFrames;
	UTextureRenderTarget2D* RenderTarget = TargetMaps[CurrentMap];

	UCanvas* Canvas;
	FVector2D Size;
	FDrawToRenderTargetContext Context;

	UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(SceneWorld, RenderTarget, Canvas, Size, Context);

	UMaterialInstanceDynamic* TargetMaterial = GetManager<UImpostorMaterialsManager>()->GetSampleMaterial(CurrentMap);
	Canvas->K2_DrawMaterial(
		TargetMaterial,
		Size / NumFrames * FVector2D(VectorIndex % NumFrames, FMath::Floor(VectorIndex / NumFrames)),
		Size / NumFrames,
		FVector2D::Zero(),
		FVector2D::One(),
		0.f,
		FVector2D(0.5f, 0.5f));

	UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(SceneWorld, Context);

	if (CurrentMap == EImpostorBakeMapType::BaseColor)
	{
		// Accumulate Alphas for Mesh Cutout
		UKismetRenderingLibrary::DrawMaterialToRenderTarget(SceneWorld, CombinedAlphasRenderTarget, GetManager<UImpostorMaterialsManager>()->AddAlphasMaterial);
	}
}

void UImpostorRenderTargetsManager::FinalizeBaking()
{
	CustomCompositing();

	SceneCaptureComponent2D->TextureTarget = nullptr;

	EndSlowTask();

	ForceTick(false);

	CurrentMap = EImpostorBakeMapType::None;
}

void UImpostorRenderTargetsManager::CustomCompositing() const
{
	if (ImpostorData->bCombineNormalAndDepth &&
		ImpostorData->MapsToRender.Contains(EImpostorBakeMapType::WorldNormal) &&
		ImpostorData->MapsToRender.Contains(EImpostorBakeMapType::Depth))
	{
		UKismetRenderingLibrary::ClearRenderTarget2D(SceneWorld, ScratchRenderTarget, FLinearColor::Black);
		if (UTextureRenderTarget2D* RenderTarget = TargetMaps.FindRef(EImpostorBakeMapType::WorldNormal))
		{
			ResampleRenderTarget(RenderTarget, ScratchRenderTarget);
			UKismetRenderingLibrary::ClearRenderTarget2D(SceneWorld, RenderTarget, FLinearColor::Black);
			UKismetRenderingLibrary::DrawMaterialToRenderTarget(SceneWorld, RenderTarget, GetManager<UImpostorMaterialsManager>()->CombinedNormalsDepthMaterial);
		}
	}

	if (ImpostorData->bCombineLightingAndColor &&
		ImpostorData->MapsToRender.Contains(EImpostorBakeMapType::BaseColor) &&
		ImpostorData->MapsToRender.Contains(EImpostorBakeMapType::CustomLighting))
	{
		UKismetRenderingLibrary::ClearRenderTarget2D(SceneWorld, ScratchRenderTarget, FLinearColor::Black);
		if (UTextureRenderTarget2D* RenderTarget = TargetMaps.FindRef(EImpostorBakeMapType::BaseColor))
		{
			ResampleRenderTarget(RenderTarget, ScratchRenderTarget);
			UKismetRenderingLibrary::ClearRenderTarget2D(SceneWorld, RenderTarget, FLinearColor::Black);
			UKismetRenderingLibrary::DrawMaterialToRenderTarget(SceneWorld, RenderTarget, GetManager<UImpostorMaterialsManager>()->BaseColorCustomLightingMaterial);
		}
	}
}