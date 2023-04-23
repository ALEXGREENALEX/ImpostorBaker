// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ImpostorData.h"
#include "Engine/DeveloperSettings.h"
#include "ImpostorBakerSettings.generated.h"

UCLASS(Config = ImpostorBaker, DefaultConfig, meta = (DisplayName = "Impostor Baker"))
class IMPOSTORBAKEREDITOR_API UImpostorBakerSettings : public UDeveloperSettings
{
	GENERATED_BODY()

	//~ Begin UDeveloperSettings Interface
	virtual FName GetCategoryName() const override
	{
		return TEXT("Plugins");
	}
	//~ End UDeveloperSettings Interface

public:
	UPROPERTY(Config, EditAnywhere, Category = "Default")
	TMap<EImpostorBakeMapType, TSoftObjectPtr<UMaterialInterface>> BufferPostProcessMaterials = {
		{EImpostorBakeMapType::BaseColor, TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath("/Engine/BufferVisualization/FinalImage.FinalImage"))},
		{EImpostorBakeMapType::Metallic, TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath("/Engine/BufferVisualization/Metallic.Metallic"))},
		{EImpostorBakeMapType::Specular, TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath("/Engine/BufferVisualization/Specular.Specular"))},
		{EImpostorBakeMapType::Roughness, TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath("/Engine/BufferVisualization/Roughness.Roughness"))},
		{EImpostorBakeMapType::Opacity, TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath("/Engine/BufferVisualization/Opacity.Opacity"))},
		{EImpostorBakeMapType::Subsurface, TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath("/Engine/BufferVisualization/SubsurfaceColor.SubsurfaceColor"))},
		{EImpostorBakeMapType::Depth, TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath("/ImpostorBaker/ImpostorBaker/Materials/PostProcess/M_ObjectDepth.M_ObjectDepth"))},
	};

	UPROPERTY(Config, EditAnywhere, Category = "Materials")
	TSoftObjectPtr<UMaterialInterface> DefaultFullSphereMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath("/ImpostorBaker/ImpostorBaker/Materials/MI_Impostor_Sphere.MI_Impostor_Sphere"));

	UPROPERTY(Config, EditAnywhere, Category = "Materials")
	TSoftObjectPtr<UMaterialInterface> DefaultUpperHemisphereMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath("/ImpostorBaker/ImpostorBaker/Materials/MI_Impostor_Hemisphere.MI_Impostor_Hemisphere"));

	UPROPERTY(Config, EditAnywhere, Category = "Materials")
	TSoftObjectPtr<UMaterialInterface> DefaultBillboardMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath("/ImpostorBaker/ImpostorBaker/Materials/MI_Billboard_No_POM.MI_Billboard_No_POM"));

	UPROPERTY(Config, EditAnywhere, Category = "Materials")
	TSoftObjectPtr<UMaterialInterface> BaseColorCustomLightingMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath("/ImpostorBaker/ImpostorBaker/Materials/Generation/M_BaseColor_CustomLighting.M_BaseColor_CustomLighting"));

	UPROPERTY(Config, EditAnywhere, Category = "Materials")
	TSoftObjectPtr<UMaterialInterface> CombinedNormalsDepthMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath("/ImpostorBaker/ImpostorBaker/Materials/Generation/M_CombineNormalsDepth.M_CombineNormalsDepth"));

	UPROPERTY(Config, EditAnywhere, Category = "Materials")
	TSoftObjectPtr<UMaterialInterface> SampleFrameMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath("/ImpostorBaker/ImpostorBaker/Materials/Generation/MI_Sample_Frame.MI_Sample_Frame"));

	UPROPERTY(Config, EditAnywhere, Category = "Materials")
	TSoftObjectPtr<UMaterialInterface> SampleFrameDFAlphaMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath("/ImpostorBaker/ImpostorBaker/Materials/Generation/MI_SampleFrame_DistanceField.MI_SampleFrame_DistanceField"));

	UPROPERTY(Config, EditAnywhere, Category = "Materials")
	TSoftObjectPtr<UMaterialInterface> AddAlphasMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath("/ImpostorBaker/ImpostorBaker/Materials/Generation/M_AddAlpha.M_AddAlpha"));

	UPROPERTY(Config, EditAnywhere, Category = "Materials")
	TSoftObjectPtr<UMaterialInterface> ResampleMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath("/ImpostorBaker/ImpostorBaker/Materials/Generation/M_Resample_RT.M_Resample_RT"));

	UPROPERTY(Config, EditAnywhere, Category = "Materials")
	TSoftObjectPtr<UMaterialInterface> AddAlphaFromFinalColor = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath("/ImpostorBaker/ImpostorBaker/Materials/Generation/M_AddAlphaFromFinalColor.M_AddAlphaFromFinalColor"));

	UPROPERTY(Config, EditAnywhere, Category = "Material Parameters|Impostor Preview")
	FName ImpostorPreviewSpecular = "Specular";

	UPROPERTY(Config, EditAnywhere, Category = "Material Parameters|Impostor Preview")
	FName ImpostorPreviewRoughness = "Roughness";

	UPROPERTY(Config, EditAnywhere, Category = "Material Parameters|Impostor Preview")
	FName ImpostorPreviewOpacity = "Opacity";

	UPROPERTY(Config, EditAnywhere, Category = "Material Parameters|Impostor Preview")
	FName ImpostorPreviewScatterMaskMin = "ScatterMaskMin";

	UPROPERTY(Config, EditAnywhere, Category = "Material Parameters|Impostor Preview")
	FName ImpostorPreviewScatterMaskLen = "ScatterMaskLen";

	UPROPERTY(Config, EditAnywhere, Category = "Material Parameters|Impostor Preview")
	FName ImpostorPreviewSubsurfaceColor = "SubsurfaceColor";

	UPROPERTY(Config, EditAnywhere, Category = "Material Parameters|Impostor Preview")
	FName ImpostorPreviewMaskOffset = "MaskOffset";

	UPROPERTY(Config, EditAnywhere, Category = "Material Parameters|Impostor Preview")
	FName ImpostorPreviewEdgeGlow = "EdgeGlow";

	UPROPERTY(Config, EditAnywhere, Category = "Material Parameters|Impostor Preview")
	FName ImpostorPreviewGreenMaskMin = "Gcut";

	UPROPERTY(Config, EditAnywhere, Category = "Material Parameters|Impostor Preview")
	FName ImpostorPreviewFramesCount = "FramesXY";

	UPROPERTY(Config, EditAnywhere, Category = "Material Parameters|Impostor Preview")
	FName ImpostorPreviewMeshRadius = "MeshSize";

	UPROPERTY(Config, EditAnywhere, Category = "Material Parameters|Impostor Preview")
	FName ImpostorPreviewPivotOffset = "Pivot Offset";

	UPROPERTY(Config, EditAnywhere, Category = "Material Parameters|Impostor Preview")
	TMap<EImpostorBakeMapType, FName> ImpostorPreviewMapNames = {
		{EImpostorBakeMapType::BaseColor, "BaseColor"},
		{EImpostorBakeMapType::Metallic, "Metallic"},
		{EImpostorBakeMapType::Specular, "Specular"},
		{EImpostorBakeMapType::Roughness, "Roughness"},
		{EImpostorBakeMapType::Opacity, "Opacity"},
		{EImpostorBakeMapType::WorldNormal, "Normal"},
		{EImpostorBakeMapType::Subsurface, "Subsurface"},
		{EImpostorBakeMapType::Depth, "Depth"},
		{EImpostorBakeMapType::CustomLighting, "CustomLighting"},
	};
};