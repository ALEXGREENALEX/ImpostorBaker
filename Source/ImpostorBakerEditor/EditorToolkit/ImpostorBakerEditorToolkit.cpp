#include "ImpostorBakerEditorToolkit.h"
#include <Modules/ModuleManager.h>
#include <PropertyEditorModule.h>
#include <Widgets/Docking/SDockTab.h>
#include "ImpostorData/ImpostorData.h"
#include "Managers/ImpostorBakerManager.h"
#include "Scene/SImpostorBakerViewport.h"
#include "Settings/ImpostorBakerSettings.h"

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

void FImpostorBakerEditorToolkit::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(ObjectBeingEdited);
}

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

	GlobalSettingsDetailsView = PropertyModule.CreateDetailView(Args);
	GlobalSettingsDetailsView->SetObject(GetMutableDefault<UImpostorBakerSettings>());
}

TSharedRef<FTabManager::FLayout> FImpostorBakerEditorToolkit::GetLayout()
{
	return FTabManager::NewLayout("FImpostorBakerEditorToolkit_Layout_V2")
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
				->AddTab(GlobalSettingsTabId, ETabState::OpenedTab)
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
	OutTabs.Add({ ViewportTabId, INVTEXT("Viewport"), "LevelEditor.Tabs.Viewports", Viewport });
	OutTabs.Add({ DetailsTabId, INVTEXT("Details"), "LevelEditor.Tabs.Details", DetailsView });
	OutTabs.Add({ TransientDetailsTabId, INVTEXT("Transient Details"), "LevelEditor.Tabs.Details", TransientDetailsView });
	OutTabs.Add({ GlobalSettingsTabId, INVTEXT("Global Settings"), "LevelEditor.Tabs.Details", GlobalSettingsDetailsView });
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
			INVTEXT("Export to Asset"),
			INVTEXT("Creates new mesh, textures and materials"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.OpenPlaceActors"));

		ToolBarBuilder.AddToolBarButton(
			FUIAction(FExecuteAction::CreateLambda([this]
			{
				if (ObjectBeingEdited &&
					Viewport &&
					Viewport->GetManager())
				{
					Viewport->GetManager()->AddLOD();
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
			INVTEXT("Export to LOD"),
			INVTEXT("Creates material and textures for impostor and adds new LOD for referenced mesh"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.LOD"));
	}
	ToolBarBuilder.EndSection();
}