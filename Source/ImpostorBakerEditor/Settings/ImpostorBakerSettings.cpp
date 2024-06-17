#include "ImpostorBakerSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ImpostorBakerSettings)

UImpostorBakerSettings::UImpostorBakerSettings()
{
	static FString Materials = TEXT("/ImpostorBaker/Materials/");
	static FString Generation = Materials + TEXT("Generation/");

	BufferPostProcessMaterials = {
		{EImpostorBakeMapType::BaseColor,	TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath("/Engine/BufferVisualization/FinalImage.FinalImage"))},
		{EImpostorBakeMapType::Metallic,	TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath("/Engine/BufferVisualization/Metallic.Metallic"))},
		{EImpostorBakeMapType::Specular,	TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath("/Engine/BufferVisualization/Specular.Specular"))},
		{EImpostorBakeMapType::Roughness,	TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath("/Engine/BufferVisualization/Roughness.Roughness"))},
		{EImpostorBakeMapType::Opacity,	TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath("/Engine/BufferVisualization/Opacity.Opacity"))},
		{EImpostorBakeMapType::Subsurface,	TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath("/Engine/BufferVisualization/SubsurfaceColor.SubsurfaceColor"))},
		{EImpostorBakeMapType::Normal,		TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(Generation + "M_PP_ImpostorBake_Normals.M_PP_ImpostorBake_Normals"))},
		{EImpostorBakeMapType::Depth,		TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(Generation + "M_PP_ImpostorBake_Depth.M_PP_ImpostorBake_Depth"))},
	};

	DefaultFullSphereMaterial			= TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(Materials + "MI_Impostor_Sphere.MI_Impostor_Sphere"));
	DefaultUpperHemisphereMaterial		= TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(Materials + "MI_Impostor_Hemisphere.MI_Impostor_Hemisphere"));
	DefaultBillboardMaterial			= TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(Materials + "M_Billboard.M_Billboard"));
	DefaultBillboardTwoSidedMaterial	= TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(Materials + "MI_Billboard_TwoSided.MI_Billboard_TwoSided"));
	BaseColorCustomLightingMaterial		= TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(Generation + "M_ImpostorBake_CombineBaseColorCustomLighting.M_ImpostorBake_CombineBaseColorCustomLighting"));
	CombinedNormalsDepthMaterial		= TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(Generation + "M_ImpostorBake_CombineNormalsDepth.M_ImpostorBake_CombineNormalsDepth"));
	SampleFrameMaterial					= TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(Generation + "M_ImpostorBake_SampleFrame.M_ImpostorBake_SampleFrame"));
	AddAlphasMaterial					= TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(Generation + "M_ImpostorBake_AddAlpha.M_ImpostorBake_AddAlpha"));
	ResampleMaterial					= TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(Generation + "M_ImpostorBake_ResampleRT.M_ImpostorBake_ResampleRT"));
	AddAlphaFromFinalColor				= TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(Generation + "M_ImpostorBake_AddAlphaFromFinalColor.M_ImpostorBake_AddAlphaFromFinalColor"));

	ImpostorPreviewMapNames = {
		{EImpostorBakeMapType::BaseColor,		TEXT("BaseColor")},
		{EImpostorBakeMapType::Metallic,		TEXT("Metallic")},
		{EImpostorBakeMapType::Specular,		TEXT("Specular")},
		{EImpostorBakeMapType::Roughness,		TEXT("Roughness")},
		{EImpostorBakeMapType::Opacity,		TEXT("Opacity")},
		{EImpostorBakeMapType::Normal,			TEXT("Normal")},
		{EImpostorBakeMapType::Subsurface,		TEXT("Subsurface")},
		{EImpostorBakeMapType::Depth,			TEXT("Depth")},
		{EImpostorBakeMapType::CustomLighting,	TEXT("CustomLighting")},
	};
}

void UImpostorBakerSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		TryUpdateDefaultConfigFile();
	}
}
