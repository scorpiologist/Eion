// Copyright 2018 Phyronnaz

#include "SGraphPinVoxelType.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Views/SListView.h"
#include "ScopedTransaction.h"

void SGraphPinVoxelType::Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj)
{
	SGraphPin::Construct(SGraphPin::FArguments(), InGraphPinObj);
}

TSharedRef<SWidget>	SGraphPinVoxelType::GetDefaultValueWidget()
{
	//Get list of enum indexes
	TArray< TSharedPtr<int32> > ComboItems;
	GenerateComboBoxIndexes( ComboItems );

	//Create widget
	return SAssignNew(ComboBox, SPinComboBox)
		.ComboItemList( ComboItems )
		.VisibleText( this, &SGraphPinVoxelType::OnGetText )
		.OnSelectionChanged( this, &SGraphPinVoxelType::ComboBoxSelectionChanged )
		.Visibility( this, &SGraphPin::GetDefaultValueVisibility )
		.OnGetDisplayName(this, &SGraphPinVoxelType::OnGetFriendlyName)
		.OnGetTooltip(this, &SGraphPinVoxelType::OnGetTooltip);
}

FText SGraphPinVoxelType::OnGetFriendlyName(int32 EnumIndex)
{
	UEnum* EnumPtr = Cast<UEnum>(GraphPinObj->PinType.PinSubCategoryObject.Get());

	check(EnumPtr);
	check(EnumIndex < EnumPtr->NumEnums());

	FText EnumValueName = EnumPtr->GetDisplayNameTextByIndex(EnumIndex);
	return EnumValueName;
}

FText SGraphPinVoxelType::OnGetTooltip(int32 EnumIndex)
{
	UEnum* EnumPtr = Cast<UEnum>(GraphPinObj->PinType.PinSubCategoryObject.Get());

	check(EnumPtr);
	check(EnumIndex < EnumPtr->NumEnums());

	FText EnumValueTooltip = EnumPtr->GetToolTipTextByIndex(EnumIndex);
	return EnumValueTooltip;
}

void SGraphPinVoxelType::ComboBoxSelectionChanged( TSharedPtr<int32> NewSelection, ESelectInfo::Type /*SelectInfo*/ )
{
	UEnum* EnumPtr = Cast<UEnum>(GraphPinObj->PinType.PinSubCategoryObject.Get());
	check(EnumPtr);

	FString EnumSelectionString;
	if (NewSelection.IsValid())
	{
		check(*NewSelection < EnumPtr->NumEnums() - 1);
		EnumSelectionString  = EnumPtr->GetNameStringByIndex(*NewSelection);
	}
	else
	{
		EnumSelectionString = FName(NAME_None).ToString();
	}
	

	if(GraphPinObj->GetDefaultAsString() != EnumSelectionString)
	{
		const FScopedTransaction Transaction( NSLOCTEXT("GraphEditor", "ChangeEnumPinValue", "Change Enum Pin Value" ) );
		GraphPinObj->Modify();

		//Set new selection
		GraphPinObj->GetSchema()->TrySetDefaultValue(*GraphPinObj, EnumSelectionString);
	}
}

FString SGraphPinVoxelType::OnGetText() const 
{
	FString SelectedString = GraphPinObj->GetDefaultAsString();

	UEnum* EnumPtr = Cast<UEnum>(GraphPinObj->PinType.PinSubCategoryObject.Get());
	if (EnumPtr && EnumPtr->NumEnums())
	{
		const int32 MaxIndex = EnumPtr->NumEnums() - 1;
		for (int32 EnumIdx = 0; EnumIdx < MaxIndex; ++EnumIdx)
		{
			// Ignore hidden enum values
			if( !EnumPtr->HasMetaData(TEXT("Hidden"), EnumIdx ) )
			{
				if (SelectedString == EnumPtr->GetNameStringByIndex(EnumIdx))
				{
					FString EnumDisplayName = EnumPtr->GetDisplayNameTextByIndex(EnumIdx).ToString();
					if (EnumDisplayName.Len() == 0)
					{
						return SelectedString;
					}
					else
					{
						return EnumDisplayName;
					}
				}
			}
		}

		if (SelectedString == EnumPtr->GetNameStringByIndex(MaxIndex))
		{
			return TEXT("(INVALID)");
		}

	}
	return SelectedString;
}

void SGraphPinVoxelType::GenerateComboBoxIndexes( TArray< TSharedPtr<int32> >& OutComboBoxIndexes )
{
	UEnum* EnumPtr = Cast<UEnum>( GraphPinObj->PinType.PinSubCategoryObject.Get() );
	if (EnumPtr)
	{

		//NumEnums() - 1, because the last item in an enum is the _MAX item
		for (int32 EnumIndex = 0; EnumIndex < EnumPtr->NumEnums() - 1; ++EnumIndex)
		{
			// Ignore hidden enum values
			if( !EnumPtr->HasMetaData(TEXT("Hidden"), EnumIndex ) )
			{
				TSharedPtr<int32> EnumIdxPtr(new int32(EnumIndex));
				OutComboBoxIndexes.Add(EnumIdxPtr);
			}
		}
	}
}
