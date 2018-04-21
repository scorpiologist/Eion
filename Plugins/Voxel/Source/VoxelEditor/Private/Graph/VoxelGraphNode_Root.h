// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "EdGraph/EdGraphNode.h"
#include "VoxelGraphNode_Base.h"
#include "VoxelNode.h"
#include "VoxelGraphNode_Root.generated.h"

UCLASS(MinimalAPI)
class UVoxelGraphNode_Root : public UVoxelGraphNode_Base
{
	GENERATED_BODY()

public:
	// UEdGraphNode interface
	virtual bool CanUserDeleteNode() const override { return false; };
	virtual bool CanDuplicateNode() const override { return false; }
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override { return FText::FromString(TEXT("Start")); };
	virtual FLinearColor GetNodeTitleColor() const override { return FLinearColor::Red; };
	// End of UEdGraphNode interface

	// USoundCueGraphNode_Base interface
	virtual void CreateOutputPins() override
	{
		UEdGraphPin* NewPin = CreatePin(EGPD_Output, FVoxelPinCategory::PC_Exec, FString(), nullptr, TEXT(" "));
	}
	// End of USoundCueGraphNode_Base interface
};
