// Fill out your copyright notice in the Description page of Project Settings.

#include "ImpostorMaterialsManager.h"

#include "ImpostorBakerSettings.h"
#include "ImpostorComponentsManager.h"
#include "ImpostorRenderTargetsManager.h"

#include "AssetToolsModule.h"
#include "MaterialInstanceDynamic.h"
#include "MaterialInstanceConstant.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"

void UImpostorMaterialsManager::Initialize()
{
	const UImpostorBakerSettings* Settings = GetDefault<UImpostorBakerSettings>();

	BaseColorCustomLightingMaterial = UMaterialInstanceDynamic::Create(Settings->BaseColorCustomLightingMaterial.LoadSynchronous(), nullptr);
	check(BaseColorCustomLightingMaterial);

	CombinedNormalsDepthMaterial = UMaterialInstanceDynamic::Create(Settings->CombinedNormalsDepthMaterial.LoadSynchronous(), nullptr);
	check(CombinedNormalsDepthMaterial);

	SampleFrameMaterial = UMaterialInstanceDynamic::Create(Settings->SampleFrameMaterial.LoadSynchronous(), nullptr);
	check(SampleFrameMaterial);

	SampleFrameDFAlphaMaterial = UMaterialInstanceDynamic::Create(Settings->SampleFrameDFAlphaMaterial.LoadSynchronous(), nullptr);
	check(SampleFrameDFAlphaMaterial);

	AddAlphasMaterial = UMaterialInstanceDynamic::Create(Settings->AddAlphasMaterial.LoadSynchronous(), nullptr);
	check(AddAlphasMaterial);

	ResampleMaterial = UMaterialInstanceDynamic::Create(Settings->ResampleMaterial.LoadSynchronous(), nullptr);
	check(ResampleMaterial);

	if (const TSoftObjectPtr<UMaterialInterface>* WeakDepthMaterial = GetDefault<UImpostorBakerSettings>()->BufferPostProcessMaterials.Find(EImpostorBakeMapType::Depth))
	{
		if (!WeakDepthMaterial->IsNull())
		{
			DepthMaterial = UMaterialInstanceDynamic::Create(WeakDepthMaterial->LoadSynchronous(), nullptr);
			check(DepthMaterial);
		}
	}

	IMPOSTOR_GET_PROPERTY_CHANGE_DELEGATE(Specular).AddWeakLambda(this, [this]
	{
		ImpostorPreviewMaterial->SetScalarParameterValue(GetDefault<UImpostorBakerSettings>()->ImpostorPreviewSpecular, ImpostorData->Specular);
	});
	IMPOSTOR_GET_PROPERTY_CHANGE_DELEGATE(Roughness).AddWeakLambda(this, [this]
	{
		ImpostorPreviewMaterial->SetScalarParameterValue(GetDefault<UImpostorBakerSettings>()->ImpostorPreviewRoughness, ImpostorData->Roughness);
	});
	IMPOSTOR_GET_PROPERTY_CHANGE_DELEGATE(Opacity).AddWeakLambda(this, [this]
	{
		ImpostorPreviewMaterial->SetScalarParameterValue(GetDefault<UImpostorBakerSettings>()->ImpostorPreviewOpacity, ImpostorData->Opacity);
	});
	IMPOSTOR_GET_PROPERTY_CHANGE_DELEGATE(SubsurfaceColor).AddWeakLambda(this, [this]
	{
		ImpostorPreviewMaterial->SetVectorParameterValue(GetDefault<UImpostorBakerSettings>()->ImpostorPreviewSubsurfaceColor, ImpostorData->SubsurfaceColor);
	});
	IMPOSTOR_GET_PROPERTY_CHANGE_DELEGATE(ScatterMaskMin).AddWeakLambda(this, [this]
	{
		ImpostorPreviewMaterial->SetScalarParameterValue(GetDefault<UImpostorBakerSettings>()->ImpostorPreviewScatterMaskMin, ImpostorData->ScatterMaskMin);
	});
	IMPOSTOR_GET_PROPERTY_CHANGE_DELEGATE(ScatterMaskLength).AddWeakLambda(this, [this]
	{
		ImpostorPreviewMaterial->SetScalarParameterValue(GetDefault<UImpostorBakerSettings>()->ImpostorPreviewScatterMaskLen, ImpostorData->ScatterMaskLength);
	});
	IMPOSTOR_GET_PROPERTY_CHANGE_DELEGATE(DFEdgeGlow).AddWeakLambda(this, [this]
	{
		ImpostorPreviewMaterial->SetScalarParameterValue(GetDefault<UImpostorBakerSettings>()->ImpostorPreviewEdgeGlow, ImpostorData->DFEdgeGlow);
	});
	IMPOSTOR_GET_PROPERTY_CHANGE_DELEGATE(GreenMaskMin).AddWeakLambda(this, [this]
	{
		ImpostorPreviewMaterial->SetScalarParameterValue(GetDefault<UImpostorBakerSettings>()->ImpostorPreviewGreenMaskMin, ImpostorData->GreenMaskMin);
	});
	IMPOSTOR_GET_PROPERTY_CHANGE_DELEGATE(MaskOffset).AddWeakLambda(this, [this]
	{
		ImpostorPreviewMaterial->SetScalarParameterValue(GetDefault<UImpostorBakerSettings>()->ImpostorPreviewMaskOffset, ImpostorData->MaskOffset);
	});
	IMPOSTOR_GET_PROPERTY_CHANGE_DELEGATE(Dither).AddWeakLambda(this, [this]
	{
		ImpostorPreviewMaterial->SetScalarParameterValue("Dither", ImpostorData->Dither);
	});
	IMPOSTOR_GET_PROPERTY_CHANGE_DELEGATE(PixelDepthOffset).AddWeakLambda(this, [this]
	{
		ImpostorPreviewMaterial->SetScalarParameterValue("PDO", ImpostorData->PixelDepthOffset);
	});
}

void UImpostorMaterialsManager::Update()
{
	CreatePreviewMaterial();

	UpdateSampleFrameMaterial();
	UpdateSampleFrameDFAlphaMaterial();
	UpdateAddAlphasMaterial();
	UpdateBaseColorCustomLightingMaterial();
	UpdateCombinedNormalsDepthMaterial();
	UpdateDepthMaterial();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UMaterialInstanceDynamic* UImpostorMaterialsManager::GetSampleMaterial(const EImpostorBakeMapType TargetMap) const
{
	if (ImpostorData->bUseDistanceFieldAlpha &&
		(TargetMap == EImpostorBakeMapType::BaseColor || TargetMap == EImpostorBakeMapType::Depth))
	{
		return SampleFrameDFAlphaMaterial;
	}

	return SampleFrameMaterial;
}

UMaterialInterface* UImpostorMaterialsManager::GetRenderTypeMaterial(const EImpostorBakeMapType TargetMap) const
{
	switch (TargetMap)
	{
	default: check(false);
	case EImpostorBakeMapType::CustomLighting:
	case EImpostorBakeMapType::BaseColor: return nullptr;
	case EImpostorBakeMapType::Depth: return DepthMaterial;
	case EImpostorBakeMapType::Metallic:
	case EImpostorBakeMapType::Specular:
	case EImpostorBakeMapType::Roughness:
	case EImpostorBakeMapType::Opacity:
	case EImpostorBakeMapType::WorldNormal:
	case EImpostorBakeMapType::Subsurface: return GetDefault<UImpostorBakerSettings>()->BufferPostProcessMaterials.FindRef(TargetMap).LoadSynchronous();
	}
}

bool UImpostorMaterialsManager::HasRenderTypeMaterial(const EImpostorBakeMapType TargetMap) const
{
	switch (TargetMap)
	{
	default: check(false);
	case EImpostorBakeMapType::CustomLighting:
	case EImpostorBakeMapType::BaseColor: return true;
	case EImpostorBakeMapType::Depth: return DepthMaterial != nullptr;
	case EImpostorBakeMapType::Metallic:
	case EImpostorBakeMapType::Specular:
	case EImpostorBakeMapType::Roughness:
	case EImpostorBakeMapType::Opacity:
	case EImpostorBakeMapType::WorldNormal:
	case EImpostorBakeMapType::Subsurface: return GetDefault<UImpostorBakerSettings>()->BufferPostProcessMaterials.FindRef(TargetMap).LoadSynchronous() != nullptr;
	}
}

UMaterialInstanceConstant* UImpostorMaterialsManager::SaveMaterial(const TMap<EImpostorBakeMapType, UTexture2D*>& Textures) const
{
	const UImpostorComponentsManager* ComponentsManager = GetManager<UImpostorComponentsManager>();
	const UImpostorBakerSettings* Settings = GetDefault<UImpostorBakerSettings>();

	UMaterialInstanceConstantFactoryNew* Factory = NewObject<UMaterialInstanceConstantFactoryNew>();
	Factory->InitialParent = ImpostorPreviewMaterial->GetMaterial();

	IAssetTools& AssetTools = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	UMaterialInstanceConstant* NewMaterial = Cast<UMaterialInstanceConstant>(AssetTools.CreateAsset(ImpostorData->NewMaterialName, ImpostorData->SaveLocation.Path, UMaterialInstanceConstant::StaticClass(), Factory));
	if (!ensure(NewMaterial))
	{
		return nullptr;
	}

	TArray<UObject*> ObjectsToSync{NewMaterial};
	GEditor->SyncBrowserToObjects(ObjectsToSync);

	NewMaterial->SetScalarParameterValueEditorOnly(Settings->ImpostorPreviewSpecular, ImpostorData->Specular);
	NewMaterial->SetScalarParameterValueEditorOnly(Settings->ImpostorPreviewRoughness, ImpostorData->Roughness);
	NewMaterial->SetScalarParameterValueEditorOnly(Settings->ImpostorPreviewOpacity, ImpostorData->Opacity);
	NewMaterial->SetVectorParameterValueEditorOnly(Settings->ImpostorPreviewSubsurfaceColor, ImpostorData->SubsurfaceColor);
	NewMaterial->SetScalarParameterValueEditorOnly(Settings->ImpostorPreviewScatterMaskMin, ImpostorData->ScatterMaskMin);
	NewMaterial->SetScalarParameterValueEditorOnly(Settings->ImpostorPreviewScatterMaskLen, ImpostorData->ScatterMaskLength);
	NewMaterial->SetScalarParameterValueEditorOnly(Settings->ImpostorPreviewEdgeGlow, ImpostorData->DFEdgeGlow);
	NewMaterial->SetScalarParameterValueEditorOnly(Settings->ImpostorPreviewGreenMaskMin, ImpostorData->GreenMaskMin);
	NewMaterial->SetScalarParameterValueEditorOnly(Settings->ImpostorPreviewMaskOffset, ImpostorData->MaskOffset);
	NewMaterial->SetScalarParameterValueEditorOnly(FName("Dither"), ImpostorData->Dither);
	NewMaterial->SetScalarParameterValueEditorOnly(FName("PDO"), ImpostorData->PixelDepthOffset);

	NewMaterial->SetScalarParameterValueEditorOnly(Settings->ImpostorPreviewFramesCount, ComponentsManager->NumFrames);
	NewMaterial->SetScalarParameterValueEditorOnly(Settings->ImpostorPreviewMeshRadius, ComponentsManager->ObjectRadius * 2.f);
	NewMaterial->SetVectorParameterValueEditorOnly(Settings->ImpostorPreviewPivotOffset, FLinearColor(ComponentsManager->OffsetVector));

	// Bind Textures
	for (const auto& It : Textures)
	{
		if (!ensure(Settings->ImpostorPreviewMapNames.Contains(It.Key)))
		{
			continue;
		}

		NewMaterial->SetTextureParameterValueEditorOnly(Settings->ImpostorPreviewMapNames[It.Key], It.Value);
	}

	NewMaterial->PostEditChange();

	return NewMaterial;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UImpostorMaterialsManager::CreatePreviewMaterial()
{
	if (!ImpostorPreviewMaterial ||
		ImpostorPreviewMaterial->GetMaterial() != ImpostorData->GetMaterial())
	{
		ImpostorPreviewMaterial = UMaterialInstanceDynamic::Create(ImpostorData->GetMaterial(), nullptr);
		check(ImpostorPreviewMaterial);
	}

	UpdateImpostorMaterial();
}

void UImpostorMaterialsManager::UpdateImpostorMaterial() const
{
	const UImpostorComponentsManager* ComponentsManager = GetManager<UImpostorComponentsManager>();
	const UImpostorBakerSettings* Settings = GetDefault<UImpostorBakerSettings>();

	ImpostorPreviewMaterial->SetScalarParameterValue(Settings->ImpostorPreviewSpecular, ImpostorData->Specular);
	ImpostorPreviewMaterial->SetScalarParameterValue(Settings->ImpostorPreviewRoughness, ImpostorData->Roughness);
	ImpostorPreviewMaterial->SetScalarParameterValue(Settings->ImpostorPreviewOpacity, ImpostorData->Opacity);
	ImpostorPreviewMaterial->SetVectorParameterValue(Settings->ImpostorPreviewSubsurfaceColor, ImpostorData->SubsurfaceColor);
	ImpostorPreviewMaterial->SetScalarParameterValue(Settings->ImpostorPreviewScatterMaskMin, ImpostorData->ScatterMaskMin);
	ImpostorPreviewMaterial->SetScalarParameterValue(Settings->ImpostorPreviewScatterMaskLen, ImpostorData->ScatterMaskLength);
	ImpostorPreviewMaterial->SetScalarParameterValue(Settings->ImpostorPreviewEdgeGlow, ImpostorData->DFEdgeGlow);
	ImpostorPreviewMaterial->SetScalarParameterValue(Settings->ImpostorPreviewGreenMaskMin, ImpostorData->GreenMaskMin);
	ImpostorPreviewMaterial->SetScalarParameterValue(Settings->ImpostorPreviewMaskOffset, ImpostorData->MaskOffset);
	ImpostorPreviewMaterial->SetScalarParameterValue("Dither", ImpostorData->Dither);
	ImpostorPreviewMaterial->SetScalarParameterValue("PDO", ImpostorData->PixelDepthOffset);

	ImpostorPreviewMaterial->SetScalarParameterValue(Settings->ImpostorPreviewFramesCount, ComponentsManager->NumFrames);
	ImpostorPreviewMaterial->SetScalarParameterValue(Settings->ImpostorPreviewMeshRadius, ComponentsManager->ObjectRadius * 2.f);
	ImpostorPreviewMaterial->SetVectorParameterValue(Settings->ImpostorPreviewPivotOffset, ComponentsManager->OffsetVector);

	// Bind Render Targets
	for (const auto& It : GetManager<UImpostorRenderTargetsManager>()->TargetMaps)
	{
		if (!ensure(Settings->ImpostorPreviewMapNames.Contains(It.Key)))
		{
			continue;
		}

		ImpostorPreviewMaterial->SetTextureParameterValue(Settings->ImpostorPreviewMapNames[It.Key], It.Value);
	}
}

void UImpostorMaterialsManager::UpdateSampleFrameMaterial() const
{
	const UImpostorBakerSettings* Settings = GetDefault<UImpostorBakerSettings>();

	SampleFrameMaterial->SetTextureParameterValue(Settings->SampleFrameRenderTarget, GetManager<UImpostorRenderTargetsManager>()->SceneCaptureMipChain[0]);
	SampleFrameMaterial->SetScalarParameterValue(Settings->SampleFrameTextureSize, ImpostorData->SceneCaptureResolution);
	SampleFrameMaterial->SetScalarParameterValue(FName("Dilation"), ImpostorData->Dilation);
	SampleFrameDFAlphaMaterial->SetScalarParameterValue(FName("DilationMaxSteps"), ImpostorData->DilationMaxSteps);
}

void UImpostorMaterialsManager::UpdateSampleFrameDFAlphaMaterial() const
{
	const UImpostorBakerSettings* Settings = GetDefault<UImpostorBakerSettings>();
	const UImpostorRenderTargetsManager* RenderTargetsManager = GetManager<UImpostorRenderTargetsManager>();

	SampleFrameDFAlphaMaterial->SetTextureParameterValue(Settings->SampleFrameDFAlphaRenderTarget, RenderTargetsManager->SceneCaptureMipChain[0]);
	SampleFrameDFAlphaMaterial->SetTextureParameterValue(Settings->SampleFrameDFAlphaMipRenderTarget, RenderTargetsManager->SceneCaptureMipChain[FMath::Min(RenderTargetsManager->SceneCaptureMipChain.Num() - 1, ImpostorData->DFMipTarget)]);
	SampleFrameDFAlphaMaterial->SetScalarParameterValue(Settings->SampleFrameDFAlphaTextureSize, ImpostorData->SceneCaptureResolution);
	SampleFrameDFAlphaMaterial->SetScalarParameterValue(FName("Dilation"), ImpostorData->Dilation);
	SampleFrameDFAlphaMaterial->SetScalarParameterValue(FName("DilationMaxSteps"), ImpostorData->DilationMaxSteps);
}

void UImpostorMaterialsManager::UpdateAddAlphasMaterial() const
{
	const UImpostorRenderTargetsManager* RenderTargetsManager = GetManager<UImpostorRenderTargetsManager>();

	AddAlphasMaterial->SetTextureParameterValue(GetDefault<UImpostorBakerSettings>()->AddAlphasRenderTarget, RenderTargetsManager->SceneCaptureMipChain[FMath::Min(RenderTargetsManager->SceneCaptureMipChain.Num() - 1, ImpostorData->CutoutMipTarget)]);
}

void UImpostorMaterialsManager::UpdateBaseColorCustomLightingMaterial() const
{
	const UImpostorBakerSettings* Settings = GetDefault<UImpostorBakerSettings>();
	const UImpostorRenderTargetsManager* RenderTargetsManager = GetManager<UImpostorRenderTargetsManager>();

	// Using Scratch RT allows the capture system to be simplistic, with any custom compositing done at the end. Combined maps can always override the original later.
	BaseColorCustomLightingMaterial->SetTextureParameterValue(Settings->BaseColorCustomLightingBaseColorTexture, RenderTargetsManager->ScratchRenderTarget);

	if (UTextureRenderTarget2D* RenderTarget = RenderTargetsManager->TargetMaps.FindRef(EImpostorBakeMapType::CustomLighting))
	{
		BaseColorCustomLightingMaterial->SetTextureParameterValue(Settings->BaseColorCustomLightingCustomLightingTexture, RenderTarget);
	}
	else
	{
		BaseColorCustomLightingMaterial->SetTextureParameterValue(Settings->BaseColorCustomLightingCustomLightingTexture, LoadObject<UTexture2D>(nullptr, TEXT("/Engine/ArtTools/RenderToTexture/Textures/T_EV_BlankWhite_01.T_EV_BlankWhite_01")));
	}

	BaseColorCustomLightingMaterial->SetScalarParameterValue(Settings->BaseColorCustomLightingPower, ImpostorData->CustomLightingPower);
	BaseColorCustomLightingMaterial->SetScalarParameterValue(Settings->BaseColorCustomLightingOpacity, ImpostorData->CustomLightingOpacity);
	BaseColorCustomLightingMaterial->SetScalarParameterValue(Settings->BaseColorCustomLightingDesaturation, ImpostorData->CustomLightingDesaturation);
	BaseColorCustomLightingMaterial->SetScalarParameterValue(Settings->BaseColorCustomLightingMultiplier, ImpostorData->CustomLightingMultiplier);
}

void UImpostorMaterialsManager::UpdateCombinedNormalsDepthMaterial() const
{
	const UImpostorBakerSettings* Settings = GetDefault<UImpostorBakerSettings>();
	const UImpostorRenderTargetsManager* RenderTargetsManager = GetManager<UImpostorRenderTargetsManager>();

	CombinedNormalsDepthMaterial->SetTextureParameterValue(Settings->CombinedNormalsDepthNormalTexture, RenderTargetsManager->ScratchRenderTarget);

	if (UTextureRenderTarget2D* RenderTarget = RenderTargetsManager->TargetMaps.FindRef(EImpostorBakeMapType::Depth))
	{
		CombinedNormalsDepthMaterial->SetTextureParameterValue(Settings->CombinedNormalsDepthDepthTexture, RenderTarget);
	}
	else
	{
		CombinedNormalsDepthMaterial->SetTextureParameterValue(Settings->CombinedNormalsDepthDepthTexture, LoadObject<UTexture2D>(nullptr, TEXT("/Engine/ArtTools/RenderToTexture/Textures/127grey.127grey")));
	}
}

void UImpostorMaterialsManager::UpdateDepthMaterial() const
{
	if (!DepthMaterial)
	{
		return;
	}

	DepthMaterial->SetScalarParameterValue(FName("Radius"), GetManager<UImpostorComponentsManager>()->ReferencedMeshComponent->Bounds.SphereRadius * 2.f);
}
