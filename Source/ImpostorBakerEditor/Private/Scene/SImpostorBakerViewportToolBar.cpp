// Fill out your copyright notice in the Description page of Project Settings.

#include "SImpostorBakerViewportToolBar.h"
#include <BufferVisualizationMenuCommands.h>
#include <SEditorViewport.h>

void SImpostorBakerViewportToolBar::Construct(const FArguments& InArgs, const TSharedPtr<ICommonEditorViewportToolbarInfoProvider>& InViewport)
{
	SCommonEditorViewportToolbarBase::Construct({}, InViewport);
}

TSharedPtr<FExtender> SImpostorBakerViewportToolBar::GetViewMenuExtender() const
{
	const TSharedRef<FExtender> ViewModeExtender = MakeShared<FExtender>();

	ViewModeExtender->AddMenuExtension(
		TEXT("ViewMode"),
		EExtensionHook::After,
		GetInfoProvider().GetViewportWidget()->GetCommandList(),
		FMenuExtensionDelegate::CreateSP(const_cast<SImpostorBakerViewportToolBar*>(this), &SImpostorBakerViewportToolBar::CreateViewMenuExtensions));

	ViewModeExtender->AddMenuExtension(
		TEXT("ViewMode"),
		EExtensionHook::After,
		GetInfoProvider().GetViewportWidget()->GetCommandList(),
		FMenuExtensionDelegate::CreateSP(this, &SImpostorBakerViewportToolBar::CreateBufferVisualizationExtensions));

	return GetCombinedExtenderList(ViewModeExtender);
}

void SImpostorBakerViewportToolBar::CreateBufferVisualizationExtensions(FMenuBuilder& MenuBuilder) const
{
	MenuBuilder.AddSubMenu(
		INVTEXT("Buffer Visualization"),
		INVTEXT("Select a mode for buffer visualization"),
		FNewMenuDelegate::CreateStatic(&FBufferVisualizationMenuCommands::BuildVisualisationSubMenu),
		FUIAction(
			FExecuteAction(),
			FCanExecuteAction(),
			FIsActionChecked::CreateLambda([this]()
			{
				const TSharedPtr<SEditorViewport> ViewportPtr = GetInfoProvider().GetViewportWidget();
				if (ViewportPtr.IsValid())
				{
					const TSharedPtr<FEditorViewportClient> ViewportClient = ViewportPtr->GetViewportClient();
					return ViewportClient->IsViewModeEnabled(VMI_VisualizeBuffer);
				}
				return false;
			})
		),
		"VisualizeBufferViewMode",
		EUserInterfaceActionType::RadioButton,
		false,
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "EditorViewport.VisualizeBufferMode")
		);
}