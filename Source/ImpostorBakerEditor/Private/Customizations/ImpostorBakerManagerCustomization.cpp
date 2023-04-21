#include "ImpostorBakerManagerCustomization.h"
#include "DetailLayoutBuilder.h"
#include "Managers/ImpostorBakerManager.h"

TSharedRef<IDetailCustomization> FImpostorBakerManagerCustomization::MakeInstance()
{
	return MakeShared<FImpostorBakerManagerCustomization>();
}

void FImpostorBakerManagerCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	const TSharedRef<IPropertyHandle> ManagersHandle = DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(UImpostorBakerManager, Managers));
	DetailLayout.HideProperty(ManagersHandle);

	uint32 NumManagers = 0;
	ensure(ManagersHandle->GetNumChildren(NumManagers) == FPropertyAccess::Success);

	for (uint32 Index = 0; Index < NumManagers; Index++)
	{
		const TSharedPtr<IPropertyHandle> ChildHandle = ManagersHandle->GetChildHandle(Index);
		if (!ensure(ChildHandle))
		{
			continue;
		}

		uint32 NumRootProperties = 0;
		ensure(ChildHandle->GetNumChildren(NumRootProperties) == FPropertyAccess::Success);

		for (uint32 RootPropertyIndex = 0; RootPropertyIndex < NumRootProperties; RootPropertyIndex++)
		{
			const TSharedPtr<IPropertyHandle> RootPropertyHandle = ChildHandle->GetChildHandle(RootPropertyIndex);
			if (!ensure(RootPropertyHandle))
			{
				continue;
			}

			uint32 NumCategories = 0;
			ensure(RootPropertyHandle->GetNumChildren(NumCategories) == FPropertyAccess::Success);

			for (uint32 CategoryIndex = 0; CategoryIndex < NumCategories; CategoryIndex++)
			{
				const TSharedPtr<IPropertyHandle> CategoryHandle = RootPropertyHandle->GetChildHandle(CategoryIndex);
				if (!ensure(CategoryHandle))
				{
					continue;
				}

				uint32 NumProperties = 0;
				ensure(CategoryHandle->GetNumChildren(NumProperties) == FPropertyAccess::Success);

				for (uint32 PropertyIndex = 0; PropertyIndex < NumProperties; PropertyIndex++)
				{
					const TSharedPtr<IPropertyHandle> PropertyHandle = CategoryHandle->GetChildHandle(PropertyIndex);
					if (!ensure(PropertyHandle))
					{
						continue;
					}

					DetailLayout.AddPropertyToCategory(PropertyHandle);
				}
			}
		}
	}
}