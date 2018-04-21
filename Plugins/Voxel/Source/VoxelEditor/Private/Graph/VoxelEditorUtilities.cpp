// Copyright 2018 Phyronnaz

#include "VoxelEditorUtilities.h"
#include "VoxelGraph.h"
#include "VoxelGraphNode.h"

#include "IVoxelEditor.h"
#include "Toolkits/ToolkitManager.h"

#include "GraphEditor.h"

bool FVoxelEditorUtilities::CanPasteNodes(const class UEdGraph* Graph)
{
	bool bCanPaste = false;
	TSharedPtr<class IVoxelEditor> VoxelEditor = GetIVoxelEditorForObject(Graph);
	if (VoxelEditor.IsValid())
	{
		bCanPaste = VoxelEditor->CanPasteNodes();
	}
	return bCanPaste;
}

void FVoxelEditorUtilities::PasteNodesHere(class UEdGraph* Graph, const FVector2D& Location)
{
	TSharedPtr<class IVoxelEditor> VoxelEditor = GetIVoxelEditorForObject(Graph);
	if (VoxelEditor.IsValid())
	{
		VoxelEditor->PasteNodesHere(Location);
	}
}

bool FVoxelEditorUtilities::GetBoundsForSelectedNodes(const UEdGraph* Graph, class FSlateRect& Rect, float Padding)
{
	TSharedPtr<class IVoxelEditor> VoxelEditor = GetIVoxelEditorForObject(Graph);
	if (VoxelEditor.IsValid())
	{
		return VoxelEditor->GetBoundsForSelectedNodes(Rect, Padding);
	}
	return false;
}

int32 FVoxelEditorUtilities::GetNumberOfSelectedNodes(const UEdGraph* Graph)
{
	TSharedPtr<class IVoxelEditor> VoxelEditor = GetIVoxelEditorForObject(Graph);
	if (VoxelEditor.IsValid())
	{
		return VoxelEditor->GetNumberOfSelectedNodes();
	}
	return 0;
}

TSet<UObject*> FVoxelEditorUtilities::GetSelectedNodes(const UEdGraph* Graph)
{
	TSharedPtr<class IVoxelEditor> VoxelEditor = GetIVoxelEditorForObject(Graph);
	if (VoxelEditor.IsValid())
	{
		return VoxelEditor->GetSelectedNodes();
	}
	return TSet<UObject*>();
}

TSharedPtr<class IVoxelEditor> FVoxelEditorUtilities::GetIVoxelEditorForObject(const UObject* ObjectToFocusOn)
{
	check(ObjectToFocusOn);

	// Find the associated Voxel
	UVoxelGraphGenerator* Voxel = Cast<const UVoxelGraph>(ObjectToFocusOn)->GetWorldGenerator();

	TSharedPtr<IVoxelEditor> VoxelEditor;
	if (Voxel != NULL)
	{
		TSharedPtr< IToolkit > FoundAssetEditor = FToolkitManager::Get().FindEditorForAsset(Voxel);
		if (FoundAssetEditor.IsValid())
		{
			VoxelEditor = StaticCastSharedPtr<IVoxelEditor>(FoundAssetEditor);
		}
	}
	return VoxelEditor;
}
