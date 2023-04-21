// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ImpostorBaseManager.h"
#include "ImpostorProceduralMeshManager.generated.h"

class UProceduralMeshComponent;
class UMaterialInstanceConstant;
struct FProcMeshTangent;

USTRUCT()
struct FImpostorTextureData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Transient)
	int32 SizeX = 0;

	UPROPERTY(VisibleAnywhere, Transient)
	int32 SizeY = 0;

	UPROPERTY(Transient)
	TArray<FLinearColor> Colors;

	FLinearColor GetColor(const FVector2D& VectorIndex) const
	{
		const int32 Index = FMath::Floor(VectorIndex.X) + SizeX * FMath::Floor(VectorIndex.Y);
		if (!ensure(Colors.IsValidIndex(Index)))
		{
			return FLinearColor::White;
		}

		return Colors[Index];
	}
};

UCLASS()
class IMPOSTORBAKEREDITOR_API UImpostorProceduralMeshManager : public UImpostorBaseManager
{
	GENERATED_BODY()

public:
	//~ Begin UImpostorBaseManager Interface
	virtual void Initialize() override;
	virtual void Update() override;
	//~ End UImpostorBaseManager Interface

	void SaveMesh(UMaterialInstanceConstant* NewMaterial) const;
	void UpdateLOD(UMaterialInstanceConstant* NewMaterial) const;

private:
	UStaticMesh* CreateMesh(UMaterialInstanceConstant* NewMaterial, UObject* TargetPacket, const FString& AssetName) const;
	void GenerateMeshData();

	TArray<FVector> GetNormalCards() const;
	FImpostorTextureData BakeAlphasData(int32 Index, const FImpostorTextureData& Data) const;

	void CutCorners(TArray<FVector2D>& Points) const;

	void GenerateMeshVerticesAndUVs(const TArray<FVector2D>& Points, int32 CardIndex, const FVector& CardNormal);
	FVector GetVertex(FVector2D Point, int32 CardIndex, const FVector& CardNormal, const FVector& OffsetVector, float ObjectRadius) const;
	FVector2D GetUV(int32 CardIndex, const FVector2D& Point, int32 NumFrames) const;
	FVector GetNormalsVector(const FVector& CardNormal) const;
	FProcMeshTangent GetTangent(const FVector& CardNormal) const;

private:
	UPROPERTY(Transient)
	UProceduralMeshComponent* MeshComponent;

public:
	UPROPERTY(VisibleAnywhere, Transient, Category = "Procedural Mesh Data")
	TArray<FVector> Vertices;

	UPROPERTY(VisibleAnywhere, Transient, Category = "Procedural Mesh Data")
	TArray<FVector> Normals;

	UPROPERTY(VisibleAnywhere, Transient, Category = "Procedural Mesh Data")
	TArray<FVector2D> UVs;

	UPROPERTY(VisibleAnywhere, Transient, Category = "Procedural Mesh Data")
	TArray<int32> Triangles;

	UPROPERTY(VisibleAnywhere, Transient, Category = "Procedural Mesh Data")
	TArray<FProcMeshTangent> Tangents;
};
