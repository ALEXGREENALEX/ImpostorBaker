#pragma once

#include <CoreMinimal.h>
#include <Camera/CameraTypes.h>
#include <Misc/EnumRange.h>
#include <UObject/Object.h>
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
	Normal,
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

UENUM()
enum class EImpostorPerspectiveCameraType
{
	Distance UMETA(Tooltip = "Camera FOV will be calculated by the specified distance"),
	FOV UMETA(Tooltip = "Camera distance will be calculated by the specified FOV"),
	Both UMETA(Tooltip = "Camera distance and the FOV will be custom")
};

UENUM()
enum class EImpostorMeshOffsetType
{
	None UMETA(Tooltip = "Will not offset mesh and use its bounding box center"),
	MeshOrigin UMETA(Tooltip = "Will offset mesh by its origin"),
	CustomOffset UMETA(Tooltip = "Will offset mesh by custom offset")
};

UCLASS()
class UImpostorData : public UObject
{
	GENERATED_BODY()

public:
	//~ Begin UObject Interface
	virtual bool IsEditorOnly() const override { return true; }
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	//~ End UObject Interface

	void AssignMesh(const FAssetData& MeshAssetData);
	void UpdateMeshData(const FAssetData& MeshAssetData);
	UMaterialInterface* GetMaterial() const;

	FString GetPackageName(const FString& AssetName) const;

	FVector2D GetMeshOffset() const;

private:
	void UpdateFOVDistance();

public:
	FSimpleDelegate OnSettingsChange;
	TMap<FName, FSimpleMulticastDelegate> OnPropertyChange;
	TMap<FName, FSimpleMulticastDelegate> OnPropertyInteractiveChange;

	static constexpr const TCHAR* PrimaryAssetType = TEXT("ImpostorData");

public:
	// This will be the name of the newly created Material Instance
	UPROPERTY(EditAnywhere, Category = "Saving", Meta = (RelativeToGameContentDir, LongPackageName))
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

	// The LOD to which impostor will be saved.
	// If selected LOD exists, it will be overwritten.
	UPROPERTY(EditAnywhere, Category = "Saving")
	int32 TargetLOD = 0;

	// When mesh will be saved, will it cast shadow or not.
	UPROPERTY(EditAnywhere, Category = "Saving")
	bool bMeshCastShadow = false;

	UPROPERTY(EditAnywhere, Category = "Default")
	TObjectPtr<UStaticMesh> ReferencedMesh;

	// List of maps (textures) to render
	UPROPERTY(EditAnywhere, Category = "Default")
	TArray<EImpostorBakeMapType> MapsToRender = {
		EImpostorBakeMapType::BaseColor,
		EImpostorBakeMapType::Depth,
		EImpostorBakeMapType::Normal
	};

	// Will use final color instead of base color
	UPROPERTY(EditAnywhere, Category = "Default", Meta = (EditCondition = "ProjectionType == ECameraProjectionMode::Perspective", EditConditionHides))
	bool bUseFinalColorInsteadBaseColor = false;

	// Choose the type of impostor
	UPROPERTY(EditAnywhere, Category = "Default")
	EImpostorLayoutType ImpostorType = EImpostorLayoutType::UpperHemisphereOnly;

	// This works only from 5.3 for all meshes and from 5.1 for nanite meshes.
	// For incompatible meshes/engine versions, use `EnableWPOParameterName` and refer to its description.
	UPROPERTY(EditAnywhere, Category = "Default")
	bool bDisableWPO = true;

	// To have constant results, it is recommended to add lerp(0, {WPO}, Impostor_WPO) into mesh materials.
	// Value will be set to 0.
	UPROPERTY(EditAnywhere, Category = "Default")
	FName EnableWPOParameterName;

	// Number of XY frames to capture into the atlas.
	UPROPERTY(EditAnywhere, Category = "Impostors", Meta = (ClampMin = 1, EditCondition = "ImpostorType != EImpostorLayoutType::TraditionalBillboards", EditConditionHides))
	int32 FramesCount = 16;

	// Textures resolution
	UPROPERTY(EditAnywhere, Category = "Impostors", Meta = (EditCondition = "ImpostorType != EImpostorLayoutType::TraditionalBillboards", EditConditionHides))
	int32 Resolution = 2048;

	UPROPERTY(EditAnywhere, Category = "Traditional Billboards", Meta = (EditCondition = "ImpostorType == EImpostorLayoutType::TraditionalBillboards", EditConditionHides))
	int32 FrameSize = 256;

	// Will generate two-sided geometry for traditional billboards (2x horizontal frames)
	UPROPERTY(EditAnywhere, Category = "Traditional Billboards", Meta = (EditCondition = "ImpostorType == EImpostorLayoutType::TraditionalBillboards", EditConditionHides))
	bool bGenerateTwoSidedGeometry = true;

	// Number of horizontal frames for traditional billboards
	UPROPERTY(EditAnywhere, Category = "Traditional Billboards", Meta = (ClampMin = 1, ClampMax = 4, EditCondition = "ImpostorType == EImpostorLayoutType::TraditionalBillboards", EditConditionHides))
	int32 HorizontalFramesCount = 3;

	UPROPERTY(EditAnywhere, Category = "Traditional Billboards", Meta = (EditCondition = "ImpostorType == EImpostorLayoutType::TraditionalBillboards", EditConditionHides))
	bool bCaptureTopFrame = true;

	// Offsets the center vert of the top card in the traditional billboard mode.
	UPROPERTY(EditAnywhere, Category = "Traditional Billboards", Meta = (EditCondition = "ImpostorType == EImpostorLayoutType::TraditionalBillboards && bCaptureTopFrame", EditConditionHides))
	float BillboardTopOffset = 0.5f;

	// Offsets the entire Z card in the traditional billboard mode.
	UPROPERTY(EditAnywhere, Category = "Traditional Billboards", Meta = (EditCondition = "ImpostorType == EImpostorLayoutType::TraditionalBillboards && bCaptureTopFrame", EditConditionHides))
	float BillboardTopOffsetCenter = 0.75f;

	// Material used for Full Sphere mode.
	UPROPERTY(EditAnywhere, Category = "Material", Meta = (EditCondition = "ImpostorType == EImpostorLayoutType::FullSphereView", EditConditionHides))
	TObjectPtr<UMaterialInterface> FullSphereMaterial;

	// Material used for Upper Hemisphere mode.
	UPROPERTY(EditAnywhere, Category = "Material", Meta = (EditCondition = "ImpostorType == EImpostorLayoutType::UpperHemisphereOnly", EditConditionHides))
	TObjectPtr<UMaterialInterface> UpperHemisphereMaterial;

	// Material used for simple billboard mode.
	UPROPERTY(EditAnywhere, Category = "Material", Meta = (EditCondition = "ImpostorType == EImpostorLayoutType::TraditionalBillboards", EditConditionHides))
	TObjectPtr<UMaterialInterface> BillboardMaterial;

	// Composites the depth texture into the normal's alpha. This should be left on unless you are rolling a custom material since all included materials expect it
	UPROPERTY(EditAnywhere, Category = "Material")
	bool bCombineNormalAndDepth = true;

	UPROPERTY(EditAnywhere, Category = "Material", Meta = (UIMin = "0", UIMax = "1"))
	float Specular = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Material", Meta = (UIMin = "0", UIMax = "1"))
	float Roughness = 0.5f;

	// Opacity used for Subsurface lighting.
	UPROPERTY(EditAnywhere, Category = "Material", Meta = (UIMin = "0", UIMax = "1", EditCondition = "ImpostorType != EImpostorLayoutType::TraditionalBillboards", EditConditionHides))
	float Opacity = 0.5f;

	// Subsurface Color Multiplier. Alpha is a multiplier for convenience.
	UPROPERTY(EditAnywhere, Category = "Material", Meta = (EditCondition = "ImpostorType != EImpostorLayoutType::TraditionalBillboards", EditConditionHides))
	FLinearColor SubsurfaceColor = FLinearColor::White;

	// The point at which to start the Distance Field Edge Gradient. 0 gives the widest and 0.5 will start the gradient at the edge.
	UPROPERTY(EditAnywhere, Category = "Material", Meta = (UIMin = "0", UIMax = "1", EditCondition = "ImpostorType != EImpostorLayoutType::TraditionalBillboards", EditConditionHides, ClampMin = "0", ClampMax = "0.5"))
	float ScatterMaskMin = 0.2f;

	// The length of the distance field edge gradient.
	UPROPERTY(EditAnywhere, Category = "Material", Meta = (EditCondition = "ImpostorType != EImpostorLayoutType::TraditionalBillboards", EditConditionHides))
	float ScatterMaskLength = 1.0f;

	// Multiplier for extra boost to Subsurface using the distance field alpha.
	UPROPERTY(EditAnywhere, Category = "Material", Meta = (UIMin = "0", UIMax = "1", EditCondition = "ImpostorType != EImpostorLayoutType::TraditionalBillboards", EditConditionHides))
	float DFEdgeGlow = 0.0f;

	// Used to extract a leaf mask to prevent Subsurface from making trunks glow. Does not always work. 
	UPROPERTY(EditAnywhere, Category = "Material", Meta = (UIMin = "0", UIMax = "1", EditCondition = "ImpostorType != EImpostorLayoutType::TraditionalBillboards", EditConditionHides))
	float GreenMaskMin = 0.03f;

	// Used to extract a leaf mask to prevent Subsurface from making trunks glow. Does not always work. 
	UPROPERTY(EditAnywhere, Category = "Material", Meta = (UIMin = "-1", UIMax = "1", EditCondition = "ImpostorType != EImpostorLayoutType::TraditionalBillboards", EditConditionHides))
	float MaskOffset = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Material", Meta = (UIMin = "0", UIMax = "1", EditCondition = "ImpostorType == EImpostorLayoutType::TraditionalBillboards", EditConditionHides))
	float Dither = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Material")
	bool bEnablePixelDepthOffset = false;

	// Values bigger 0 allow to hide wrong shadows on mesh.
	UPROPERTY(EditAnywhere, Category = "Material")
	float PixelDepthOffset = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Material")
	bool bEnableDilation = false;

	UPROPERTY(EditAnywhere, Category = "Material", Meta = (EditCondition = "bEnableDilation"))
	bool bOverrideDilationSteps = false;

	UPROPERTY(EditAnywhere, Category = "Material", Meta = (EditCondition = "bEnableDilation && bOverrideDilationSteps", ClampMin = "1"))
	int32 DilationMaxSteps = 64;

	UPROPERTY(EditAnywhere, Category = "Custom Lighting")
	float DirectionalLightBrightness = 3.14;

	UPROPERTY(EditAnywhere, Category = "Custom Lighting", Meta = (ClampMin = 1))
	int32 LightingGridSize = 4;

	UPROPERTY(EditAnywhere, Category = "Custom Lighting")
	float UpwardBias = 0.75f;

	UPROPERTY(EditAnywhere, Category = "Custom Lighting")
	float CustomSkyLightIntensity = 1.0f;

	// Optionally multiplies Base Color by the custom lighting texture, using settings under Custom Lighting
	UPROPERTY(EditAnywhere, Category = "Custom Lighting")
	bool bCombineLightingAndColor = false;

	// (CustomLightingValue * {Power}) * BaseColor
	// Lower power values produces custom lighting as 1, so it doesn't change the base color;
	// Power as 1 uses original custom lighting, so it affects the brightness of base color.
	UPROPERTY(EditAnywhere, Category = "Custom Lighting", Meta = (EditCondition = "bCombineLightingAndColor", EditConditionHides, ClampMin = "0.001"))
	float CustomLightingPower = 1.0f;

	// 0 - custom lighting won't affect base color, 1 - fully affects base color
	UPROPERTY(EditAnywhere, Category = "Custom Lighting", Meta = (EditCondition = "bCombineLightingAndColor", EditConditionHides, ClampMin = "0", ClampMax = "1"))
	float CustomLightingOpacity = 1.0f;

	// Multiplies custom lighting value (allows to brighten/darken the object)
	UPROPERTY(EditAnywhere, Category = "Custom Lighting", Meta = (EditCondition = "bCombineLightingAndColor", EditConditionHides, ClampMin = "0"))
	float CustomLightingMultiplier = 1.0f;

	// Saturate custom lighting value.
	UPROPERTY(EditAnywhere, Category = "Custom Lighting", Meta = (EditCondition = "bCombineLightingAndColor", EditConditionHides, ClampMin = "0", ClampMax = "1"))
	float CustomLightingSaturation = 1.0f;


	// Orthographic is the most accurate, but some shader effects require a perspective view. The Camera Distance should be set as far back as it can without introducing too many Z fighting artifacts.
	UPROPERTY(EditAnywhere, Category = "Projection")
	TEnumAsByte<ECameraProjectionMode::Type> ProjectionType = ECameraProjectionMode::Perspective;

	UPROPERTY(EditAnywhere, Category = "Projection", Meta = (EditCondition = "ProjectionType == ECameraProjectionMode::Perspective", EditConditionHides))
	EImpostorPerspectiveCameraType PerspectiveCameraType = EImpostorPerspectiveCameraType::FOV;

	// Distance at which to capture. The Projection Distance should be set as far back as it can without introducing too many Z fighting artifacts.
	UPROPERTY(EditAnywhere, Category = "Projection", Meta = (EditCondition = "ProjectionType == ECameraProjectionMode::Perspective", EditConditionHides))
	float CameraDistance = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "Projection", Meta = (EditCondition = "ProjectionType == ECameraProjectionMode::Perspective", EditConditionHides))
	float CameraFOV = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Advanced")
	bool bPreviewCaptureSphere = false;

	// Optionally disable mesh cutouts. Useful for debugging, such as to see the whole debug triangle view.
	UPROPERTY(EditAnywhere, Category = "Advanced")
	bool bUseMeshCutout = true;

	UPROPERTY(EditAnywhere, Category = "Advanced")
	EImpostorMeshOffsetType MeshOffsetType = EImpostorMeshOffsetType::None;

	UPROPERTY(EditAnywhere, Category = "Advanced", Meta = (EditCondition = "MeshOffsetType == EImpostorMeshOffsetType::CustomOffset", EditConditionHides))
	FVector2D CustomMeshOffset = FVector2D::ZeroVector;

	// Allows higher quality silhouette blending as well as subsurface edge effects. Makes rendering take much longer.
	UPROPERTY(EditAnywhere, Category = "Advanced")
	bool bUseDistanceFieldAlpha = true;

	// Resolution for scene capturing (single frame before compositing into one texture). Generally should be slightly higher than sub frame resolution. Large sizes (>512) will take a long time to render due to distance field calculation
	UPROPERTY(EditAnywhere, Category = "Advanced")
	int32 SceneCaptureResolution = 512;

	UPROPERTY(VisibleAnywhere, Category = "Advanced", AdvancedDisplay)
	int32 SceneCaptureMips = 9;

	UPROPERTY(VisibleAnywhere, Category = "Advanced", AdvancedDisplay)
	int32 CutoutMipTarget = 5;

	UPROPERTY(VisibleAnywhere, Category = "Advanced", AdvancedDisplay)
	int32 CheckTargetMipSize = 16;

	UPROPERTY(VisibleAnywhere, Category = "Advanced", AdvancedDisplay)
	int32 DFMipTarget = 8;

	// Will display vertices with their data in viewport
	UPROPERTY(VisibleAnywhere, Category = "Advanced", AdvancedDisplay)
	bool bDisplayVertices = false;
};
