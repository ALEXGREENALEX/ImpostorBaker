// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <UObject/Object.h>
#include "ImpostorBakerManager.generated.h"

class UImpostorBaseManager;
class UImpostorData;
class USkyLightComponent;

DECLARE_DELEGATE_OneParam(FImpostorManageComponent, USceneComponent*);
DECLARE_DELEGATE_OneParam(FImpostorForceTick, bool);
typedef TDelegate<void(const TMap<FName, FString>&)> FImpostorPopulateOverlay;

UCLASS()
class IMPOSTORBAKEREDITOR_API UImpostorBakerManager : public UObject
{
	GENERATED_BODY()

public:
	void AssignData(UImpostorData* InImpostorData,
		UWorld* InSceneWorld,
		const FImpostorManageComponent& InAddComponentDelegate,
		const FImpostorManageComponent& InDestroyComponentDelegate,
		const FImpostorForceTick& InForceTickDelegate,
		const FImpostorPopulateOverlay& InPopulateOverlay);
	void AssignSkyLight(USkyLightComponent* InSkyLight);

public:
	void Initialize();
	void FullUpdate();
	void Bake();
	void ClearRenderTargets() const;
	void CreateAssets() const;
	void AddLOD();
	void Cleanup();

public:
	void Tick();

	void SetOverlayText(FName Section, const FString& SectionName, const FString& Text);
	void SetOverlayText(FName Section, const FString& Text, bool bIsWarning);
	void SetOverlayText(FName Section, const FString& Text);

	bool NeedsCapture() const
	{
		return bNeedsCapture;
	}

private:
	template<typename ManagerClass>
	void AddManager()
	{
		AddManager(ManagerClass::StaticClass());
	}

public:
	template<typename Class>
	Class* GetManager() const
	{
		return Cast<Class>(MappedManagers.FindRef(Class::StaticClass()->GetFName()));
	}

private:
	void AddManager(const TSubclassOf<UImpostorBaseManager>& ManagerClass);

public:
	UPROPERTY(Transient)
	TObjectPtr<UImpostorData> ImpostorData;

	UPROPERTY(Transient)
	TObjectPtr<UWorld> SceneWorld;

	UPROPERTY(Transient)
	TObjectPtr<USkyLightComponent> SkyLight;

	UPROPERTY(Category = "Config", VisibleAnywhere, Transient, Instanced)
	TArray<TObjectPtr<UImpostorBaseManager>> Managers;

private:
	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UImpostorBaseManager>> MappedManagers;

	TMap<FName, FString> TextItems;

	FImpostorManageComponent AddComponentDelegate;
	FImpostorManageComponent DestroyComponentDelegate;
	FImpostorForceTick ForceTickDelegate;
	FImpostorPopulateOverlay PopulateOverlay;

	bool bNeedsCapture = true;

	friend class UImpostorBaseManager;
};
