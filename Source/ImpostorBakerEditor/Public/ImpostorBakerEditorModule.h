#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FAssetTypeActions_ImpostorSettings;

class FImpostorBakerEditorModule : public IModuleInterface
{
public:
	//~ Begin IModuleInterface Interface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	//~ End IModuleInterface Interface

private:
	FDelegateHandle ContentBrowserAssetsExtenderDelegateHandle;
	TSharedPtr<FAssetTypeActions_ImpostorSettings> AssetTypeAction;
};
