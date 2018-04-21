// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"

class IVoxelEditor;
class UEdGraph;
struct Rect;

//////////////////////////////////////////////////////////////////////////
// FSoundCueEditorUtilities

class VOXELEDITOR_API FVoxelEditorUtilities
{
public:

	/** Can we paste to this graph? */
	static bool CanPasteNodes(const class UEdGraph* Graph);

	/** Perform paste on graph, at location */
	static void  PasteNodesHere(class UEdGraph* Graph, const FVector2D& Location);

	/** Get the bounding area for the currently selected nodes
	 *
	 * @param Graph The Graph we are finding bounds for
	 * @param Rect Final output bounding area, including padding
	 * @param Padding An amount of padding to add to all sides of the bounds
	 *
	 * @return false if nothing is selected*/
	static bool GetBoundsForSelectedNodes(const UEdGraph* Graph, class FSlateRect& Rect, float Padding = 0.0f);

	/** Gets the number of nodes that are currently selected */
	static int32 GetNumberOfSelectedNodes(const UEdGraph* Graph);

	/** Get the currently selected set of nodes */
	static TSet<UObject*> GetSelectedNodes(const UEdGraph* Graph);

private:
	/** Get ISoundCueEditor for given object, if it exists */
	static TSharedPtr<class IVoxelEditor> GetIVoxelEditorForObject(const UObject* ObjectToFocusOn);

	FVoxelEditorUtilities() {}
};
