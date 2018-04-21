// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Layout/Visibility.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Input/Reply.h"
#include "SGraphNode.h"

class SVerticalBox;
class UVoxelGraphNode;

class SGraphNodeVoxel : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SGraphNodeVoxel){}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, class UVoxelGraphNode_Base* InNode);

protected:
	//~ Begin SGraphNode Interface
	virtual void UpdateGraphNode() override;
	virtual void CreateOutputSideAddButton(TSharedPtr<SVerticalBox> OutputBox) override;
	virtual EVisibility IsAddPinButtonVisible() const override;
	virtual FReply OnAddPin() override;
	//~ End SGraphNode Interface

private:
	/** Set up node in 'standard' mode */
	void UpdateStandardNode();
	/** Set up node in 'compact' mode */
	void UpdateCompactNode();
	/** Get title in compact mode */
	FText GetNodeCompactTitle() const;

	UVoxelGraphNode_Base* VoxelNode;
};
