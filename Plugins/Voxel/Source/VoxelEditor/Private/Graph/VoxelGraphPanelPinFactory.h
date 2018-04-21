// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "EdGraphUtilities.h"
#include "EdGraphSchema_K2.h"
#include "SGraphPin.h"
#include "KismetPins/SGraphPinBool.h"
#include "KismetPins/SGraphPinExec.h"
#include "KismetPins/SGraphPinInteger.h"
#include "KismetPins/SGraphPinNum.h"
#include "SGraphPinVoxelType.h"
#include "VoxelNode.h"
#include "Runtime/Launch/Resources/Version.h"

class FVoxelGraphPanelPinFactory : public FGraphPanelPinFactory
{
	virtual TSharedPtr<class SGraphPin> CreatePin(class UEdGraphPin* InPin) const override
	{
#if ENGINE_MINOR_VERSION < 19
		FString Category = InPin->PinType.PinCategory;
#else
		FString Category = InPin->PinType.PinCategory.ToString();
#endif

		if (Category == FVoxelPinCategory::PC_Boolean)
		{
			return SNew(SGraphPinBool, InPin);
		}
		else if (Category == FVoxelPinCategory::PC_Exec)
		{
			return SNew(SGraphPinExec, InPin);
		}
		else if (Category == FVoxelPinCategory::PC_Int)
		{
			return SNew(SGraphPinInteger, InPin);
		}
		else if (Category == FVoxelPinCategory::PC_Float)
		{
#if ENGINE_MINOR_VERSION < 19
			return SNew(SGraphPinNum, InPin);
#else
			return SNew(SGraphPinNum<float>, InPin);
#endif
		}
		else if (Category == FVoxelPinCategory::PC_Material)
		{
			return NULL;
		}
		else if (Category == FVoxelPinCategory::PC_ValueType)
		{
			InPin->PinType.PinSubCategoryObject = FindObject<UEnum>(ANY_PACKAGE, TEXT("EVoxelValueType"), true);
			return SNew(SGraphPinVoxelType, InPin);
		}
		else if (Category == FVoxelPinCategory::PC_MaterialType)
		{
			InPin->PinType.PinSubCategoryObject = FindObject<UEnum>(ANY_PACKAGE, TEXT("EVoxelMaterialType"), true);
			return SNew(SGraphPinVoxelType, InPin);
		}
		else if (Category == FVoxelPinCategory::PC_VoxelType)
		{
			return NULL;
		}
		else
		{
			return NULL;
		}
	}
};
