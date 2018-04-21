// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "UObject/GCObject.h"
#include "Toolkits/IToolkitHost.h"
#include "Misc/NotifyHook.h"
#include "VoxelGraphWorldGenerator.h"
#include "EditorUndoClient.h"
#include "EditorStyleSet.h"
#include "IVoxelEditor.h"
#include "Commands.h"

class IDetailsView;
class SDockableTab;
class SGraphEditor;
class SVoxelPalette;
class UEdGraphNode;
class UVoxel;
struct FPropertyChangedEvent;
struct Rect;

class FVoxelGraphEditorCommands : public TCommands<FVoxelGraphEditorCommands>
{
public:
	/** Constructor */
	FVoxelGraphEditorCommands()
		: TCommands<FVoxelGraphEditorCommands>
		(
		"VoxelGraphEditor", // Context name for fast lookup
		NSLOCTEXT("Contexts", "VoxelGraphEditor", "Voxel Graph Editor"), // Localized context name for displaying
		NAME_None, // Parent
		"VoxelStyle" // Icon Style Set
		)
	{
	}

	/** Compile the Voxel */
	TSharedPtr<FUICommandInfo> CompileToCpp;

	/** Breaks the node input/output link */
	TSharedPtr<FUICommandInfo> BreakLink;

	/** Adds an input to the node */
	TSharedPtr<FUICommandInfo> AddInput;

	/** Removes an input from the node */
	TSharedPtr<FUICommandInfo> DeleteInput;

#define LOCTEXT_NAMESPACE "VoxelGraphEditorCommands"
	/** Initialize commands */
	virtual void RegisterCommands() override
	{
		UI_COMMAND(CompileToCpp, "Compile To C++", "Create C++ file from graph", EUserInterfaceActionType::Button, FInputChord());

		UI_COMMAND(AddInput, "Add Input", "Adds an input to the node", EUserInterfaceActionType::Button, FInputChord());
		UI_COMMAND(DeleteInput, "Delete Input", "Removes an input from the node", EUserInterfaceActionType::Button, FInputChord());
	}
#undef LOCTEXT_NAMESPACE
};

/*-----------------------------------------------------------------------------
   FVoxelEditor
-----------------------------------------------------------------------------*/

class FVoxelEditor : public IVoxelEditor, public FGCObject, public FNotifyHook, public FEditorUndoClient
{
public:
	FVoxelEditor();

	virtual void RegisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;

	/** Destructor */
	virtual ~FVoxelEditor();

	/** Edits the specified Voxel object */
	void InitVoxelEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, UObject* ObjectToEdit);

	/** IVoxelEditor interface */
	virtual UVoxelGraphGenerator* GetWorldGenerator() const override;
	virtual void SetSelection(TArray<UObject*> SelectedObjects) override;
	virtual bool GetBoundsForSelectedNodes(class FSlateRect& Rect, float Padding) override;
	virtual int32 GetNumberOfSelectedNodes() const override;
	virtual TSet<UObject*> GetSelectedNodes() const override;

	/** IToolkit interface */
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;

	virtual FString GetDocumentationLink() const override
	{
		return FString(TEXT("Engine/Audio/Voxels/Editor"));
	}

	/** FGCObject interface */
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

	//~ Begin FEditorUndoClient Interface
	virtual void PostUndo(bool bSuccess) override;
	virtual void PostRedo(bool bSuccess) override { PostUndo(bSuccess); }
	// End of FEditorUndoClient

private:
	TSharedRef<SDockTab> SpawnTab_GraphCanvas(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Properties(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Palette(const FSpawnTabArgs& Args);

protected:
	/** Called when the preview text changes */
	void OnPreviewTextChanged(const FString& Text);

	/** Called when the selection changes in the GraphEditor */
	void OnSelectedNodesChanged(const TSet<class UObject*>& NewSelection);

	/**
	 * Called when a node's title is committed for a rename
	 *
	 * @param	NewText				New title text
	 * @param	CommitInfo			How text was committed
	 * @param	NodeBeingChanged	The node being changed
	 */
	void OnNodeTitleCommitted(const FText& NewText, ETextCommit::Type CommitInfo, UEdGraphNode* NodeBeingChanged);

	/** Select every node in the graph */
	void SelectAllNodes();
	/** Whether we can select every node */
	bool CanSelectAllNodes() const;

	/** Delete the currently selected nodes */
	void DeleteSelectedNodes();
	/** Whether we are able to delete the currently selected nodes */
	bool CanDeleteNodes() const;
	/** Delete only the currently selected nodes that can be duplicated */
	void DeleteSelectedDuplicatableNodes();

	/** Cut the currently selected nodes */
	void CutSelectedNodes();
	/** Whether we are able to cut the currently selected nodes */
	bool CanCutNodes() const;

	/** Copy the currently selected nodes */
	void CopySelectedNodes();
	/** Whether we are able to copy the currently selected nodes */
	bool CanCopyNodes() const;

	/** Paste the contents of the clipboard */
	void PasteNodes();
	/** Paste the contents of the clipboard at a specific location */
	virtual void PasteNodesHere(const FVector2D& Location) override;
	/** Whether we are able to paste the contents of the clipboard */
	virtual bool CanPasteNodes() const override;

	/** Duplicate the currently selected nodes */
	void DuplicateNodes();
	/** Whether we are able to duplicate the currently selected nodes */
	bool CanDuplicateNodes() const;

	void CompileToCpp();

	/** Called to undo the last action */
	void UndoGraphAction();

	/** Called to redo the last undone action */
	void RedoGraphAction();

private:
	/** FNotifyHook interface */
	virtual void NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, UProperty* PropertyThatChanged) override;

	/** Creates all internal widgets for the tabs to point at */
	void CreateInternalWidgets();

	/** Builds the toolbar widget for the Voxel editor */
	void ExtendToolbar();

	/** Binds new graph commands to delegates */
	void BindGraphCommands();
	
	/** Add an input to the currently selected node */
	void AddInput();
	/** Whether we can add an input to the currently selected node */
	bool CanAddInput() const;

	/** Delete an input from the currently selected node */
	void DeleteInput();
	/** Whether we can delete an input from the currently selected node */
	bool CanDeleteInput() const;

	/* Create comment node on graph */
	void OnCreateComment();

	/** Create new graph editor widget */
	TSharedRef<SGraphEditor> CreateGraphEditorWidget();

private:
	/** The Voxel asset being inspected */
	UVoxelGraphGenerator* WorldGenerator;

	/** List of open tool panels; used to ensure only one exists at any one time */
	TMap< FName, TWeakPtr<SDockableTab> > SpawnedToolPanels;

	/** New Graph Editor */
	TSharedPtr<SGraphEditor> VoxelGraphEditor;

	/** Properties tab */
	TSharedPtr<class IDetailsView> VoxelProperties;

	/** Palette of Sound Node types */
	TSharedPtr<class SVoxelPalette> Palette;

	/** Command list for this editor */
	TSharedPtr<FUICommandList> GraphEditorCommands;

	/**	The tab ids for all the tabs used */
	static const FName GraphCanvasTabId;
	static const FName PropertiesTabId;
	static const FName PaletteTabId;
};
