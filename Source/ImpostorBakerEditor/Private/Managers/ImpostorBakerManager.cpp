// Fill out your copyright notice in the Description page of Project Settings.

#include "ImpostorBakerManager.h"
#include "ImpostorLightingManager.h"
#include "ImpostorMaterialsManager.h"
#include "ImpostorComponentsManager.h"
#include "ImpostorProceduralMeshManager.h"
#include "ImpostorRenderTargetsManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ImpostorBakerManager)

void UImpostorBakerManager::AssignData(UImpostorData* InImpostorData,
									   UWorld* InSceneWorld,
									   const FImpostorManageComponent& InAddComponentDelegate,
									   const FImpostorManageComponent& InDestroyComponentDelegate,
									   const FImpostorForceTick& InForceTickDelegate,
									   const FImpostorPopulateOverlay& InPopulateOverlay)
{
	ImpostorData = InImpostorData;
	SceneWorld = InSceneWorld;
	AddComponentDelegate = InAddComponentDelegate;
	DestroyComponentDelegate = InDestroyComponentDelegate;
	ForceTickDelegate = InForceTickDelegate;
	PopulateOverlay = InPopulateOverlay;

	ImpostorData->OnSettingsChange.BindUObject(this, &UImpostorBakerManager::FullUpdate);
}

void UImpostorBakerManager::AssignSkyLight(USkyLightComponent* InSkyLight)
{
	SkyLight = InSkyLight;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UImpostorBakerManager::Initialize()
{
	AddManager<UImpostorComponentsManager>();
	AddManager<UImpostorLightingManager>();
	AddManager<UImpostorRenderTargetsManager>();
	AddManager<UImpostorMaterialsManager>();
	AddManager<UImpostorProceduralMeshManager>();

	FullUpdate();
}

void UImpostorBakerManager::FullUpdate()
{
	for (UImpostorBaseManager* Manager : Managers)
	{
		Manager->Update();
	}

	bNeedsCapture = true;
	SetOverlayText("NeedsRebake", "Capturing is required, for impostor preview to appear or changes to apply", true);
}

void UImpostorBakerManager::Bake()
{
	GetManager<UImpostorRenderTargetsManager>()->BakeRenderTargets();

	bNeedsCapture = false;
	SetOverlayText("NeedsRebake", "");
}

void UImpostorBakerManager::ClearRenderTargets() const
{
	GetManager<UImpostorRenderTargetsManager>()->ClearRenderTargets();
}

void UImpostorBakerManager::CreateAssets() const
{
	UImpostorBaseManager::StartSlowTask(GetManager<UImpostorRenderTargetsManager>()->MapsToSave.Num() + 2, "Creating impostor mesh assets...");
	const TMap<EImpostorBakeMapType, UTexture2D*> NewTextures = GetManager<UImpostorRenderTargetsManager>()->SaveTextures();
	if (UMaterialInstanceConstant* NewMaterial = GetManager<UImpostorMaterialsManager>()->SaveMaterial(NewTextures))
	{
		GetManager<UImpostorProceduralMeshManager>()->SaveMesh(NewMaterial);
	}
	UImpostorBaseManager::EndSlowTask();
}

void UImpostorBakerManager::AddLOD()
{
	UImpostorBaseManager::StartSlowTask(GetManager<UImpostorRenderTargetsManager>()->MapsToSave.Num() + 2, "Adding impostor LOD to referenced mesh...");
	const TMap<EImpostorBakeMapType, UTexture2D*> NewTextures = GetManager<UImpostorRenderTargetsManager>()->SaveTextures();
	if (UMaterialInstanceConstant* NewMaterial = GetManager<UImpostorMaterialsManager>()->SaveMaterial(NewTextures))
	{
		GetManager<UImpostorProceduralMeshManager>()->UpdateLOD(NewMaterial);
	}
	UImpostorBaseManager::EndSlowTask();
}

void UImpostorBakerManager::Cleanup()
{
	for (UImpostorBaseManager* Manager : Managers)
	{
		Manager->ImpostorData = nullptr;
		Manager->SceneWorld = nullptr;
	}

	ImpostorData = nullptr;
	SceneWorld = nullptr;
	SkyLight = nullptr;
	Managers = {};
	MappedManagers = {};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UImpostorBakerManager::Tick()
{
	for (UImpostorBaseManager* Manager : Managers)
	{
		Manager->Tick();
	}
}

void UImpostorBakerManager::SetOverlayText(const FName Section, const FString& SectionName, const FString& Text)
{
	ON_SCOPE_EXIT
	{
		PopulateOverlay.ExecuteIfBound(TextItems);
	};

	if (Text.IsEmpty())
	{
		TextItems.Remove(Section);
		return;
	}

	TextItems.Add(Section, "<TextBlock.ShadowedTextWarning>" + SectionName + "</>: <TextBlock.ShadowedText>" + Text + "</>");
}

void UImpostorBakerManager::SetOverlayText(const FName Section, const FString& Text, const bool bIsWarning)
{
	ON_SCOPE_EXIT
	{
		PopulateOverlay.ExecuteIfBound(TextItems);
	};

	if (Text.IsEmpty())
	{
		TextItems.Remove(Section);
		return;
	}

	TextItems.Add(Section, bIsWarning ? ("<TextBlock.ShadowedTextWarning>" + Text + "</>") : ("<TextBlock.ShadowedText>" + Text + "</>"));
}

void UImpostorBakerManager::SetOverlayText(const FName Section, const FString& Text)
{
	ON_SCOPE_EXIT
	{
		PopulateOverlay.ExecuteIfBound(TextItems);
	};

	if (Text.IsEmpty())
	{
		TextItems.Remove(Section);
		return;
	}

	TextItems.Add(Section, Text);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UImpostorBakerManager::AddManager(const TSubclassOf<UImpostorBaseManager>& ManagerClass)
{
	UImpostorBaseManager* NewManager = NewObject<UImpostorBaseManager>(this, ManagerClass, NAME_None, RF_Transient);
	NewManager->AssignData(ImpostorData, SceneWorld, AddComponentDelegate, DestroyComponentDelegate, ForceTickDelegate);
	NewManager->Initialize();
	NewManager->bInitialized = true;

	Managers.Add(NewManager);
	MappedManagers.Add(NewManager->GetClass()->GetFName(), NewManager);
}