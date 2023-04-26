// Fill out your copyright notice in the Description page of Project Settings.

#include "ImpostorProceduralMeshManager.h"

#include "ImpostorBakerUtilities.h"
#include "ImpostorMaterialsManager.h"
#include "ImpostorComponentsManager.h"
#include "ImpostorRenderTargetsManager.h"

#include "MeshDescription.h"
#include "AssetToolsModule.h"
#include "MaterialInstanceDynamic.h"
#include "ProceduralMeshComponent.h"
#include "ProceduralMeshConversion.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Materials/MaterialInstanceConstant.h"
#include "BlueprintMaterialTextureNodesBPLibrary.h"

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

	TArray<FVector> CardNormalList = GetNormalCards();

	if (ImpostorData->ManualPoints.Num() == 0)
	{
		FImpostorTextureData TextureData;
		for (int32 NormalsIndex = 0; NormalsIndex < CardNormalList.Num(); NormalsIndex++)
		{
			TextureData = BakeAlphasData(NormalsIndex, TextureData);

			TArray<FVector2D> Points;
			Points.Reserve(8);

			for (int32 CornerIndex = 0; CornerIndex < 4; CornerIndex++)
			{
				float MinSlope = 1000.f;
				TArray<float> MinSlopes{FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX};
				int32 CornerX = -1;
				int32 CornerY = -1;

				// Finds potential cut angles with non-intersecting edges
				for (int32 Index = 0; Index < 8; Index++)
				{
					int32 X = 0;
					int32 Y = 0;

					FImpostorBakerUtilities::TraceUntilIntersection(CornerIndex, TextureData, ImpostorData->ImpostorType, Index, X, Y);

					// Find the first intersection or "Corner"
					if (Y < 16 &&
						CornerX == -1 &&
						CornerY == -1)
					{
						CornerX = X;
						CornerY = Y;
						continue;
					}

					// Once a corner is found, trace down in lines, finding intersection slopes and proposed alternates
					if (Y >= 16)
					{
						continue;
					}

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
					case 0: Points.Add(Degrees0); break;
					case 1: Points.Add(Degrees90 + FVector2D(1.f, 0.f)); break;
					case 2: Points.Add(Degrees180 + FVector2D(1.f, 1.f)); break;
					case 3: Points.Add(Degrees270 + FVector2D(0.f, 1.f)); break;
					}
				}

				Points.Add(FImpostorBakerUtilities::GetRotatedCoordsByCorner(FVector2D(1.f, MinSlope).GetSafeNormal(), 16, true, CornerIndex));
			}

			CutCorners(Points);
			GenerateMeshVerticesAndUVs(Points, NormalsIndex, CardNormalList[NormalsIndex]);
		}
	}

	if (ImpostorData->ManualPoints.Num() > 0)
	{
		TArray<FVector2D> Points;
		Points.Reserve(ImpostorData->ManualPoints.Num());

		const UImpostorComponentsManager* ComponentsManager = GetManager<UImpostorComponentsManager>();
		for (const FVector& Point : ImpostorData->ManualPoints)
		{
			FVector ObjectSize(ComponentsManager->ObjectRadius * -2.f, 0.f, -10.f);
			Points.Add(FVector2D((Point - ObjectSize + FVector(ComponentsManager->DebugTexelSize * 16.f / 2.f, ComponentsManager->DebugTexelSize * 16.f / 2.f, 0.f)) / ComponentsManager->DebugTexelSize));
		}

		GenerateMeshVerticesAndUVs(Points, 0, CardNormalList[0]);
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
		const UImpostorRenderTargetsManager* RenderTargetsManager = GetManager<UImpostorRenderTargetsManager>();
		RenderTargetsManager->ClearRenderTarget(RenderTargetsManager->CombinedAlphasRenderTarget);

		if (UTextureRenderTarget2D* RenderTarget = RenderTargetsManager->TargetMaps.FindRef(EImpostorBakeMapType::BaseColor))
		{
			RenderTargetsManager->ResampleRenderTarget(RenderTarget, RenderTargetsManager->CombinedAlphasRenderTarget);
		}

		return GetManager<UImpostorComponentsManager>()->ViewCaptureVectors;
	}

	return {FVector::UpVector};
}

FImpostorTextureData UImpostorProceduralMeshManager::BakeAlphasData(const int32 Index, const FImpostorTextureData& Data) const
{
	FLinearColor Rectangle;
	switch (ImpostorData->ImpostorType)
	{
	default: check(false);
	case EImpostorLayoutType::FullSphereView:
	case EImpostorLayoutType::UpperHemisphereOnly:
	{
		if (Data.SizeX != 0)
		{
			return Data;
		}

		Rectangle = FLinearColor(0.f, 0.f, 16.f, 16.f);
		break;
	}
	case EImpostorLayoutType::TraditionalBillboards:
	{
		FVector2D VectorIndex(Index % 3, FMath::Floor(Index / 3));
		VectorIndex *= 16.f;
		Rectangle = FLinearColor(VectorIndex.X, VectorIndex.Y, VectorIndex.X + 16.f, VectorIndex.Y + 16.f);
		break;
	}
	}

	TArray<FLinearColor> ColorData = UBlueprintMaterialTextureNodesBPLibrary::RenderTarget_SampleRectangle_EditorOnly(GetManager<UImpostorRenderTargetsManager>()->CombinedAlphasRenderTarget, Rectangle);

	if (!ImpostorData->bUseMeshCutout)
	{
		for (int32 ColorIndex = 0; ColorIndex < ColorData.Num(); ColorIndex++)
		{
			ColorData[ColorIndex] = FLinearColor::White;
		}
	}

	FImpostorTextureData Result;
	Result.SizeX = 16;
	Result.SizeY = 16;
	Result.Colors = ColorData;

	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UImpostorProceduralMeshManager::CutCorners(TArray<FVector2D>& Points) const
{
	// Cut Corners
	for (int32 CornerIndex = 0; CornerIndex < 4; CornerIndex++)
	{
		const int32 PreviousCornerIndex = CornerIndex * 2;
		const int32 CurrentCornerIndex = CornerIndex * 2 + 1;
		const int32 NextCornerIndex = (CornerIndex * 2 + 2) % Points.Num();
		if (!ensure(Points.IsValidIndex(PreviousCornerIndex)) ||
			!ensure(Points.IsValidIndex(CurrentCornerIndex)) ||
			!ensure(Points.IsValidIndex(NextCornerIndex)))
		{
			continue;
		}

		const FVector2D& PreviousCorner = Points[PreviousCornerIndex];
		const FVector2D& CurrentCorner = Points[CurrentCornerIndex];
		const FVector2D& NextCorner = Points[NextCornerIndex];

		FVector2D Result = FVector2D::ZeroVector;
		switch (CornerIndex)
		{
		default: check(false);
		case 0:
		case 2:
		{
			if (CurrentCorner.Y != 0.f)
			{
				Result = CurrentCorner / CurrentCorner.Y * (NextCorner.Y - PreviousCorner.Y);
			}
			break;
		}
		case 1:
		case 3:
		{
			if (CurrentCorner.X != 0.f)
			{
				Result = CurrentCorner / CurrentCorner.X * (NextCorner.X - PreviousCorner.X);
			}
			break;
		}
		}

		Result += PreviousCorner;
		Points[CurrentCornerIndex] = Result;
	}
}

void UImpostorProceduralMeshManager::GenerateMeshVerticesAndUVs(const TArray<FVector2D>& Points, const int32 CardIndex, const FVector& CardNormal)
{
	const int32 VertexOffset = Vertices.Num();
	const int32 NumNewVertices = Points.Num() + 1;

	const UImpostorComponentsManager* ComponentsManager = GetManager<UImpostorComponentsManager>();

	switch (ImpostorData->ImpostorType)
	{
	default: check(false);
	case EImpostorLayoutType::FullSphereView:
	case EImpostorLayoutType::UpperHemisphereOnly: Vertices.Add(ComponentsManager->OffsetVector);
		break;
	case EImpostorLayoutType::TraditionalBillboards: Vertices.Add(ComponentsManager->OffsetVector + ((ImpostorData->BillboardTopOffsetCenter * ComponentsManager->ObjectRadius) + (ImpostorData->BillboardTopOffset * ComponentsManager->ObjectRadius)) * FVector::UpVector * (CardIndex == 4 ? 1.f : 0.f));
		break;
	}

	switch (ImpostorData->ImpostorType)
	{
	default: check(false);
	case EImpostorLayoutType::FullSphereView:
	case EImpostorLayoutType::UpperHemisphereOnly: UVs.Add(FVector2D(0.05f, 0.05f) / ComponentsManager->NumFrames);
		break;
	case EImpostorLayoutType::TraditionalBillboards: UVs.Add((FVector2D(0.5f, 0.5f) / ComponentsManager->NumFrames) + (FVector2D(CardIndex % 3, FMath::Floor(CardIndex / 3)) / 3.f) + (CardIndex == 4 ? 1.f : 0.f));
		break;
	}

	const FProcMeshTangent Tangent = GetTangent(CardNormal);
	Tangents.Add(Tangent);

	const FVector Normal = GetNormalsVector(CardNormal);
	Normals.Add(Normal);

	// Construct Vertices from Cut Points
	for (const FVector2D& Point : Points)
	{
		Vertices.Add(GetVertex(Point, CardIndex, CardNormal, ComponentsManager->OffsetVector, ComponentsManager->ObjectRadius));
		UVs.Add(GetUV(CardIndex, Point, ComponentsManager->NumFrames));
		Normals.Add(Normal);
		Tangents.Add(Tangent);
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

	switch (ImpostorData->ImpostorType)
	{
	default: check(false);
	case EImpostorLayoutType::FullSphereView:
	case EImpostorLayoutType::UpperHemisphereOnly: return FVector(Point * ObjectRadius / 10.f, 0.f) + OffsetVector;
	case EImpostorLayoutType::TraditionalBillboards:
	{
		FVector X, Y, Z;
		FImpostorBakerUtilities::DeriveAxes(CardNormal, X, Y, Z);
		return (((Point.X * X) + (Point.Y * Y)) * ObjectRadius + OffsetVector + ((ImpostorData->BillboardTopOffset * ObjectRadius) * (CardIndex == 4 ? 1.f : 0.f) * FVector::UpVector));
	}
	}
}

FVector2D UImpostorProceduralMeshManager::GetUV(const int32 CardIndex, const FVector2D& Point, const int32 NumFrames) const
{
	FVector2D TargetPoint = Point / 16.f;
	TargetPoint.X = FMath::Clamp(TargetPoint.X, 0.f, 1.f);
	TargetPoint.Y = FMath::Clamp(TargetPoint.Y, 0.f, 1.f);
	TargetPoint /= NumFrames;

	switch (ImpostorData->ImpostorType)
	{
	default: check(false);
	case EImpostorLayoutType::FullSphereView:
	case EImpostorLayoutType::UpperHemisphereOnly: return TargetPoint / 10.f;
	case EImpostorLayoutType::TraditionalBillboards: return TargetPoint + FVector2D(CardIndex % 3, FMath::Floor(CardIndex / 3.f)) / 3.f + (CardIndex == 4 ? 1.f : 0.f);
	}
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