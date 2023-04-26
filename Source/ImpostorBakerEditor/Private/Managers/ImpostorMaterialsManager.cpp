// Fill out your copyright notice in the Description page of Project Settings.

#include "ImpostorMaterialsManager.h"

#include "ImpostorBakerSettings.h"
#include "ImpostorBakerUtilities.h"
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

	AddAlphaFromFinalColorMaterial = UMaterialInstanceDynamic::Create(Settings->AddAlphaFromFinalColor.LoadSynchronous(), nullptr);
	check(AddAlphaFromFinalColorMaterial);

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
	UpdateAddAlphaFromFinalColorMaterial();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UMaterialInstanceDynamic* UImpostorMaterialsManager::GetSampleMaterial(const EImpostorBakeMapType TargetMap) const
{
	if (ImpostorData->bUseDistanceFieldAlpha &&
		(TargetMap == EImpostorBakeMapType::BaseColor))
	{
		return SampleFrameDFAlphaMaterial;
	}

	if (TargetMap == EImpostorBakeMapType::WorldNormal)
	{
		SampleFrameMaterial->SetScalarParameterValue("Bias", 1.f);
		SampleFrameMaterial->SetScalarParameterValue("Scale", 0.5f);
	}
	else
	{
		SampleFrameMaterial->SetScalarParameterValue("Bias", 0.f);
		SampleFrameMaterial->SetScalarParameterValue("Scale", 1.f);
	}

	return SampleFrameMaterial;
}

UMaterialInterface* UImpostorMaterialsManager::GetRenderTypeMaterial(const EImpostorBakeMapType TargetMap) const
{
	switch (TargetMap)
	{
	default: check(false);
	case EImpostorBakeMapType::CustomLighting:
	case EImpostorBakeMapType::WorldNormal:
	case EImpostorBakeMapType::BaseColor: return nullptr;
	case EImpostorBakeMapType::Depth: return DepthMaterial;
	case EImpostorBakeMapType::Metallic:
	case EImpostorBakeMapType::Specular:
	case EImpostorBakeMapType::Roughness:
	case EImpostorBakeMapType::Opacity:
	case EImpostorBakeMapType::Subsurface: return GetDefault<UImpostorBakerSettings>()->BufferPostProcessMaterials.FindRef(TargetMap).LoadSynchronous();
	}
}

bool UImpostorMaterialsManager::HasRenderTypeMaterial(const EImpostorBakeMapType TargetMap) const
{
	switch (TargetMap)
	{
	default: check(false);
	case EImpostorBakeMapType::CustomLighting:
	case EImpostorBakeMapType::WorldNormal:
	case EImpostorBakeMapType::BaseColor: return true;
	case EImpostorBakeMapType::Depth: return DepthMaterial != nullptr;
	case EImpostorBakeMapType::Metallic:
	case EImpostorBakeMapType::Specular:
	case EImpostorBakeMapType::Roughness:
	case EImpostorBakeMapType::Opacity:
	case EImpostorBakeMapType::Subsurface: return GetDefault<UImpostorBakerSettings>()->BufferPostProcessMaterials.FindRef(TargetMap).LoadSynchronous() != nullptr;
	}
}

UMaterialInstanceConstant* UImpostorMaterialsManager::SaveMaterial(const TMap<EImpostorBakeMapType, UTexture2D*>& Textures) const
{
	ProgressSlowTask("Creating impostor material...", true);
	const UImpostorComponentsManager* ComponentsManager = GetManager<UImpostorComponentsManager>();
	const UImpostorBakerSettings* Settings = GetDefault<UImpostorBakerSettings>();

	const FString AssetName = ImpostorData->NewMaterialName;
	const FString PackageName = ImpostorData->GetPackage(AssetName);

	UMaterialInstanceConstant* NewMaterial = FindObject<UMaterialInstanceConstant>(CreatePackage(*PackageName), *AssetName);
	if (!NewMaterial)
	{
		UMaterialInstanceConstantFactoryNew* Factory = NewObject<UMaterialInstanceConstantFactoryNew>();
		Factory->InitialParent = ImpostorPreviewMaterial->GetMaterial();

		IAssetTools& AssetTools = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		NewMaterial = Cast<UMaterialInstanceConstant>(AssetTools.CreateAsset(ImpostorData->NewMaterialName, ImpostorData->SaveLocation.Path, UMaterialInstanceConstant::StaticClass(), Factory));
		if (!ensure(NewMaterial))
		{
			return nullptr;
		}

		TArray<UObject*> ObjectsToSync{NewMaterial};
		GEditor->SyncBrowserToObjects(ObjectsToSync);
	}
	else
	{
		if (NewMaterial->Parent != ImpostorPreviewMaterial->Parent)
		{
			NewMaterial->Parent = ImpostorPreviewMaterial->Parent;
		}
	}

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

void UImpostorMaterialsManager::UpdateDepthMaterialData(const FVector& ViewCaptureDirection) const
{
	FVector X, Y, Z;
	FImpostorBakerUtilities::DeriveAxes(ViewCaptureDirection, X, Y, Z);

	DepthMaterial->SetVectorParameterValue(FName("X"), X);
	DepthMaterial->SetVectorParameterValue(FName("Y"), Y);
	DepthMaterial->SetVectorParameterValue(FName("Z"), Z);
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
	SampleFrameMaterial->SetTextureParameterValue(FName("BaseColor"), GetManager<UImpostorRenderTargetsManager>()->SceneCaptureMipChain[0]);
	SampleFrameMaterial->SetTextureParameterValue(FName("Alpha"), GetManager<UImpostorRenderTargetsManager>()->SceneCaptureMipChain[0]);
	SampleFrameMaterial->SetScalarParameterValue(FName("TextureSize"), ImpostorData->SceneCaptureResolution);
	SampleFrameMaterial->SetScalarParameterValue(FName("Dilation"), ImpostorData->Dilation);
	SampleFrameDFAlphaMaterial->SetScalarParameterValue(FName("DilationMaxSteps"), ImpostorData->DilationMaxSteps);
}

void UImpostorMaterialsManager::UpdateSampleFrameDFAlphaMaterial() const
{
	const UImpostorRenderTargetsManager* RenderTargetsManager = GetManager<UImpostorRenderTargetsManager>();

	SampleFrameDFAlphaMaterial->SetTextureParameterValue(FName("BaseColor"), RenderTargetsManager->SceneCaptureMipChain[0]);
	SampleFrameDFAlphaMaterial->SetTextureParameterValue(FName("Alpha"), RenderTargetsManager->SceneCaptureMipChain[0]);
	SampleFrameDFAlphaMaterial->SetTextureParameterValue(FName("MipAlpha"), RenderTargetsManager->SceneCaptureMipChain[FMath::Min(RenderTargetsManager->SceneCaptureMipChain.Num() - 1, ImpostorData->DFMipTarget)]);
	SampleFrameDFAlphaMaterial->SetScalarParameterValue(FName("TextureSize"), ImpostorData->SceneCaptureResolution);
	SampleFrameDFAlphaMaterial->SetScalarParameterValue(FName("Dilation"), ImpostorData->Dilation);
	SampleFrameDFAlphaMaterial->SetScalarParameterValue(FName("DilationMaxSteps"), ImpostorData->DilationMaxSteps);
}

void UImpostorMaterialsManager::UpdateAddAlphasMaterial() const
{
	const UImpostorRenderTargetsManager* RenderTargetsManager = GetManager<UImpostorRenderTargetsManager>();

	AddAlphasMaterial->SetTextureParameterValue(FName("MipRT"), RenderTargetsManager->SceneCaptureMipChain[FMath::Min(RenderTargetsManager->SceneCaptureMipChain.Num() - 1, ImpostorData->CutoutMipTarget)]);
}

void UImpostorMaterialsManager::UpdateBaseColorCustomLightingMaterial() const
{
	const UImpostorRenderTargetsManager* RenderTargetsManager = GetManager<UImpostorRenderTargetsManager>();

	// Using Scratch RT allows the capture system to be simplistic, with any custom compositing done at the end. Combined maps can always override the original later.
	BaseColorCustomLightingMaterial->SetTextureParameterValue(FName("BaseColor"), RenderTargetsManager->ScratchRenderTarget);

	if (UTextureRenderTarget2D* RenderTarget = RenderTargetsManager->TargetMaps.FindRef(EImpostorBakeMapType::CustomLighting))
	{
		BaseColorCustomLightingMaterial->SetTextureParameterValue(FName("CustomLighting"), RenderTarget);
	}
	else
	{
		BaseColorCustomLightingMaterial->SetTextureParameterValue(FName("CustomLighting"), LoadObject<UTexture2D>(nullptr, TEXT("/Engine/ArtTools/RenderToTexture/Textures/T_EV_BlankWhite_01.T_EV_BlankWhite_01")));
	}

	BaseColorCustomLightingMaterial->SetScalarParameterValue(FName("CustomLightingPower"), ImpostorData->CustomLightingPower);
	BaseColorCustomLightingMaterial->SetScalarParameterValue(FName("CustomLightingBaseColorOpacity"), ImpostorData->CustomLightingOpacity);
	BaseColorCustomLightingMaterial->SetScalarParameterValue(FName("Desaturation"), ImpostorData->CustomLightingDesaturation);
	BaseColorCustomLightingMaterial->SetScalarParameterValue(FName("CustomLightingMultiplier"), ImpostorData->CustomLightingMultiplier);
}

void UImpostorMaterialsManager::UpdateCombinedNormalsDepthMaterial() const
{
	const UImpostorBakerSettings* Settings = GetDefault<UImpostorBakerSettings>();
	const UImpostorRenderTargetsManager* RenderTargetsManager = GetManager<UImpostorRenderTargetsManager>();

	CombinedNormalsDepthMaterial->SetTextureParameterValue(FName("Normal"), RenderTargetsManager->ScratchRenderTarget);

	if (UTextureRenderTarget2D* RenderTarget = RenderTargetsManager->TargetMaps.FindRef(EImpostorBakeMapType::Depth))
	{
		CombinedNormalsDepthMaterial->SetTextureParameterValue(FName("Depth"), RenderTarget);
	}
	else
	{
		CombinedNormalsDepthMaterial->SetTextureParameterValue(FName("Depth"), LoadObject<UTexture2D>(nullptr, TEXT("/Engine/ArtTools/RenderToTexture/Textures/127grey.127grey")));
	}
}

void UImpostorMaterialsManager::UpdateDepthMaterial() const
{
	if (!DepthMaterial)
	{
		return;
	}

	DepthMaterial->SetVectorParameterValue(FName("Origin"), GetManager<UImpostorComponentsManager>()->ReferencedMeshComponent->Bounds.Origin);
	DepthMaterial->SetScalarParameterValue(FName("Radius"), GetManager<UImpostorComponentsManager>()->ObjectRadius);
}

void UImpostorMaterialsManager::UpdateAddAlphaFromFinalColorMaterial() const
{
	const UImpostorRenderTargetsManager* RenderTargetsManager = GetManager<UImpostorRenderTargetsManager>();

	AddAlphaFromFinalColorMaterial->SetTextureParameterValue(FName("BaseColor"), RenderTargetsManager->BaseColorScratchRenderTarget);
	AddAlphaFromFinalColorMaterial->SetTextureParameterValue(FName("FinalColor"), RenderTargetsManager->ScratchRenderTarget);
}