#pragma once

#include <CoreMinimal.h>
#include <Engine/DeveloperSettings.h>
#include <Materials/MaterialInterface.h>
#include "ImpostorData/ImpostorData.h"
#include "ImpostorBakerSettings.generated.h"

UCLASS(Config = ImpostorBaker, DefaultConfig, Meta = (DisplayName = "Impostor Baker"))
class IMPOSTORBAKEREDITOR_API UImpostorBakerSettings : public UObject
{
	GENERATED_BODY()

public:
	UImpostorBakerSettings();

	//~ Begin UObject Interface
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	//~ End UObject Interface

	UPROPERTY(Config, EditAnywhere, Category = "Default")
	TMap<EImpostorBakeMapType, TSoftObjectPtr<UMaterialInterface>> BufferPostProcessMaterials;

	UPROPERTY(Config, EditAnywhere, Category = "Materials")
	TSoftObjectPtr<UMaterialInterface> DefaultFullSphereMaterial;

	UPROPERTY(Config, EditAnywhere, Category = "Materials")
	TSoftObjectPtr<UMaterialInterface> DefaultUpperHemisphereMaterial;

	UPROPERTY(Config, EditAnywhere, Category = "Materials")
	TSoftObjectPtr<UMaterialInterface> DefaultBillboardMaterial;

	UPROPERTY(Config, EditAnywhere, Category = "Materials")
	TSoftObjectPtr<UMaterialInterface> DefaultBillboardTwoSidedMaterial;

	UPROPERTY(Config, EditAnywhere, Category = "Materials")
	TSoftObjectPtr<UMaterialInterface> BaseColorCustomLightingMaterial;

	UPROPERTY(Config, EditAnywhere, Category = "Materials")
	TSoftObjectPtr<UMaterialInterface> CombinedNormalsDepthMaterial;

	UPROPERTY(Config, EditAnywhere, Category = "Materials")
	TSoftObjectPtr<UMaterialInterface> SampleFrameMaterial;

	UPROPERTY(Config, EditAnywhere, Category = "Materials")
	TSoftObjectPtr<UMaterialInterface> AddAlphasMaterial;

	UPROPERTY(Config, EditAnywhere, Category = "Materials")
	TSoftObjectPtr<UMaterialInterface> ResampleMaterial;

	UPROPERTY(Config, EditAnywhere, Category = "Materials")
	TSoftObjectPtr<UMaterialInterface> AddAlphaFromFinalColor;

	// Default parameter name used to disable WPO usage for mesh.
	// To have constant results, it is recommended to add lerp(0, {WPO}, Impostor_WPO) into mesh materials.
	UPROPERTY(Config, EditAnywhere, Category = "Material Parameters")
	FName EnableWPOParameterName = "Impostor_WPO";

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
	FName ImpostorPreviewFramesCount = "FramesNum";

	UPROPERTY(Config, EditAnywhere, Category = "Material Parameters|Impostor Preview")
	FName ImpostorPreviewMeshRadius = "MeshSize";

	UPROPERTY(Config, EditAnywhere, Category = "Material Parameters|Impostor Preview")
	FName ImpostorPreviewPivotOffset = "PivotOffset";

	UPROPERTY(Config, EditAnywhere, Category = "Material Parameters|Impostor Preview")
	TMap<EImpostorBakeMapType, FName> ImpostorPreviewMapNames;
};
