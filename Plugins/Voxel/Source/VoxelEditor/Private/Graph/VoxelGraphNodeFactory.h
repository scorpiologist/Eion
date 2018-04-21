// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "EdGraphUtilities.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "SGraphNode.h"
#include "SGraphNodeVoxel.h"

#include "VoxelGraphNode_Base.h"
#include "VoxelGraph.h"
#include "VoxelGraphNode_Knot.h"
#include "SGraphNodeVoxelKnot.h"

class FVoxelGraphNodeFactory : public FGraphPanelNodeFactory
{
	virtual TSharedPtr<class SGraphNode> CreateNode(class UEdGraphNode* InNode) const override
	{
		if (UVoxelGraphNode_Knot* Knot = Cast<UVoxelGraphNode_Knot>(InNode))
		{
			return SNew(SGraphNodeVoxelKnot, Knot);
		}
		else if (UVoxelGraphNode_Base* VoxelNode = Cast<UVoxelGraphNode_Base>(InNode))
		{
			return SNew(SGraphNodeVoxel, VoxelNode);
		}
		return nullptr;
	}
};
