// Fill out your copyright notice in the Description page of Project Settings.

#include "ImpostorBaseManager.h"
#include "Misc/ScopedSlowTask.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ImpostorBaseManager)

TSharedPtr<FImpostorFeedbackContextEditor> UImpostorBaseManager::FeedbackContext = nullptr;
TSharedPtr<FScopedSlowTask> UImpostorBaseManager::SlowTask = nullptr;

void UImpostorBaseManager::AssignData(UImpostorData* InImpostorData, UWorld* InSceneWorld, const FImpostorManageComponent& InAddComponentDelegate, const FImpostorManageComponent& InDestroyComponentDelegate, const FImpostorForceTick& InForceTickDelegate)
{
	SceneWorld = InSceneWorld;
	ImpostorData = InImpostorData;
	AddComponentDelegate = InAddComponentDelegate;
	DestroyComponentDelegate = InDestroyComponentDelegate;
	ForceTickDelegate = InForceTickDelegate;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UImpostorBaseManager::StartSlowTask(int32 AmountOfWork, const FString& DefaultMessage)
{
	FeedbackContext = MakeShared<FImpostorFeedbackContextEditor>();
	SlowTask = MakeShared<FScopedSlowTask>(AmountOfWork, FText::FromString(DefaultMessage), true, *FeedbackContext.Get());
	SlowTask->MakeDialog(false);
}

void UImpostorBaseManager::ProgressSlowTask(const FString& Message, const bool bForceUpdate)
{
	if (!ensure(SlowTask))
	{
		return;
	}

	SlowTask->EnterProgressFrame(1.f, FText::FromString(Message));

	if (bForceUpdate)
	{
		FeedbackContext->DoUpdate();
	}
}

void UImpostorBaseManager::EndSlowTask()
{
	SlowTask = nullptr;
	FeedbackContext = nullptr;
}