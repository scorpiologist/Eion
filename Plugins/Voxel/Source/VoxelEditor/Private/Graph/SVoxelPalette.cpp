// Copyright 2018 Phyronnaz

#include "SVoxelPalette.h"
#include "VoxelGraphSchema.h"

void SVoxelPalette::Construct(const FArguments& InArgs)
{
	// Auto expand the palette as there's so few nodes
	SGraphPalette::Construct(SGraphPalette::FArguments().AutoExpandActionMenu(true));
}

void SVoxelPalette::CollectAllActions(FGraphActionListBuilderBase& OutAllActions)
{
	const UVoxelGraphSchema* Schema = GetDefault<UVoxelGraphSchema>();

	FGraphActionMenuBuilder ActionMenuBuilder;

	Schema->GetPaletteActions(ActionMenuBuilder);
	OutAllActions.Append(ActionMenuBuilder);
}
