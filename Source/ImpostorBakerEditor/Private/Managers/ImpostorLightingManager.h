// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ImpostorBaseManager.h"
#include "ImpostorLightingManager.generated.h"

UCLASS()
class IMPOSTORBAKEREDITOR_API UImpostorLightingManager : public UImpostorBaseManager
{
	GENERATED_BODY()

public:
	//~ Begin UImpostorBaseManager Interface
	virtual void Initialize() override;
	virtual void Update() override;
	//~ End UImpostorBaseManager Interface

	void SetupLightVectors();

private:
	void DisplayCustomLighting();

private:
	UPROPERTY(EditAnywhere, Transient, Category = "Lighting")
	TObjectPtr<USkyLightComponent> SkyLightComponent;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UDirectionalLightComponent>> Lights;

public:
	UPROPERTY(VisibleAnywhere, Transient, Category = "Lighting")
	TArray<FVector> LightVectors;
};
