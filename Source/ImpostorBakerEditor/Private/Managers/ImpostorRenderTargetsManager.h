// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ImpostorData.h"
#include "SceneViewExtension.h"
#include "ImpostorBaseManager.h"
#include "ImpostorRenderTargetsManager.generated.h"

struct FLightingViewExtension final : FSceneViewExtensionBase
{
	FSceneInterface* Scene;
	FLightingViewExtension(const FAutoRegister& AutoRegister, FSceneInterface* Scene)
		: FSceneViewExtensionBase(AutoRegister)
		, Scene(Scene)
	{
	}

	virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override
	{
	}
	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override
	{
		InView.DiffuseOverrideParameter = FVector4f(GEngine->LightingOnlyBrightness.R, GEngine->LightingOnlyBrightness.G, GEngine->LightingOnlyBrightness.B, 0.0f);
		InView.SpecularOverrideParameter = FVector4f(0.f, 0.f, 0.f, 0.f);
	}

	virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override
	{
	}

protected:
	virtual bool IsActiveThisFrame_Internal(const FSceneViewExtensionContext& Context) const override
	{
		return Context.Scene == Scene;
	}
};

UCLASS()
class IMPOSTORBAKEREDITOR_API UImpostorRenderTargetsManager : public UImpostorBaseManager
{
	GENERATED_BODY()

public:
	//~ Begin UImpostorBaseManager Interface
	virtual void Initialize() override;
	virtual void Update() override;
	virtual void Tick() override;
	//~ End UImpostorBaseManager Interface

private:
	void AllocateRenderTargets();
	void CreateRenderTargetMips();
	void CreateAlphasScratchRenderTargets();
	void FillMapsToSave();

	void SceneCaptureSetup() const;

public:
	void ClearRenderTargets();
	void ClearRenderTarget(UTextureRenderTarget2D* RenderTarget) const;
	void ResampleRenderTarget(UTextureRenderTarget2D* Source, UTextureRenderTarget2D* Dest) const;

	void BakeRenderTargets();
	TMap<EImpostorBakeMapType, UTexture2D*> SaveTextures();

private:
	void PreparePostProcess(const EImpostorBakeMapType TargetMap);
	void CaptureImposterGrid();
	void DrawSingleFrame(int32 VectorIndex);
	void FinalizeBaking();

	void CustomCompositing() const;

public:
	UPROPERTY(Transient)
	TObjectPtr<USceneCaptureComponent2D> SceneCaptureComponent2D;

	UPROPERTY(VisibleAnywhere, Transient, Category = "Render Targets")
	TSet<EImpostorBakeMapType> MapsToSave;

	UPROPERTY(VisibleAnywhere, Transient, Category = "Render Targets")
	TMap<EImpostorBakeMapType, TObjectPtr<UTextureRenderTarget2D>> TargetMaps;

	UPROPERTY(VisibleAnywhere, Transient, Category = "Render Targets")
	TArray<TObjectPtr<UTextureRenderTarget2D>> SceneCaptureMipChain;

	UPROPERTY(VisibleAnywhere, Transient, Category = "Render Targets")
	TObjectPtr<UTextureRenderTarget2D> SceneCaptureSRGBMip;

	UPROPERTY(VisibleAnywhere, Transient, Category = "Render Targets")
	TArray<TObjectPtr<UTextureRenderTarget2D>> CombinedAlphas;

	UPROPERTY(VisibleAnywhere, Transient, Category = "Render Targets")
	TObjectPtr<UTextureRenderTarget2D> ScratchRenderTarget;

	UPROPERTY(VisibleAnywhere, Transient, Category = "Render Targets")
	TObjectPtr<UTextureRenderTarget2D> BaseColorScratchRenderTarget;

private:
	UPROPERTY(Transient)
	TArray<EImpostorBakeMapType> MapsToBake;

	int32 NumMapsToBake = 0;
	EImpostorBakeMapType CurrentMap = EImpostorBakeMapType::None;
	TSharedPtr<FLightingViewExtension> Extension;
	int32 FramesBeforeCapture = 0;

	bool bCapturingFinalColor = false;
};
