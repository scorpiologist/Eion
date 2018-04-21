// Copyright 2018 Phyronnaz

#include "SGraphNodeDefaultVoxel.h"

void SGraphNodeDefaultVoxel::Construct( const FArguments& InArgs )
{
	this->GraphNode = InArgs._GraphNodeObj;

	this->SetCursor( EMouseCursor::CardinalCross );

	this->UpdateGraphNode();
}
