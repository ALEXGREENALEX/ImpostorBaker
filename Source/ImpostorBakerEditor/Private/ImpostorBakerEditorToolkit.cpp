// Fill out your copyright notice in the Description page of Project Settings.

#include "ImpostorBakerEditorToolkit.h"
#include "ImpostorData.h"
#include "Scene/SImpostorBakerViewport.h"
#include "Managers/ImpostorBakerManager.h"

void FImpostorBakerEditorToolkit::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(GetBaseToolkitName());

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	TArray<FTab> RegisteredTabs;
	RegisterTabs(RegisteredTabs);

	for (const FTab& Tab : RegisteredTabs)
	{
		InTabManager->RegisterTabSpawner(Tab.TabId, FOnSpawnTab::CreateLambda([DisplayName = Tab.DisplayName, Widget = TWeakPtr<SWidget>(Tab.Widget)](const FSpawnTabArgs& Args) -> TSharedRef<SDockTab>
		{
			return SNew(SDockTab)
			.Label(DisplayName)
			[
				Widget.IsValid() ? Widget.Pin().ToSharedRef() : SNullWidget::NullWidget
			];
		}))
		.SetDisplayName(Tab.DisplayName)
		.SetGroup(WorkspaceMenuCategory.ToSharedRef())
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), Tab.Icon));

		RegisteredTabIds.Add(Tab.TabId);
	}
}

void FImpostorBakerEditorToolkit::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	for (const FName& TabId : RegisteredTabIds)
	{
		InTabManager->UnregisterTabSpawner(TabId);
	}
}

FText FImpostorBakerEditorToolkit::GetBaseToolkitName() const
{
	return FText::FromString(FName::NameToDisplayString(ToolkitName, false));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FImpostorBakerEditorToolkit::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(ObjectBeingEdited);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FImpostorBakerEditorToolkit::InitImpostorAssetEditor(EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UObject* ObjectToEdit)
{
	ObjectBeingEdited = Cast<UImpostorData>(ObjectToEdit);
	ObjectBeingEdited->SetFlags(RF_Transactional);

	GEditor->RegisterForUndo(this);

	CreateInternalWidgets();

	InitAssetEditor(Mode, InitToolkitHost, *ToolkitName, GetLayout(), true, true, ObjectToEdit, false);

	{
		const TSharedRef<FExtender> ToolbarExtender = MakeShared<FExtender>();

		ToolbarExtender->AddToolBarExtension(
			"Asset",
			EExtensionHook::After,
			GetToolkitCommands(),
			FToolBarExtensionDelegate::CreateSP(this, &FImpostorBakerEditorToolkit::BuildToolbar)
		);

		AddToolbarExtender(ToolbarExtender);
	}

	RegenerateMenusAndToolbars();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FImpostorBakerEditorToolkit::CreateInternalWidgets()
{
	FDetailsViewArgs Args;
	Args.bHideSelectionTip = true;
	Args.NotifyHook = this;

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	DetailsView = PropertyModule.CreateDetailView(Args);
	DetailsView->SetObject(ObjectBeingEdited);

	TransientDetailsView = PropertyModule.CreateDetailView(Args);

	Viewport = SNew(SImpostorBakerViewport)
	.Object(ObjectBeingEdited);

	TransientDetailsView->SetObject(Viewport->GetManager());
}

TSharedRef<FTabManager::FLayout> FImpostorBakerEditorToolkit::GetLayout()
{
	return FTabManager::NewLayout("FImpostorBakerEditorToolkit_Layout_V1")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Horizontal)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.3f)
				->AddTab(DetailsTabId, ETabState::OpenedTab)
				->AddTab(TransientDetailsTabId, ETabState::OpenedTab)
				->SetForegroundTab(FName(DetailsTabId))
			)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.7f)
				->AddTab(ViewportTabId, ETabState::OpenedTab)
			)
		);
}

void FImpostorBakerEditorToolkit::RegisterTabs(TArray<FTab>& OutTabs) const
{
	OutTabs.Add({ViewportTabId, INVTEXT("Viewport"), "LevelEditor.Tabs.Viewports", Viewport});
	OutTabs.Add({DetailsTabId, INVTEXT("Details"), "LevelEditor.Tabs.Details", DetailsView});
	OutTabs.Add({TransientDetailsTabId, INVTEXT("Transient Details"), "LevelEditor.Tabs.Details", TransientDetailsView});
}

void FImpostorBakerEditorToolkit::BuildToolbar(FToolBarBuilder& ToolBarBuilder) const
{
	ToolBarBuilder.BeginSection("Impostor");
	{
		ToolBarBuilder.AddToolBarButton(
			FUIAction(FExecuteAction::CreateLambda([this]
			{
				if (ObjectBeingEdited &&
					Viewport &&
					Viewport->GetManager())
				{
					Viewport->GetManager()->ClearRenderTargets();
				}
			})),
			NAME_None,
			INVTEXT("Clear Render Targets"),
			INVTEXT("Clears Render Targets and display default material"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "ShowFlagsMenu.Decals"));

		ToolBarBuilder.AddToolBarButton(
			FUIAction(FExecuteAction::CreateLambda([this]
			{
				if (ObjectBeingEdited &&
					Viewport &&
					Viewport->GetManager())
				{
					Viewport->GetManager()->Bake();
				}
			})),
			NAME_None,
			INVTEXT("Capture"),
			INVTEXT("Captures mesh to render targets"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.SceneCaptureComponent2D"));

		ToolBarBuilder.AddToolBarButton(
			FUIAction(FExecuteAction::CreateLambda([this]
			{
				if (ObjectBeingEdited &&
					Viewport &&
					Viewport->GetManager())
				{
					Viewport->GetManager()->CreateAssets();
				}
			}),
			FCanExecuteAction::CreateLambda([this]
			{
				if (!ObjectBeingEdited ||
					!Viewport ||
					!Viewport->GetManager())
				{
					return false;
				}

				return !Viewport->GetManager()->NeedsCapture();
			})),
			NAME_None,
			INVTEXT("Create Assets"),
			INVTEXT("Creates impostor texture and materials assets"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.OpenPlaceActors"));
	}
	ToolBarBuilder.EndSection();
}