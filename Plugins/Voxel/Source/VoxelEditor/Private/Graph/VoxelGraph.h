// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "EdGraph/EdGraph.h"
#include "VoxelGraphWorldGenerator.h"
#include "VoxelGraph.generated.h"

UCLASS(MinimalAPI)
class UVoxelGraph : public UEdGraph
{
	GENERATED_UCLASS_BODY()

public:
	VOXELEDITOR_API class UVoxelGraphGenerator* GetWorldGenerator() const
	{
		return CastChecked<UVoxelGraphGenerator>(GetOuter());
	}
};

/////////////////////////////////////////////////////////////////////////

class FVoxelGraphEditor : public IVoxelGraphEditor
{
public:
	~FVoxelGraphEditor() {}

	UEdGraph* CreateNewVoxelGraph(UVoxelGraphGenerator* InSoundCue) override;

	void SetupVoxelNode(UEdGraph* SoundCueGraph, UVoxelNode* InSoundNode, bool bSelectNewNode) override;

	void CompileVoxelNodesFromGraphNodes(UVoxelGraphGenerator* WorldGenerator) override;

	void CreateInputPin(UEdGraphNode* VoxelNode) override;
	void CreateOutputPin(UEdGraphNode* VoxelNode) override;
};
