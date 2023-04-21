// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ImpostorBaseManager.h"
#include "ImpostorComponentsManager.generated.h"

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
	void UpdateComponentsData();
	void SetupOctahedronLayout();

public:
	UPROPERTY(Transient)
	UStaticMeshComponent* ReferencedMeshComponent;

private:
	UPROPERTY(Transient)
	UStaticMeshComponent* MeshStandComponent;

	UPROPERTY(Transient)
	UStaticMeshComponent* ImpostorStandComponent;

	UPROPERTY(Transient)
	TArray<UStaticMeshComponent*> VisualizedMeshes;

public:
	UPROPERTY(VisibleAnywhere, Category = "Base")
	FVector OffsetVector = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, Category = "Base")
	float ObjectRadius = 0.f;

	UPROPERTY(VisibleAnywhere, Category = "Base")
	float DebugTexelSize = 0.f;

	UPROPERTY(VisibleAnywhere, Category = "Base")
	int32 NumFrames = 0;

	UPROPERTY(VisibleAnywhere, Category = "Base")
	TArray<FVector> ViewCaptureVectors;
};
