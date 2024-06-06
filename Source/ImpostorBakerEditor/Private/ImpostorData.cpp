// Fill out your copyright notice in the Description page of Project Settings.

#include "ImpostorData.h"
#include <AssetRegistry/AssetData.h>
#include <Engine/StaticMesh.h>
#include <PackageTools.h>
#include "ImpostorBakerSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ImpostorData)

void UImpostorData::PreEditChange(FProperty* PropertyAboutToChange)
{
	Super::PreEditChange(PropertyAboutToChange);

	if (PropertyAboutToChange->GetFName() == GET_MEMBER_NAME_CHECKED(UImpostorData, MapsToRender))
	{
		bNeedUpdateCustomLightingBool = false;

		if (bCombineLightingAndColor)
		{
			if (MapsToRender.Contains(EImpostorBakeMapType::BaseColor) &&
				MapsToRender.Contains(EImpostorBakeMapType::CustomLighting))
			{
				bNeedUpdateCustomLightingBool = true;
			}
		}
		else
		{
			if (!MapsToRender.Contains(EImpostorBakeMapType::BaseColor) ||
				!MapsToRender.Contains(EImpostorBakeMapType::CustomLighting))
			{
				bNeedUpdateCustomLightingBool = true;
			}
		}
	}
}

void UImpostorData::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.ChangeType == EPropertyChangeType::Interactive)
	{
		if (const FSimpleMulticastDelegate* Delegate = OnPropertyInteractiveChange.Find(PropertyChangedEvent.GetMemberPropertyName()))
		{
			Delegate->Broadcast();
		}

		return;
	}

	if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(UImpostorData, Resolution))
	{
		Resolution = 1 << FMath::Clamp(FMath::FloorLog2(Resolution), 9, 14);
	}
	else if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(UImpostorData, FrameSize))
	{
		FrameSize = 1 << FMath::Clamp(FMath::FloorLog2(FrameSize), 4, 11);
	}
	else if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(UImpostorData, SceneCaptureResolution))
	{
		const uint32 Mip = FMath::Clamp(FMath::FloorLog2(Resolution), 7, 11);
		SceneCaptureResolution = 1 << Mip;
		SceneCaptureMips = Mip;
		CutoutMipTarget = Mip - 4;
		DFMipTarget = Mip - 1;
	}
	else if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(UImpostorData, MapsToRender))
	{
		if (MapsToRender.Contains(EImpostorBakeMapType::None))
		{
			MapsToRender.Remove(EImpostorBakeMapType::None);
			for (EImpostorBakeMapType Enum : TEnumRange<EImpostorBakeMapType>())
			{
				if (!MapsToRender.Contains(Enum))
				{
					MapsToRender.Add(Enum);
					break;
				}
			}
		}

		if (bNeedUpdateCustomLightingBool)
		{
			bNeedUpdateCustomLightingBool = false;
			if (bCombineLightingAndColor)
			{
				if (!MapsToRender.Contains(EImpostorBakeMapType::BaseColor) ||
					!MapsToRender.Contains(EImpostorBakeMapType::CustomLighting))
				{
					bCombineLightingAndColor = false;
				}
			}
			else
			{
				if (MapsToRender.Contains(EImpostorBakeMapType::BaseColor) &&
					MapsToRender.Contains(EImpostorBakeMapType::CustomLighting))
				{
					bCombineLightingAndColor = true;
				}
			}
		}
	}
	else if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(UImpostorData, ReferencedMesh))
	{
		UpdateMeshData(ReferencedMesh.Get());
	}
	else if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(UImpostorData, TargetLOD))
	{
		TargetLOD = FMath::Min(TargetLOD, ReferencedMesh->GetNumLODs());
	}
	else if (
		PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(UImpostorData, PerspectiveCameraType) ||
		PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(UImpostorData, CameraDistance) ||
		PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(UImpostorData, CameraFOV))
	{
		UpdateFOVDistance();
	}
	else if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(UImpostorData, bGenerateTwoSidedGeometry))
	{
		BillboardMaterial = bGenerateTwoSidedGeometry ? GetDefault<UImpostorBakerSettings>()->DefaultBillboardMaterial.LoadSynchronous() : GetDefault<UImpostorBakerSettings>()->DefaultBillboardTwoSidedMaterial.LoadSynchronous();
	}

	if (const FSimpleMulticastDelegate* Delegate = OnPropertyInteractiveChange.Find(PropertyChangedEvent.GetMemberPropertyName()))
	{
		Delegate->Broadcast();
		return;
	}

	if (const FSimpleMulticastDelegate* Delegate = OnPropertyChange.Find(PropertyChangedEvent.GetMemberPropertyName()))
	{
		Delegate->Broadcast();
	}

	OnSettingsChange.ExecuteIfBound();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UImpostorData::AssignMesh(const FAssetData& MeshAssetData)
{
	FullSphereMaterial = GetDefault<UImpostorBakerSettings>()->DefaultFullSphereMaterial.LoadSynchronous();
	UpperHemisphereMaterial = GetDefault<UImpostorBakerSettings>()->DefaultUpperHemisphereMaterial.LoadSynchronous();
	BillboardMaterial = bGenerateTwoSidedGeometry ? GetDefault<UImpostorBakerSettings>()->DefaultBillboardMaterial.LoadSynchronous() : GetDefault<UImpostorBakerSettings>()->DefaultBillboardTwoSidedMaterial.LoadSynchronous();
	EnableWPOParameterName = GetDefault<UImpostorBakerSettings>()->EnableWPOParameterName;

	ReferencedMesh = Cast<UStaticMesh>(MeshAssetData.GetAsset());
	UpdateMeshData(ReferencedMesh.Get());
}

void UImpostorData::UpdateMeshData(const FAssetData& MeshAssetData)
{
	SaveLocation.Path = MeshAssetData.PackagePath.ToString();

	FString AssetName = MeshAssetData.AssetName.ToString();
	AssetName.RemoveFromStart("SM_");

	NewMaterialName = "MI_" + AssetName + "_Impostor";
	NewTextureName = "T_" + AssetName + "_Impostor";
	NewMeshName = "SM_" + AssetName + "_Impostor";

	if (ReferencedMesh)
	{
		TargetLOD = ReferencedMesh->GetNumLODs();
	}

	UpdateFOVDistance();
}

UMaterialInterface* UImpostorData::GetMaterial() const
{
	switch (ImpostorType)
	{
	default: check(false);
	case EImpostorLayoutType::FullSphereView: return FullSphereMaterial;
	case EImpostorLayoutType::UpperHemisphereOnly: return UpperHemisphereMaterial;
	case EImpostorLayoutType::TraditionalBillboards: return BillboardMaterial;
	}
}

FString UImpostorData::GetPackage(const FString& AssetName)
{
	if (SaveLocation.Path.EndsWith("/"))
	{
		return FPaths::Combine(SaveLocation.Path, AssetName);
	}
	else
	{
		return FPaths::Combine(SaveLocation.Path, "/", AssetName);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UImpostorData::UpdateFOVDistance()
{
	if (!ReferencedMesh)
	{
		return;
	}

	if (PerspectiveCameraType == EImpostorPerspectiveCameraType::Both)
	{
		return;
	}

	const float Radius = ReferencedMesh->GetBounds().SphereRadius + GetMeshOffset().GetAbsMax();

	if (PerspectiveCameraType == EImpostorPerspectiveCameraType::Distance)
	{
		CameraFOV = ((180.f / UE_DOUBLE_PI) * FMath::Atan(Radius / CameraDistance)) * 2.f;
		return;
	}

	CameraDistance = Radius / FMath::Tan(CameraFOV / 2.f / (180.f / UE_DOUBLE_PI));
}

FVector2D UImpostorData::GetMeshOffset() const
{
	FVector2D Offset = FVector2D::ZeroVector;
	switch (MeshOffsetType)
	{
	case EImpostorMeshOffsetType::MeshOrigin:
	{
		if (ReferencedMesh)
		{
			Offset = FVector2D(ReferencedMesh->GetBounds().Origin.X, ReferencedMesh->GetBounds().Origin.Y);
		}
		break;
	}
	case EImpostorMeshOffsetType::CustomOffset: Offset = CustomMeshOffset; break;
	case EImpostorMeshOffsetType::None: break;
	}

	return Offset;
}
