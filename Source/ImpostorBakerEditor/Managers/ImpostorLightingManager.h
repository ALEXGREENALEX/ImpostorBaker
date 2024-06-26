﻿#pragma once

#include <CoreMinimal.h>
#include "ImpostorBaseManager.h"
#include "ImpostorLightingManager.generated.h"

class UDirectionalLightComponent;
class USkyLightComponent;

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

	void SetLightsVisibility(bool bVisible);

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
