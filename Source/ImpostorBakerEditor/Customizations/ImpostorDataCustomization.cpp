#include "ImpostorDataCustomization.h"
#include <DetailLayoutBuilder.h>
#include "ImpostorData/ImpostorData.h"

TSharedRef<IDetailCustomization> FImpostorDataCustomization::MakeInstance()
{
	return MakeShared<FImpostorDataCustomization>();
}

void FImpostorDataCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	const TSharedRef<IPropertyHandle> ProjectionTypeHandle = DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(UImpostorData, ProjectionType));
	const TSharedRef<IPropertyHandle> PerspectiveCameraTypeHandle = DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(UImpostorData, PerspectiveCameraType));
	const TSharedRef<IPropertyHandle> CameraDistanceHandle = DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(UImpostorData, CameraDistance));
	const TSharedRef<IPropertyHandle> CameraFOVHandle = DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(UImpostorData, CameraFOV));

	const auto VisibilityLambda = MakeAttributeLambda([ProjectionTypeHandle]
	{
		uint8 Value = 0;
		ensure(ProjectionTypeHandle->GetValue(Value) == FPropertyAccess::Success);
		return ECameraProjectionMode::Type(Value) == ECameraProjectionMode::Perspective ? EVisibility::Visible : EVisibility::Collapsed;
	});

	IDetailPropertyRow* CameraDistanceRow = DetailLayout.EditDefaultProperty(CameraDistanceHandle);
	CameraDistanceRow->Visibility(VisibilityLambda);
	CameraDistanceRow->EditCondition(MakeAttributeLambda([PerspectiveCameraTypeHandle]
	{
		uint8 Value = 0;
		ensure(PerspectiveCameraTypeHandle->GetValue(Value) == FPropertyAccess::Success);

		const EImpostorPerspectiveCameraType CameraType = EImpostorPerspectiveCameraType(Value);
		return
			CameraType == EImpostorPerspectiveCameraType::Both ||
			CameraType  == EImpostorPerspectiveCameraType::Distance;
	}), {});

	IDetailPropertyRow* CameraFOVRow = DetailLayout.EditDefaultProperty(CameraFOVHandle);
	CameraFOVRow->Visibility(VisibilityLambda);
	CameraFOVRow->EditCondition(MakeAttributeLambda([PerspectiveCameraTypeHandle]
	{
		uint8 Value = 0;
		ensure(PerspectiveCameraTypeHandle->GetValue(Value) == FPropertyAccess::Success);

		const EImpostorPerspectiveCameraType CameraType = EImpostorPerspectiveCameraType(Value);
		return
			CameraType == EImpostorPerspectiveCameraType::Both ||
			CameraType  == EImpostorPerspectiveCameraType::FOV;
	}), {});
}