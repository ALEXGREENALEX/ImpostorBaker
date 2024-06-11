#pragma once

#include <CoreMinimal.h>
#include "ImpostorBaseManager.h"
#include "ImpostorMaterialsManager.generated.h"

class UMaterialInstanceConstant;
class UMaterialInstanceDynamic;

enum class EImpostorBakeMapType;

UCLASS()
class IMPOSTORBAKEREDITOR_API UImpostorMaterialsManager : public UImpostorBaseManager
{
	GENERATED_BODY()

public:
	//~ Begin UImpostorBaseManager Interface
	virtual void Initialize() override;
	virtual void Update() override;
	//~ End UImpostorBaseManager Interface

	UMaterialInstanceDynamic* GetSampleMaterial(EImpostorBakeMapType TargetMap) const;
	UMaterialInterface* GetRenderTypeMaterial(EImpostorBakeMapType TargetMap) const;
	bool HasRenderTypeMaterial(EImpostorBakeMapType TargetMap) const;
	UMaterialInstanceConstant* SaveMaterial(const TMap<EImpostorBakeMapType, UTexture2D*>& Textures) const;

	void UpdateDepthMaterialData(const FVector& ViewCaptureDirection) const;

private:
	void CreatePreviewMaterial();
	void UpdateImpostorMaterial() const;
	void UpdateSampleFrameMaterial() const;
	void UpdateAddAlphasMaterial() const;
	void UpdateBaseColorCustomLightingMaterial() const;
	void UpdateCombinedNormalsDepthMaterial() const;
	void UpdateDepthMaterial() const;
	void UpdateAddAlphaFromFinalColorMaterial() const;

public:
	UPROPERTY(VisibleAnywhere, Transient, Category = "Materials")
	TObjectPtr<UMaterialInstanceDynamic> ImpostorPreviewMaterial;

	UPROPERTY(VisibleAnywhere, Transient, Category = "Materials")
	TObjectPtr<UMaterialInstanceDynamic> BaseColorCustomLightingMaterial;

	UPROPERTY(VisibleAnywhere, Transient, Category = "Materials")
	TObjectPtr<UMaterialInstanceDynamic> CombinedNormalsDepthMaterial;

	UPROPERTY(VisibleAnywhere, Transient, Category = "Materials")
	TObjectPtr<UMaterialInstanceDynamic> SampleFrameMaterial;

	UPROPERTY(VisibleAnywhere, Transient, Category = "Materials")
	TObjectPtr<UMaterialInstanceDynamic> AddAlphasMaterial;

	UPROPERTY(VisibleAnywhere, Transient, Category = "Materials")
	TObjectPtr<UMaterialInstanceDynamic> ResampleMaterial;

	UPROPERTY(VisibleAnywhere, Transient, Category = "Materials")
	TObjectPtr<UMaterialInstanceDynamic> NormalMaterial;

	UPROPERTY(VisibleAnywhere, Transient, Category = "Materials")
	TObjectPtr<UMaterialInstanceDynamic> DepthMaterial;

	UPROPERTY(VisibleAnywhere, Transient, Category = "Materials")
	TObjectPtr<UMaterialInstanceDynamic> AddAlphaFromFinalColorMaterial;
};