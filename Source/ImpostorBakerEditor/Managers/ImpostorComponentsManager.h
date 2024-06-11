#pragma once

#include <CoreMinimal.h>
#include "ImpostorBaseManager.h"
#include "ImpostorComponentsManager.generated.h"

class UMaterialInstanceDynamic;
class UStaticMeshComponent;

UCLASS()
class IMPOSTORBAKEREDITOR_API UImpostorComponentsManager : public UImpostorBaseManager
{
	GENERATED_BODY()

public:
	//~ Begin UImpostorBaseManager Interface
	virtual void Initialize() override;
	virtual void Update() override;
	//~ End UImpostorBaseManager Interface

private:
	void UpdateReferencedMeshData();
	void UpdateComponentsData();
	void SetupOctahedronLayout();
	void SetupTraditionalBillboardLayout();
	void SetupPreviewMeshes();

public:
	FBoxSphereBounds GetBounds() const;

public:
	FVector2D GetRenderTargetSize() const;

public:
	UPROPERTY(Transient)
	TObjectPtr<UStaticMeshComponent> ReferencedMeshComponent;

private:
	UPROPERTY(Transient)
	TObjectPtr<UStaticMeshComponent> MeshStandComponent;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMeshComponent> ImpostorStandComponent;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> VisualizedMeshes;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UMaterialInstanceDynamic>> OverridenMeshMaterials;

public:
	UPROPERTY(VisibleAnywhere, Category = "Base")
	FVector OffsetVector = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, Category = "Base")
	float ObjectRadius = 0.f;

	UPROPERTY(VisibleAnywhere, Category = "Base")
	float DebugTexelSize = 0.f;

	UPROPERTY(VisibleAnywhere, Category = "Base")
	int32 NumHorizontalFrames = 0;

	UPROPERTY(VisibleAnywhere, Category = "Base")
	int32 NumVerticalFrames = 0;

	UPROPERTY(VisibleAnywhere, Category = "Base")
	int32 BillboardTopFrame = 0;

	UPROPERTY(VisibleAnywhere, Category = "Base")
	TArray<FVector> ViewCaptureVectors;
};
