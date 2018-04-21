// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "SGraphNode.h"

class SGraphNodeDefaultVoxel : public SGraphNode
{
public:

	SLATE_BEGIN_ARGS( SGraphNodeDefaultVoxel )
		: _GraphNodeObj( static_cast<UEdGraphNode*>(NULL) )
		{}

		SLATE_ARGUMENT( UEdGraphNode*, GraphNodeObj )
	SLATE_END_ARGS()

	void Construct( const FArguments& InArgs );
};
