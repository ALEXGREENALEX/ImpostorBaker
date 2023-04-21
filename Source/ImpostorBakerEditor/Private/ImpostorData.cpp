// Fill out your copyright notice in the Description page of Project Settings.

#include "ImpostorData.h"
#include "AssetToolsModule.h"
#include "ImpostorBakerSettings.h"

void UImpostorData::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UObject::PostEditChangeProperty(PropertyChangedEvent);

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
	}
	else if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(UImpostorData, ReferencedMesh))
	{
		UpdateMeshData(ReferencedMesh);
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
	BillboardMaterial = GetDefault<UImpostorBakerSettings>()->DefaultBillboardMaterial.LoadSynchronous();

	ReferencedMesh = Cast<UStaticMesh>(MeshAssetData.GetAsset());
	UpdateMeshData(ReferencedMesh);
}

void UImpostorData::UpdateMeshData(const FAssetData& MeshAssetData)
{
	SaveLocation.Path = MeshAssetData.PackagePath.ToString();

	FString AssetName = MeshAssetData.AssetName.ToString();
	AssetName.RemoveFromStart("SM_");

	NewMaterialName = "MI_" + AssetName + "_Impostor";
	NewTextureName = "T_" + AssetName + "_Impostor";
	NewMeshName = "SM_" + AssetName + "_Impostor";
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

void UImpostorData::GetAssetPathName(const FString& AssetName, FString& OutPackageName, FString& OutAssetName)
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	const FString PackageName = FPaths::Combine(SaveLocation.Path, AssetName);
	AssetTools.CreateUniqueAssetName(PackageName, "", OutPackageName, OutAssetName);
}