// Fill out your copyright notice in the Description page of Project Settings.

#include "ImpostorComponentsManager.h"
#include "ImpostorBakerUtilities.h"
#include "ImpostorLightingManager.h"

void UImpostorComponentsManager::Initialize()
{
	ReferencedMeshComponent = NewObject<UStaticMeshComponent>(GetTransientPackage());
	AddComponent(ReferencedMeshComponent);

	MeshStandComponent = NewObject<UStaticMeshComponent>(GetTransientPackage());
	AddComponent(MeshStandComponent);
	MeshStandComponent->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/EditorMeshes/EditorCube.EditorCube")));

	ImpostorStandComponent = NewObject<UStaticMeshComponent>(GetTransientPackage());
	AddComponent(ImpostorStandComponent);
	ImpostorStandComponent->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/EditorMeshes/EditorCube.EditorCube")));
}

void UImpostorComponentsManager::Update()
{
	UpdateComponentsData();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UImpostorComponentsManager::UpdateComponentsData()
{
	ReferencedMeshComponent->SetStaticMesh(ImpostorData->ReferencedMesh);
	ReferencedMeshComponent->SetRelativeLocation(FVector(ReferencedMeshComponent->Bounds.SphereRadius * -2.25f, 0.f, 10.f));

	ObjectRadius = ReferencedMeshComponent->Bounds.SphereRadius;
	OffsetVector = ReferencedMeshComponent->Bounds.Origin - ReferencedMeshComponent->GetComponentLocation();
	DebugTexelSize = ObjectRadius / 16.f;

	const FVector Scale(ObjectRadius / 256.f * 2.f, ObjectRadius / 256.f * 2.f, 0.05f);
	{
		const FVector RelativeOffset(ObjectRadius * 2.25f, 0.f, -10.f);
		ImpostorStandComponent->SetWorldScale3D(Scale);
		ImpostorStandComponent->SetRelativeLocation(RelativeOffset);
	}

	{
		const FVector RelativeOffset(ObjectRadius * -2.25f, 0.f, -10.f);
		MeshStandComponent->SetWorldScale3D(Scale);
		MeshStandComponent->SetRelativeLocation(RelativeOffset);
	}

	SetupOctahedronLayout();
}

void UImpostorComponentsManager::SetupOctahedronLayout()
{
	NumFrames = ImpostorData->ImpostorType == EImpostorLayoutType::TraditionalBillboards ? 3 : ImpostorData->FramesCount;

	ViewCaptureVectors = {};
	ViewCaptureVectors.Reserve(NumFrames * NumFrames);
	ViewCaptureVectors.SetNumUninitialized(NumFrames * NumFrames);

	for (int32 Y = 0; Y < NumFrames; Y++)
	{
		for (int32 X = 0; X < NumFrames; X++)
		{
			ViewCaptureVectors[Y * NumFrames + X] = FImpostorBakerUtilities::GetGridVector(X, Y, NumFrames, ImpostorData->ImpostorType);

			if (ImpostorData->bPreviewCaptureSphere)
			{
				const FVector& CaptureVector = ViewCaptureVectors[Y * NumFrames + X];

				FTransform Transform = FTransform::Identity;
				Transform.SetLocation(CaptureVector * (ObjectRadius * 1.1f * 1.1f) + OffsetVector + FVector(ReferencedMeshComponent->Bounds.SphereRadius * -2.25f, 0.f, 0.f));
				Transform.SetRotation(FRotationMatrix::MakeFromYZ(CaptureVector.GetSafeNormal(), FVector::UpVector).ToQuat());

				if (!VisualizedMeshes.IsValidIndex(Y * NumFrames + X))
				{
					UStaticMeshComponent* NewComponent = NewObject<UStaticMeshComponent>(GetTransientPackage());
					AddComponent(NewComponent);

					VisualizedMeshes.Add(NewComponent);
					NewComponent->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/EditorMeshes/Camera/SM_CineCam.SM_CineCam")));
				}

				UStaticMeshComponent* CaptureVisualization = VisualizedMeshes[Y * NumFrames + X];
				CaptureVisualization->SetRelativeTransform(Transform);
			}
		}
	}

	if (!ImpostorData->bPreviewCaptureSphere)
	{
		for (int32 Index = 0; Index < VisualizedMeshes.Num(); Index++)
		{
			DestroyComponent(VisualizedMeshes[Index]);
		}
		VisualizedMeshes = {};
	}
	else if (VisualizedMeshes.Num() > NumFrames * NumFrames)
	{
		for (int32 Index = VisualizedMeshes.Num() - 1; Index >= NumFrames * NumFrames; Index--)
		{
			DestroyComponent(VisualizedMeshes[Index]);
			VisualizedMeshes.RemoveAt(Index);
		}
	}

	SetOverlayText("FramesCount", "Frames Count", LexToString(NumFrames * NumFrames));
	SetOverlayText("FrameSize", "Single Frame Size", LexToString(ImpostorData->Resolution / NumFrames) + "x" + LexToString(ImpostorData->Resolution / NumFrames));
}