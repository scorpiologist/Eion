// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "EdGraph/EdGraphNode.h"
#include "VoxelGraphNode_Base.generated.h"

class UEdGraphPin;
class UEdGraphSchema;

UCLASS(MinimalAPI)
class UVoxelGraphNode_Base : public UEdGraphNode
{
	GENERATED_BODY()

public:
	virtual void CreateInputPins() {};
	virtual void CreateOutputPins() {};

	virtual bool IsCompact() const { return false; };

	/** Add an input pin to this node and recompile the SoundCue */
	virtual void AddInputPin() {};
	/** Checks whether an input can be added to this node */
	virtual bool CanAddInputPin() const { return false; };

	VOXELEDITOR_API void GetInputPins(TArray<UEdGraphPin*>& OutInputPins);
	VOXELEDITOR_API void GetOutputPins(TArray<UEdGraphPin*>& OutOutputPins);

	VOXELEDITOR_API UEdGraphPin* GetInputPin(int32 InputIndex);
	VOXELEDITOR_API UEdGraphPin* GetOutputPin(int32 OutputIndex);

	VOXELEDITOR_API int32 GetInputCount() const;
	VOXELEDITOR_API int32 GetOutputCount() const;
	

	/**
	 * Handles inserting the node between the FromPin and what the FromPin was original connected to
	 *
	 * @param FromPin			The pin this node is being spawned from
	 * @param NewLinkPin		The new pin the FromPin will connect to
	 * @param OutNodeList		Any nodes that are modified will get added to this list for notification purposes
	 */
	void InsertNewNode(UEdGraphPin* FromPin, UEdGraphPin* NewLinkPin, TSet<UEdGraphNode*>& OutNodeList);

	// UEdGraphNode interface.
	virtual void AllocateDefaultPins() override;
	virtual void ReconstructNode() override;
	virtual void AutowireNewNode(UEdGraphPin* FromPin) override;
	virtual bool CanCreateUnderSpecifiedSchema(const UEdGraphSchema* Schema) const override;
	// End of UEdGraphNode interface.
};
