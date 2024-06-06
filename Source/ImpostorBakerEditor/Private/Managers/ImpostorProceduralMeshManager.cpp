// Fill out your copyright notice in the Description page of Project Settings.

#include "ImpostorProceduralMeshManager.h"
#include <AssetRegistry/AssetRegistryModule.h>
#include <AssetToolsModule.h>
#include <Engine/StaticMesh.h>
#include <Engine/TextureRenderTarget2D.h>
#include <Materials/MaterialInstanceConstant.h>
#include <Materials/MaterialInstanceDynamic.h>
#include <MeshDescription.h>
#include <PhysicsEngine/BodySetup.h>
#include <ProceduralMeshComponent.h>
#include <ProceduralMeshConversion.h>
#include <StaticMeshResources.h>
#include <TextureResource.h>
#include "ImpostorBakerUtilities.h"
#include "ImpostorComponentsManager.h"
#include "ImpostorMaterialsManager.h"
#include "ImpostorRenderTargetsManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ImpostorProceduralMeshManager)

void UImpostorProceduralMeshManager::Initialize()
{
	MeshComponent = NewObject<UProceduralMeshComponent>(GetTransientPackage());
	AddComponent(MeshComponent);
}

void UImpostorProceduralMeshManager::Update()
{
	MeshComponent->SetRelativeLocation(FVector(GetManager<UImpostorComponentsManager>()->ObjectRadius * 2.25f, 0.f, 10.f));

	GenerateMeshData();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UImpostorProceduralMeshManager::SaveMesh(UMaterialInstanceConstant* NewMaterial) const
{
	ProgressSlowTask("Creating impostor static mesh...", true);
	if (!ensure(MeshComponent))
	{
		return;
	}

	const FString AssetName = ImpostorData->NewMeshName;
	const FString PackageName = ImpostorData->GetPackage(AssetName);

	UPackage* Package = CreatePackage(*PackageName);
	if (!ensure(Package))
	{
		return;
	}

	UStaticMesh* NewMesh = FindObject<UStaticMesh>(Package, *AssetName);
	if (NewMesh)
	{
		FMeshDescription MeshDescription = BuildMeshDescription(MeshComponent);

		if (!ensure(MeshDescription.Polygons().Num() > 0))
		{
			return;
		}

		{
			FStaticMeshSourceModel& SrcModel = NewMesh->GetSourceModel(0);
			SrcModel.BuildSettings.bRecomputeNormals = false;
			SrcModel.BuildSettings.bRecomputeTangents = false;
			SrcModel.BuildSettings.bRemoveDegenerates = false;
			SrcModel.BuildSettings.bUseHighPrecisionTangentBasis = false;
			SrcModel.BuildSettings.bUseFullPrecisionUVs = false;
			SrcModel.BuildSettings.bGenerateLightmapUVs = true;
			SrcModel.BuildSettings.SrcLightmapIndex = 0;
			SrcModel.BuildSettings.DstLightmapIndex = 1;
			NewMesh->CreateMeshDescription(0, MoveTemp(MeshDescription));
			NewMesh->CommitMeshDescription(0);
		}

		if (!MeshComponent->bUseComplexAsSimpleCollision)
		{
			UBodySetup* NewBodySetup = NewMesh->GetBodySetup();
			NewBodySetup->BodySetupGuid = FGuid::NewGuid();
			NewBodySetup->AggGeom.ConvexElems = MeshComponent->ProcMeshBodySetup->AggGeom.ConvexElems;
			NewBodySetup->bGenerateMirroredCollision = false;
			NewBodySetup->bDoubleSidedGeometry = true;
			NewBodySetup->CollisionTraceFlag = CTF_UseDefault;
			NewBodySetup->CreatePhysicsMeshes();
		}

		const int32 NumSections = NewMesh->GetNumSections(0);
		for (int32 SectionIndex = 0; SectionIndex < NumSections; SectionIndex++)
		{
			FMeshSectionInfo SectionInfo = NewMesh->GetSectionInfoMap().Get(0, SectionIndex);
			SectionInfo.bCastShadow = ImpostorData->bMeshCastShadow;
			NewMesh->GetSectionInfoMap().Set(0, SectionIndex, SectionInfo);
		}

		NewMesh->Build(false);
		return;
	}

	NewMesh = CreateMesh(NewMaterial, Package, AssetName);
	if (!ensure(NewMesh))
	{
		return;
	}

	NewMesh->MarkPackageDirty();

	FAssetRegistryModule::AssetCreated(NewMesh);
}

void UImpostorProceduralMeshManager::UpdateLOD(UMaterialInstanceConstant* NewMaterial) const
{
	ProgressSlowTask("Adding impostor mesh as a LOD" + LexToString(ImpostorData->TargetLOD) + " to referenced mesh...", true);
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	UStaticMesh* Mesh = ImpostorData->ReferencedMesh;
	if (!Mesh)
	{
		return;
	}

	FString AssetName = ImpostorData->NewMeshName;
	FString PackageName = ImpostorData->GetPackage(AssetName);
	AssetTools.CreateUniqueAssetName(PackageName, "", PackageName, AssetName);

	const UStaticMesh* NewMesh = CreateMesh(NewMaterial, Mesh, AssetName);
	if (!ensure(NewMesh))
	{
		return;
	}

	if (ImpostorData->TargetLOD >= Mesh->GetNumSourceModels())
	{
		FStaticMeshSourceModel& SrcModel = Mesh->AddSourceModel();
		SrcModel.BuildSettings.bRecomputeNormals = false;
		SrcModel.BuildSettings.bRecomputeTangents = false;
		SrcModel.BuildSettings.bRemoveDegenerates = false;
		SrcModel.BuildSettings.bUseHighPrecisionTangentBasis = false;
		SrcModel.BuildSettings.bUseFullPrecisionUVs = false;
		SrcModel.BuildSettings.bGenerateLightmapUVs = true;
		SrcModel.BuildSettings.SrcLightmapIndex = 0;
		SrcModel.BuildSettings.DstLightmapIndex = 1;
	}

	Mesh->SetCustomLOD(NewMesh, ImpostorData->TargetLOD, "");

	int32 MaterialIndex = Mesh->GetMaterialIndex(*NewMaterial->GetName());
	if (MaterialIndex == -1)
	{
		const FName SlotName = Mesh->AddMaterial(NewMaterial);
		MaterialIndex = Mesh->GetMaterialIndex(SlotName);
	}
	else
	{
		Mesh->SetMaterial(MaterialIndex, NewMaterial);
	}

	const int32 NumSections = Mesh->GetNumSections(ImpostorData->TargetLOD);
	for (int32 SectionIndex = 0; SectionIndex < NumSections; SectionIndex++)
	{
		FMeshSectionInfo SectionInfo = Mesh->GetSectionInfoMap().Get(ImpostorData->TargetLOD, SectionIndex);
		SectionInfo.bCastShadow = ImpostorData->bMeshCastShadow;
		SectionInfo.MaterialIndex = MaterialIndex;
		Mesh->GetSectionInfoMap().Set(ImpostorData->TargetLOD, SectionIndex, SectionInfo);
	}

	Mesh->Build(false);
	Mesh->PostEditChange();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UStaticMesh* UImpostorProceduralMeshManager::CreateMesh(UMaterialInstanceConstant* NewMaterial, UObject* TargetPacket, const FString& AssetName) const
{
	FMeshDescription MeshDescription = BuildMeshDescription(MeshComponent);

	if (!ensure(MeshDescription.Polygons().Num() > 0))
	{
		return nullptr;
	}

	UStaticMesh* NewMesh = NewObject<UStaticMesh>(TargetPacket, *AssetName, RF_Public | RF_Standalone);
	NewMesh->InitResources();

	NewMesh->SetLightingGuid();

	{
		FStaticMeshSourceModel& SrcModel = NewMesh->AddSourceModel();
		SrcModel.BuildSettings.bRecomputeNormals = false;
		SrcModel.BuildSettings.bRecomputeTangents = false;
		SrcModel.BuildSettings.bRemoveDegenerates = false;
		SrcModel.BuildSettings.bUseHighPrecisionTangentBasis = false;
		SrcModel.BuildSettings.bUseFullPrecisionUVs = false;
		SrcModel.BuildSettings.bGenerateLightmapUVs = true;
		SrcModel.BuildSettings.SrcLightmapIndex = 0;
		SrcModel.BuildSettings.DstLightmapIndex = 1;
		NewMesh->CreateMeshDescription(0, MoveTemp(MeshDescription));
		NewMesh->CommitMeshDescription(0);
	}

	if (!MeshComponent->bUseComplexAsSimpleCollision)
	{
		NewMesh->CreateBodySetup();

		UBodySetup* NewBodySetup = NewMesh->GetBodySetup();
		NewBodySetup->BodySetupGuid = FGuid::NewGuid();
		NewBodySetup->AggGeom.ConvexElems = MeshComponent->ProcMeshBodySetup->AggGeom.ConvexElems;
		NewBodySetup->bGenerateMirroredCollision = false;
		NewBodySetup->bDoubleSidedGeometry = true;
		NewBodySetup->CollisionTraceFlag = CTF_UseDefault;
		NewBodySetup->CreatePhysicsMeshes();
	}

	{
		const int32 NumSections = MeshComponent->GetNumSections();
		for (int32 SectionIdx = 0; SectionIdx < NumSections; SectionIdx++)
		{
			NewMesh->GetStaticMaterials().Add(NewMaterial);
		}
	}

	const int32 NumSections = NewMesh->GetNumSections(0);
	for (int32 SectionIndex = 0; SectionIndex < NumSections; SectionIndex++)
	{
		FMeshSectionInfo SectionInfo = NewMesh->GetSectionInfoMap().Get(0, SectionIndex);
		SectionInfo.bCastShadow = ImpostorData->bMeshCastShadow;
		NewMesh->GetSectionInfoMap().Set(0, SectionIndex, SectionInfo);
	}

	NewMesh->ImportVersion = LastVersion;

	NewMesh->Build(false);
	NewMesh->PostEditChange();

	return NewMesh;
}

void UImpostorProceduralMeshManager::GenerateMeshData()
{
	Vertices = {};
	Normals = {};
	UVs = {};
	Triangles = {};
	Tangents = {};
	Points = {};

	TArray<FVector> CardNormalList = GetNormalCards();

	for (int32 NormalsIndex = 0; NormalsIndex < CardNormalList.Num(); NormalsIndex++)
	{
		FImpostorTextureData TextureData;
		TextureData = BakeAlphasData(NormalsIndex, TextureData);

		TArray<FVector2D> LocalPoints;
		LocalPoints.Reserve(8);

		for (int32 CornerIndex = 0; CornerIndex < 4; CornerIndex++)
		{
			float MinSlope = FLT_MAX;
			TArray<float> MinSlopes{FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX};
			int32 CornerX = -1;
			int32 CornerY = -1;

			// Finds potential cut angles with non-intersecting edges
			for (int32 Index = 0; Index < 16; Index++)
			{
				int32 X = 0;
				int32 Y = 0;

				// If no intersection found in column, continue
				if (!FImpostorBakerUtilities::FindIntersection(CornerIndex, TextureData, ImpostorData->ImpostorType, Index, X, Y))
				{
					continue;
				}

				// If first intersection not yet set
				if (CornerX == -1 &&
					CornerY == -1)
				{
					CornerX = X;
					CornerY = Y;
					continue;
				}

				// How far away is the next intersection
				const float Run = X - CornerX;
				const float Rise = Y - CornerY;

				for (int32 RiseOffset = 0; RiseOffset < 4; RiseOffset++)
				{
					MinSlopes[RiseOffset] = FMath::Min(MinSlopes[RiseOffset], (Rise + RiseOffset) / Run);
				}
			}

			// Finds the most optimal potential slice. In theory anyways.
			float LargestArea = -FLT_MAX;
			int32 CornerOffset = -1;
			for (int32 CornerOffsetIndex = 0; CornerOffsetIndex < 4; CornerOffsetIndex++)
			{
				const float CornerOffsetBias = float(CornerY - CornerOffsetIndex);
				const float CurrentArea = MinSlopes[CornerOffsetIndex] == 0.f ? 0.f : CornerOffsetBias * FMath::Abs(CornerOffsetBias / MinSlopes[CornerOffsetIndex]) / 2.f;
				if (CurrentArea < LargestArea)
				{
					continue;
				}

				LargestArea = CurrentArea;
				CornerOffset = CornerOffsetIndex;
				MinSlope = MinSlopes[CornerOffsetIndex];
			}

			// Un-rotate final corners.
			{
				FVector2D Degrees0;
				FVector2D Degrees90;
				FVector2D Degrees180;
				FVector2D Degrees270;
				FImpostorBakerUtilities::GetRotatedCoords(FVector2D(CornerX, CornerY - CornerOffset), 16, false, Degrees0, Degrees90, Degrees180, Degrees270);

				switch (CornerIndex)
				{
				default: check(false);
				case 0: LocalPoints.Add(Degrees0); break;
				case 1: LocalPoints.Add(Degrees90 + FVector2D(1.f, 0.f)); break;
				case 2: LocalPoints.Add(Degrees180 + FVector2D(1.f, 1.f)); break;
				case 3: LocalPoints.Add(Degrees270 + FVector2D(0.f, 1.f)); break;
				}
			}

			LocalPoints.Add(FImpostorBakerUtilities::GetRotatedCoordsByCorner(FVector2D(1.f, MinSlope).GetSafeNormal(), 16, true, CornerIndex));
		}

		CutCorners(LocalPoints);
		Points.Add(CardNormalList[NormalsIndex], FImpostorPoints{ LocalPoints });
		GenerateMeshVerticesAndUVs(LocalPoints, NormalsIndex, CardNormalList[NormalsIndex]);
	}

	MeshComponent->ClearMeshSection(0);
	MeshComponent->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, {}, Tangents, false);
	MeshComponent->SetMaterial(0, GetManager<UImpostorMaterialsManager>()->ImpostorPreviewMaterial);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TArray<FVector> UImpostorProceduralMeshManager::GetNormalCards() const
{
	if (ImpostorData->ImpostorType == EImpostorLayoutType::TraditionalBillboards)
	{
		return GetManager<UImpostorComponentsManager>()->ViewCaptureVectors;
	}

	return { FVector::UpVector };
}

FImpostorTextureData UImpostorProceduralMeshManager::BakeAlphasData(const int32 Index, const FImpostorTextureData& Data) const
{
	if (Data.SizeX != 0)
	{
		return Data;
	}

	const FIntRect Rectangle(0, 0, 16, 16);

	TArray<float> Alphas;
	Alphas.Reserve(256);
	Alphas.SetNum(256);

	if (!ImpostorData->bUseMeshCutout)
	{
		for (int32 ColorIndex = 0; ColorIndex < Alphas.Num(); ColorIndex++)
		{
			Alphas[ColorIndex] = 1.f;
		}
	}
	else
	{
		FTextureRenderTargetResource* RTResource = GetManager<UImpostorRenderTargetsManager>()->CombinedAlphas[Index]->GameThread_GetRenderTargetResource();

		const FReadSurfaceDataFlags ReadPixelFlags(RCM_MinMax);

		TArray<FColor> Colors;
		RTResource->ReadPixels(Colors, ReadPixelFlags, Rectangle);
		for (int32 ColorIndex = 0; ColorIndex < FMath::Min(Alphas.Num(), Colors.Num()); ColorIndex++)
		{
			Alphas[ColorIndex] = float(Colors[ColorIndex].R) / 255.f;
		}
	}

	FImpostorTextureData Result;
	Result.SizeX = 16;
	Result.SizeY = 16;
	Result.Alphas = Alphas;

	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UImpostorProceduralMeshManager::CutCorners(TArray<FVector2D>& LocalPoints) const
{
	// Cut Corners
	for (int32 CornerIndex = 0; CornerIndex < 4; CornerIndex++)
	{
		const int32 PreviousCornerIndex = CornerIndex * 2;
		const int32 CurrentCornerIndex = CornerIndex * 2 + 1;
		const int32 NextCornerIndex = (CornerIndex * 2 + 2) % LocalPoints.Num();
		if (!ensure(LocalPoints.IsValidIndex(PreviousCornerIndex)) ||
			!ensure(LocalPoints.IsValidIndex(CurrentCornerIndex)) ||
			!ensure(LocalPoints.IsValidIndex(NextCornerIndex)))
		{
			continue;
		}

		const FVector2D& PreviousCorner = LocalPoints[PreviousCornerIndex];
		const FVector2D& CurrentCorner = LocalPoints[CurrentCornerIndex];
		const FVector2D& NextCorner = LocalPoints[NextCornerIndex];

		FVector2D Result = FVector2D::ZeroVector;
		if (CornerIndex % 2 == 0)
		{
			if (CurrentCorner.Y != 0.f)
			{
				Result = CurrentCorner / CurrentCorner.Y * (NextCorner.Y - PreviousCorner.Y);
			}
		}
		else
		{
			if (CurrentCorner.X != 0.f)
			{
				Result = CurrentCorner / CurrentCorner.X * (NextCorner.X - PreviousCorner.X);
			}
		}

		LocalPoints[CurrentCornerIndex] = Result + PreviousCorner;
	}
}

void UImpostorProceduralMeshManager::GenerateMeshVerticesAndUVs(const TArray<FVector2D>& LocalPoints, const int32 CardIndex, const FVector& CardNormal)
{
	const int32 VertexOffset = Vertices.Num();
	const int32 NumNewVertices = LocalPoints.Num() + 1;

	const UImpostorComponentsManager* ComponentsManager = GetManager<UImpostorComponentsManager>();
	const int32 BillboardTopCard = ComponentsManager->BillboardTopFrame;
	const FVector2D NumFrames(ComponentsManager->NumHorizontalFrames, ComponentsManager->NumVerticalFrames);

	switch (ImpostorData->ImpostorType)
	{
	default: check(false);
	case EImpostorLayoutType::FullSphereView:
	case EImpostorLayoutType::UpperHemisphereOnly: Vertices.Add(ComponentsManager->OffsetVector); break;
	case EImpostorLayoutType::TraditionalBillboards: Vertices.Add(ComponentsManager->OffsetVector + ((ImpostorData->BillboardTopOffsetCenter * ComponentsManager->ObjectRadius) + (ImpostorData->BillboardTopOffset * ComponentsManager->ObjectRadius)) * FVector::UpVector * (CardIndex == BillboardTopCard ? 1.f : 0.f)); break;
	}

	switch (ImpostorData->ImpostorType)
	{
	default: check(false);
	case EImpostorLayoutType::FullSphereView:
	case EImpostorLayoutType::UpperHemisphereOnly: UVs.Add(FVector2D(0.05f, 0.05f) / NumFrames); break;
	case EImpostorLayoutType::TraditionalBillboards: UVs.Add((FVector2D(0.5f, 0.5f) / NumFrames) + (ConvertIndexToGrid(CardIndex) / NumFrames) + (CardIndex == BillboardTopCard ? 1.f : 0.f)); break;
	}

	const FProcMeshTangent Tangent = GetTangent(CardNormal);
	Tangents.Add(Tangent);

	const FVector Normal = GetNormalsVector(CardNormal);
	Normals.Add(Normal);

	// Construct Vertices from Cut Points
	for (const FVector2D& Point : LocalPoints)
	{
		Vertices.Add(GetVertex(Point, CardIndex, CardNormal, ComponentsManager->OffsetVector, ComponentsManager->ObjectRadius));
		UVs.Add(GetUV(CardIndex, Point, NumFrames));
		Normals.Add(Normal);
		Tangents.Add(Tangent);
		Points[CardNormal].PointToVertex.Add(Point, Vertices.Last());
	}

	// Build Index Buffer
	for (int32 VertexIndex = 0; VertexIndex < NumNewVertices - 2; VertexIndex++)
	{
		Triangles.Add(VertexIndex + VertexOffset + 2);
		Triangles.Add(VertexIndex + VertexOffset + 1);
		Triangles.Add(VertexOffset);
	}

	Triangles.Add((NumNewVertices - 1) + VertexOffset);
	Triangles.Add(VertexOffset);
	Triangles.Add(VertexOffset + 1);
}

FVector UImpostorProceduralMeshManager::GetVertex(FVector2D Point, const int32 CardIndex, const FVector& CardNormal, const FVector& OffsetVector, const float ObjectRadius) const
{
	Point /= 16.f;

	Point.X = FMath::Clamp(Point.X, 0.f, 1.f);
	Point.Y = FMath::Clamp(Point.Y, 0.f, 1.f);

	Point = (Point - 0.5f) * 2.f;

	if (ImpostorData->ImpostorType != EImpostorLayoutType::TraditionalBillboards)
	{
		return FVector(Point * ObjectRadius / 10.f, 0.f) + OffsetVector;
	}

	FVector X, Y, Z;
	FImpostorBakerUtilities::DeriveAxes(CardNormal, X, Y, Z);

	FVector Result = ((Point.X * X) + (Point.Y * Y)) * ObjectRadius + OffsetVector;
	if (CardIndex == GetManager<UImpostorComponentsManager>()->BillboardTopFrame)
	{
		Result += ImpostorData->BillboardTopOffset * ObjectRadius * FVector::UpVector;
	}

	return Result;
}

FVector2D UImpostorProceduralMeshManager::GetUV(const int32 CardIndex, const FVector2D& Point, const FVector2D& NumFrames) const
{
	FVector2D TargetPoint = Point / 16.f;
	TargetPoint.X = FMath::Clamp(TargetPoint.X, 0.f, 1.f);
	TargetPoint.Y = FMath::Clamp(TargetPoint.Y, 0.f, 1.f);

	TargetPoint /= NumFrames;

	if (ImpostorData->ImpostorType != EImpostorLayoutType::TraditionalBillboards)
	{
		return TargetPoint / 10.f;
	}

	TargetPoint += ConvertIndexToGrid(CardIndex) / NumFrames;

	if (CardIndex == GetManager<UImpostorComponentsManager>()->BillboardTopFrame)
	{
		TargetPoint += FVector2D::One();
	}

	return TargetPoint;
}

FVector UImpostorProceduralMeshManager::GetNormalsVector(const FVector& CardNormal) const
{
	switch (ImpostorData->ImpostorType)
	{
	default: check(false);
	case EImpostorLayoutType::FullSphereView:
	case EImpostorLayoutType::UpperHemisphereOnly: return FVector::UpVector;
	case EImpostorLayoutType::TraditionalBillboards: return FImpostorBakerUtilities::DeriveAxis(CardNormal, EAxis::Z) * -1.f;
	}
}

FProcMeshTangent UImpostorProceduralMeshManager::GetTangent(const FVector& CardNormal) const
{
	switch (ImpostorData->ImpostorType)
	{
	default: check(false);
	case EImpostorLayoutType::FullSphereView:
	case EImpostorLayoutType::UpperHemisphereOnly: return FProcMeshTangent(FVector::ForwardVector, false);
	case EImpostorLayoutType::TraditionalBillboards: return FProcMeshTangent(FImpostorBakerUtilities::DeriveAxis(CardNormal, EAxis::X), false);
	}
}

FVector2D UImpostorProceduralMeshManager::ConvertIndexToGrid(const int32 Index) const
{
	const UImpostorComponentsManager* ComponentsManager = GetManager<UImpostorComponentsManager>();
	return FVector2D(Index % ComponentsManager->NumHorizontalFrames, FMath::Floor(Index / ComponentsManager->NumHorizontalFrames));
}