﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ImpostorData.generated.h"

UENUM()
enum class EImpostorBakeMapType
{
	None UMETA(Hidden),
	BaseColor,
	Metallic,
	Specular,
	Roughness,
	Opacity,
	WorldNormal,
	Subsurface,
	Depth,
	CustomLighting
};
ENUM_RANGE_BY_FIRST_AND_LAST(EImpostorBakeMapType, EImpostorBakeMapType::BaseColor, EImpostorBakeMapType::CustomLighting)

UENUM()
enum class EImpostorLayoutType
{
	FullSphereView UMETA(Tooltip = "Full Sphere captures from all views, using an Octahedral layout."),
	UpperHemisphereOnly UMETA(Tooltip = "Upper Hemisphere captures from only above the horizon, using a Hemi-Octahedral layout."),
	TraditionalBillboards UMETA(Tooltip = "Uses a 3x3 grid of tradtional billboards. 8 views are from the sides and 1 from above.")
};

UCLASS()
class UImpostorData : public UObject
{
	GENERATED_BODY()

public:
	//~ Begin UObject Interface
	virtual bool IsEditorOnly() const override
	{
		return true;
	}
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(PrimaryAssetType, GetFName());
	}
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	//~ End UObject Interface

	void AssignMesh(const FAssetData& MeshAssetData);
	void UpdateMeshData(const FAssetData& MeshAssetData);
	UMaterialInterface* GetMaterial() const;

	void GetAssetPathName(const FString& AssetName, FString& OutPackageName, FString& OutAssetName);

public:
	FSimpleDelegate OnSettingsChange;
	TMap<FName, FSimpleMulticastDelegate> OnPropertyChange;
	TMap<FName, FSimpleMulticastDelegate> OnPropertyInteractiveChange;

	static constexpr const TCHAR* PrimaryAssetType = TEXT("ImpostorData");

public:
	// This will be the name of the newly created Material Instance
	UPROPERTY(EditAnywhere, Category = "Saving", meta = (RelativeToGameContentDir, LongPackageName))
	FDirectoryPath SaveLocation;

	// This will be the name of the newly created Material Instance
	UPROPERTY(EditAnywhere, Category = "Saving")
	FString NewMaterialName;

	// This will be the name of the newly created Texture
	UPROPERTY(EditAnywhere, Category = "Saving")
	FString NewTextureName;

	// This will be the name of the newly created Mesh
	UPROPERTY(EditAnywhere, Category = "Saving")
	FString NewMeshName;

	UPROPERTY(EditAnywhere, Category = "Default")
	UStaticMesh* ReferencedMesh;

	// List of maps (textures) to render
	UPROPERTY(EditAnywhere, Category = "Default")
	TArray<EImpostorBakeMapType> MapsToRender = {
		EImpostorBakeMapType::BaseColor,
		EImpostorBakeMapType::Depth,
		EImpostorBakeMapType::WorldNormal
	};

	// Choose the type of impostor
	UPROPERTY(EditAnywhere, Category = "Default")
	EImpostorLayoutType ImpostorType = EImpostorLayoutType::UpperHemisphereOnly;

	// Number of XY frames to capture into the atlas.
	UPROPERTY(EditAnywhere, Category = "Default", meta = (ClampMin = 1, EditCondition = "ImpostorType != EImpostorLayoutType::TraditionalBillboards", EditConditionHides))
	int32 FramesCount = 16;

	// Allows higher quality silhouette blending as well as subsurface edge effects. Makes rendering take much longer.
	UPROPERTY(EditAnywhere, Category = "Default")
	bool bUseDistanceFieldAlpha = true;

	UPROPERTY(EditAnywhere, Category = "Custom Lighting")
	float DirectionalLightBrightness = 3.14;

	UPROPERTY(EditAnywhere, Category = "Custom Lighting", meta = (ClampMin = 1))
	int32 LightingGridSize = 4;

	UPROPERTY(EditAnywhere, Category = "Custom Lighting")
	float UpwardBias = 0.75f;

	UPROPERTY(EditAnywhere, Category = "Custom Lighting")
	float CustomSkyLightIntensity = 1.f;

	// Optionally multiplies Base Color by the custom lighting texture, using settings under Custom Lighting
	UPROPERTY(EditAnywhere, Category = "Custom Lighting")
	bool bCombineLightingAndColor = false;

	// (CustomLightingValue * {Power}) * BaseColor
	// Lower power values produces custom lighting as 1, so it doesn't change the base color;
	// Power as 1 uses original custom lighting, so it affects the brightness of base color.
	UPROPERTY(EditAnywhere, Category = "Custom Lighting", meta = (EditCondition = "bCombineLightingAndColor", EditConditionHides, ClampMin = "0.001"))
	float CustomLightingPower = 0.001f;

	// 0 - custom lighting won't affect base color, 1 - fully affects base color
	UPROPERTY(EditAnywhere, Category = "Custom Lighting", meta = (EditCondition = "bCombineLightingAndColor", EditConditionHides, ClampMin = "0", ClampMax = "1"))
	float CustomLightingOpacity = 0.f;

	// Multiplies custom lighting value (allows to brighten/darken the object)
	UPROPERTY(EditAnywhere, Category = "Custom Lighting", meta = (EditCondition = "bCombineLightingAndColor", EditConditionHides))
	float CustomLightingMultiplier = 1.f;

	UPROPERTY(EditAnywhere, Category = "Custom Lighting", meta = (EditCondition = "bCombineLightingAndColor", EditConditionHides))
	float CustomLightingDesaturation = 0.5f;

	// Material used for Full Sphere mode.
	UPROPERTY(EditAnywhere, Category = "Material", meta = (EditCondition = "ImpostorType == EImpostorLayoutType::FullSphereView", EditConditionHides))
	UMaterialInterface* FullSphereMaterial;

	// Material used for Upper Hemisphere mode.
	UPROPERTY(EditAnywhere, Category = "Material", meta = (EditCondition = "ImpostorType == EImpostorLayoutType::UpperHemisphereOnly", EditConditionHides))
	UMaterialInterface* UpperHemisphereMaterial;

	// Material used for simple billboard mode.
	UPROPERTY(EditAnywhere, Category = "Material", meta = (EditCondition = "ImpostorType == EImpostorLayoutType::TraditionalBillboards", EditConditionHides))
	UMaterialInterface* BillboardMaterial;

	UPROPERTY(EditAnywhere, Category = "Material")
	float Specular = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Material")
	float Roughness = 0.5f;

	// Opacity used for Subsurface lighting.
	UPROPERTY(EditAnywhere, Category = "Material", meta = (EditCondition = "ImpostorType != EImpostorLayoutType::TraditionalBillboards", EditConditionHides))
	float Opacity = 0.5f;

	// Subsurface Color Multiplier. Alpha is a multiplier for convenience.
	UPROPERTY(EditAnywhere, Category = "Material", meta = (EditCondition = "ImpostorType != EImpostorLayoutType::TraditionalBillboards", EditConditionHides))
	FLinearColor SubsurfaceColor = FLinearColor::White;

	// The point at which to start the Distance Field Edge Gradient. 0 gives the widest and 0.5 will start the gradient at the edge.
	UPROPERTY(EditAnywhere, Category = "Material", meta = (EditCondition = "ImpostorType != EImpostorLayoutType::TraditionalBillboards", EditConditionHides, ClampMin = "0", ClampMax = "0.5"))
	float ScatterMaskMin = 0.2f;

	// The length of the distance field edge gradient.
	UPROPERTY(EditAnywhere, Category = "Material", meta = (EditCondition = "ImpostorType != EImpostorLayoutType::TraditionalBillboards", EditConditionHides))
	float ScatterMaskLength = 1.f;

	// Multiplier for extra boost to Subsurface using the distance field alpha.
	UPROPERTY(EditAnywhere, Category = "Material", meta = (EditCondition = "ImpostorType != EImpostorLayoutType::TraditionalBillboards", EditConditionHides))
	float DFEdgeGlow = 0.f;

	// Used to extract a leaf mask to prevent Subsurface from making trunks glow. Does not always work. 
	UPROPERTY(EditAnywhere, Category = "Material", meta = (EditCondition = "ImpostorType != EImpostorLayoutType::TraditionalBillboards", EditConditionHides))
	float GreenMaskMin = 0.03f;

	// Used to extract a leaf mask to prevent Subsurface from making trunks glow. Does not always work. 
	UPROPERTY(EditAnywhere, Category = "Material", meta = (EditCondition = "ImpostorType != EImpostorLayoutType::TraditionalBillboards", EditConditionHides))
	float MaskOffset = 0.f;

	UPROPERTY(EditAnywhere, Category = "Material", meta = (EditCondition = "ImpostorType == EImpostorLayoutType::TraditionalBillboards", EditConditionHides))
	float Dither = 1.f;

	UPROPERTY(EditAnywhere, Category = "Material", meta = (EditCondition = "ImpostorType == EImpostorLayoutType::TraditionalBillboards", EditConditionHides))
	float PixelDepthOffset = 0.f;

	UPROPERTY(EditAnywhere, Category = "Material", meta = (ClampMin = "0", ClampMax = "1"))
	float Dilation = 0.f;

	UPROPERTY(EditAnywhere, Category = "Material", meta = (ClampMin = "1"))
	int32 DilationMaxSteps = 64;

	UPROPERTY(EditAnywhere, Category = "Advanced")
	bool bPreviewCaptureSphere = false;

	// Composites the depth texture into the normal's alpha. This should be left on unless you are rolling a custom material since all included materials expect it
	UPROPERTY(EditAnywhere, Category = "Advanced")
	bool bCombineNormalAndDepth = true;

	// Orthographic is the most accurate, but some shader effects require a perspective view. The Camera Distance should be set as far back as it can without introducing too many Z fighting artifacts.
	UPROPERTY(EditAnywhere, Category = "Advanced")
	TEnumAsByte<ECameraProjectionMode::Type> ProjectionType = ECameraProjectionMode::Type::Orthographic;

	// Distance at which to capture for perspective (if orthographic is set to false). The Camera Distance should be set as far back as it can without introducing too many Z fighting artifacts.
	UPROPERTY(EditAnywhere, Category = "Advanced")
	float CameraDistance = 1000.f;

	// Textures resolution
	UPROPERTY(EditAnywhere, Category = "Advanced")
	int32 Resolution = 2048;

	// Resolution for scene capturing. Generally should be slightly higher than sub frame resolution. Large sizes (>512) will take a long time to render due to distance field calculation
	UPROPERTY(EditAnywhere, Category = "Advanced")
	int32 SceneCaptureResolution = 512;

	// Offsets the center vert of the top card in the traditional billboard mode.
	UPROPERTY(EditAnywhere, Category = "Advanced", meta = (EditCondition = "ImpostorType == EImpostorLayoutType::TraditionalBillboards", EditConditionHides))
	float BillboardTopOffset = 0.5f;

	// Offsets the entire Z card in the traditional billboard mode.
	UPROPERTY(EditAnywhere, Category = "Advanced", meta = (EditCondition = "ImpostorType == EImpostorLayoutType::TraditionalBillboards", EditConditionHides))
	float BillboardTopOffsetCenter = 0.75f;

	// Optionally disable mesh cutouts. Useful for debugging, such as to see the whole debug triangle view.
	UPROPERTY(EditAnywhere, Category = "Advanced")
	bool bUseMeshCutout = true;

	UPROPERTY(EditAnywhere, Category = "Advanced")
	TArray<FVector> ManualPoints;

	UPROPERTY(VisibleAnywhere, Category = "Advanced", AdvancedDisplay)
	int32 SceneCaptureMips = 9;

	UPROPERTY(VisibleAnywhere, Category = "Advanced", AdvancedDisplay)
	int32 CutoutMipTarget = 5;

	UPROPERTY(VisibleAnywhere, Category = "Advanced", AdvancedDisplay)
	int32 CheckTargetMipSize = 16;

	UPROPERTY(VisibleAnywhere, Category = "Advanced", AdvancedDisplay)
	int32 DFMipTarget = 8;
};