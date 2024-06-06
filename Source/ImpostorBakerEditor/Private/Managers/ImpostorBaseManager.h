// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <CoreMinimal.h>
#include <Components/SceneComponent.h>
#include <FeedbackContextEditor.h>
#include <UObject/Object.h>
#include "ImpostorBakerManager.h"
#include "ImpostorBaseManager.generated.h"

#define IMPOSTOR_GET_PROPERTY_CHANGE_DELEGATE(Property, ...) ImpostorData->OnPropertyInteractiveChange.FindOrAdd(GET_MEMBER_NAME_CHECKED(UImpostorData, Property), {}) \

struct FScopedSlowTask;

// We do need to force the UI refresh more frequently than usual slow task updates
struct FImpostorFeedbackContextEditor final : FFeedbackContextEditor
{
public:
	void DoUpdate()
	{
		RequestUpdateUI(true);
	}
};

class UImpostorData;

UCLASS()
class IMPOSTORBAKEREDITOR_API UImpostorBaseManager : public UObject
{
	GENERATED_BODY()

public:
	void AssignData(
		UImpostorData* InImpostorData,
		UWorld* InSceneWorld,
		const FImpostorManageComponent& InAddComponentDelegate,
		const FImpostorManageComponent& InDestroyComponentDelegate,
		const FImpostorForceTick& InForceTickDelegate);

	virtual void Initialize()
	{
	}

	virtual void Update()
	{
	}

	virtual void Tick()
	{
		
	}

protected:
	template<typename Class>
	Class* GetManager() const
	{
		Class* Manager = Cast<Class>(GetManager(Class::StaticClass()->GetFName()));
		if (!Manager)
		{
			return nullptr;
		}

		return Manager;
	}

	UImpostorBaseManager* GetManager(FName ManagerName) const
	{
		return GetTypedOuter<UImpostorBakerManager>()->MappedManagers.FindRef(ManagerName);
	}

	void AddComponent(USceneComponent* Component) const
	{
		Component->SetFlags(RF_Transient);

		AddComponentDelegate.ExecuteIfBound(Component);
	}

	void DestroyComponent(USceneComponent* Component) const
	{
		DestroyComponentDelegate.ExecuteIfBound(Component);
	}

	void ForceTick(const bool bForce) const
	{
		ForceTickDelegate.ExecuteIfBound(bForce);
	}

	FORCEINLINE bool IsInitialized() const
	{
		return bInitialized;
	}

	void SetOverlayText(const FName Section, const FString& SectionName, const FString& Text) const
	{
		GetTypedOuter<UImpostorBakerManager>()->SetOverlayText(Section, SectionName, Text);
	}

	void SetOverlayText(const FName Section, const FString& Text, const bool bIsWarning) const
	{
		GetTypedOuter<UImpostorBakerManager>()->SetOverlayText(Section, Text, bIsWarning);
	}

	void SetOverlayText(const FName Section, const FString& Text) const
	{
		GetTypedOuter<UImpostorBakerManager>()->SetOverlayText(Section, Text);
	}

public:
	static void StartSlowTask(int32 AmountOfWork, const FString& DefaultMessage);
	static void ProgressSlowTask(const FString& Message, bool bForceUpdate);
	static void EndSlowTask();

protected:
	UPROPERTY(Transient)
	TObjectPtr<UImpostorData> ImpostorData;

	UPROPERTY(Transient)
	TObjectPtr<UWorld> SceneWorld;

private:
	FImpostorManageComponent AddComponentDelegate;
	FImpostorManageComponent DestroyComponentDelegate;
	FImpostorForceTick ForceTickDelegate;

	bool bInitialized = false;

	static TSharedPtr<FImpostorFeedbackContextEditor> FeedbackContext;
	static TSharedPtr<FScopedSlowTask> SlowTask;

	friend class UImpostorBakerManager;
};