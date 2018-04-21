// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "EdGraph/EdGraphNode.h"
#include "VoxelGraphWorldGenerator.h"
#include "VoxelGraphNode_Base.h"
#include "VoxelGraphNode.generated.h"

UCLASS(MinimalAPI)
class UVoxelGraphNode : public UVoxelGraphNode_Base
{
	GENERATED_BODY()

public:
	UPROPERTY()
		UVoxelNode* VoxelNode;


	/** Set the SoundNode this represents (also assigns this to the SoundNode in Editor)*/
	VOXELEDITOR_API void SetVoxelNode(UVoxelNode* InSoundNode);
	/** Fix up the node's owner after being copied */
	VOXELEDITOR_API void PostCopyNode();
	/** Create a new input pin for this node */
	VOXELEDITOR_API void CreateInputPin();
	/** Create a new output pin for this node */
	VOXELEDITOR_API void CreateOutputPin();
	/** Remove a specific input pin from this node and recompile the SoundCue */
	VOXELEDITOR_API void RemoveInputPin(UEdGraphPin* InGraphPin);
	/** Estimate the width of this Node from the length of its title */
	VOXELEDITOR_API int32 EstimateNodeWidth() const;


	// UVoxelGraphNode_Base interface
	virtual void CreateInputPins() override;
	virtual void CreateOutputPins() override;
	virtual bool IsCompact() const override;
	virtual void AddInputPin() override;
	virtual bool CanAddInputPin() const override;
	// End of UVoxelGraphNode_Base interface

	// UEdGraphNode interface
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual void PrepareForCopying() override;
	virtual void GetContextMenuActions(const FGraphNodeContextMenuBuilder& Context) const override;
	virtual FText GetTooltipText() const override;
	virtual FString GetDocumentationExcerptName() const override;
	// End of UEdGraphNode interface

	// UObject interface
	virtual void PostLoad() override;
	virtual void PostEditImport() override;
	virtual void PostDuplicate(bool bDuplicateForPIE) override;
	// End of UObject interface

private:
	/** Make sure the soundnode is owned by the SoundCue */
	void ResetVoxelNodeOwner();

};
