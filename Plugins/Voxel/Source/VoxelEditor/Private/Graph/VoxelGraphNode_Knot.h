// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "VoxelGraphNode_Base.h"
#include "VoxelGraphNode_Knot.generated.h"

#define WILDCARD TEXT("PC_Wildcard")

class FBlueprintActionDatabaseRegistrar;
class INameValidatorInterface;
class UEdGraph;
class UEdGraphPin;

UCLASS(MinimalAPI)
class UVoxelGraphNode_Knot : public UVoxelGraphNode_Base
{
	GENERATED_BODY()

public:
	// UEdGraphNode interface
	virtual void AllocateDefaultPins() override;
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual bool ShouldOverridePinNames() const override;
	virtual FText GetPinNameOverride(const UEdGraphPin& Pin) const override;
	virtual void OnRenameNode(const FString& NewName) override;
	virtual bool CanSplitPin(const UEdGraphPin* Pin) const override;
	virtual bool IsCompilerRelevant() const override { return false; }
	virtual UEdGraphPin* GetPassThroughPin(const UEdGraphPin* FromPin) const override;	
	virtual bool ShouldDrawNodeAsControlPointOnly(int32& OutInputPinIndex, int32& OutOutputPinIndex) const override { OutInputPinIndex = 0;  OutOutputPinIndex = 1; return true; }
	// End of UEdGraphNode interface

	virtual bool IsCompact() const override { return true; }

	UEdGraphPin* GetInputPin() const
	{
		return Pins[0];
	}

	UEdGraphPin* GetOutputPin() const
	{
		return Pins[1];
	}

	void PropagatePinType();

	UEdGraphPin* GetRealInputPin();
	UEdGraphPin* GetRealOutputPin();

private:
	void PropagatePinTypeFromInput();
	void PropagatePinTypeFromOutput();

	/** Recursion guard boolean to prevent PropagatePinType from infinitely recursing if you manage to create a loop of knots */
	bool bRecursionGuard;
};
