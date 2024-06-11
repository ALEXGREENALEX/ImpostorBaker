#include "ImpostorLightingManager.h"
#include <Components/DirectionalLightComponent.h>
#include <Components/SkyLightComponent.h>
#include <Engine/World.h>
#include "ImpostorComponentsManager.h"
#include "ImpostorData/ImpostorData.h"
#include "Utilities/ImpostorBakerUtilities.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ImpostorLightingManager)

void UImpostorLightingManager::Initialize()
{
	if (GetTypedOuter<UImpostorBakerManager>()->SkyLight)
	{
		SkyLightComponent = GetTypedOuter<UImpostorBakerManager>()->SkyLight;
	}
	else
	{
		SkyLightComponent = NewObject<USkyLightComponent>(SceneWorld);
		AddComponent(SkyLightComponent);
	}
}

void UImpostorLightingManager::Update()
{
	SetupLightVectors();
}

void UImpostorLightingManager::SetupLightVectors()
{
	LightVectors.Empty();
	LightVectors.SetNumUninitialized(ImpostorData->LightingGridSize * ImpostorData->LightingGridSize);

	for (int32 Y = 0; Y < ImpostorData->LightingGridSize; Y++)
	{
		for (int32 X = 0; X < ImpostorData->LightingGridSize; X++)
		{
			LightVectors[Y * ImpostorData->LightingGridSize + X] = FMath::Lerp(FImpostorBakerUtilities::GetGridVector(X, Y, ImpostorData->LightingGridSize, ImpostorData->ImpostorType), FVector::UpVector, ImpostorData->UpwardBias);
		}
	}

	DisplayCustomLighting();
}

void UImpostorLightingManager::DisplayCustomLighting()
{
	int32 Index = 0;
	for (const FVector& LightVector : LightVectors)
	{
		FTransform Transform;
		Transform.SetLocation(LightVector * (GetManager<UImpostorComponentsManager>()->ObjectRadius * 2.5f) + GetManager<UImpostorComponentsManager>()->OffsetVector);
		Transform.SetRotation((LightVector * -1.f).ToOrientationQuat());

		UDirectionalLightComponent* Light = nullptr;
		if (Lights.IsValidIndex(Index))
		{
			if (UDirectionalLightComponent* ExistingLight = Lights[Index])
			{
				Light = ExistingLight;
			}
			else
			{
				Light = NewObject<UDirectionalLightComponent>(SceneWorld);
				AddComponent(Light);
				Lights[Index] = Light;
			}
		}
		else
		{
			Light = NewObject<UDirectionalLightComponent>(SceneWorld);
			AddComponent(Light);
			Lights.Add(Light);
		}

		Light->SetRelativeTransform(Transform);
		Light->SetIntensity(ImpostorData->DirectionalLightBrightness / (ImpostorData->LightingGridSize * ImpostorData->LightingGridSize));

		Index++;
	}

	for (Index = Lights.Num() - 1; Index >= LightVectors.Num(); Index--)
	{
		DestroyComponent(Lights[Index]);
		Lights.RemoveAt(Index);
	}

	SkyLightComponent->SetIntensity(ImpostorData->CustomSkyLightIntensity);
	SkyLightComponent->RecaptureSky();
}