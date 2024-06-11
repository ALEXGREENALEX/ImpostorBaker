#pragma once

#include <CoreMinimal.h>
#include <SCommonEditorViewportToolbarBase.h>

class SImpostorBakerViewport;

class SImpostorBakerViewportToolBar : public SCommonEditorViewportToolbarBase
{
public:
	SLATE_BEGIN_ARGS(SImpostorBakerViewportToolBar)
		{
		}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedPtr<ICommonEditorViewportToolbarInfoProvider>& InViewport);

	virtual TSharedPtr<FExtender> GetViewMenuExtender() const override;
	void CreateBufferVisualizationExtensions(FMenuBuilder& MenuBuilder) const;
};