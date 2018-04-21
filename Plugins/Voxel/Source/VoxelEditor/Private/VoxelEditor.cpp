// Copyright 2018 Phyronnaz

#include "VoxelEditor.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "EdGraph/EdGraphNode.h"
#include "Modules/ModuleManager.h"
#include "EditorStyleSet.h"

#include "Components/AudioComponent.h"
#include "AudioEditorModule.h"

#include "ScopedTransaction.h"
#include "GraphEditor.h"
#include "GraphEditorActions.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EdGraphUtilities.h"
#include "SNodePanel.h"
#include "Editor.h"

#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Commands/GenericCommands.h"

#include "SVoxelPalette.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "SlateApplication.h"
#include "PlatformFilemanager.h"
#include "GenericPlatformFile.h"
#include "FileHelper.h"
#include "SNotificationList.h"
#include "NotificationManager.h"
#include "MessageDialog.h"

#include "VoxelGraph.h"
#include "VoxelGraphSchema.h"
#include "VoxelGraphNode.h"
#include "VoxelGraphNode_Root.h"

#define LOCTEXT_NAMESPACE "VoxelEditor"

const FName FVoxelEditor::GraphCanvasTabId(TEXT("VoxelEditor_GraphCanvas"));
const FName FVoxelEditor::PropertiesTabId(TEXT("VoxelEditor_Properties"));
const FName FVoxelEditor::PaletteTabId(TEXT("VoxelEditor_Palette"));

FVoxelEditor::FVoxelEditor()
	: WorldGenerator(nullptr)
{
}

void FVoxelEditor::RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_VoxelEditor", "Voxel Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(GraphCanvasTabId, FOnSpawnTab::CreateSP(this, &FVoxelEditor::SpawnTab_GraphCanvas))
		.SetDisplayName(LOCTEXT("GraphCanvasTab", "Viewport"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "GraphEditor.EventGraph_16x"));

	InTabManager->RegisterTabSpawner(PropertiesTabId, FOnSpawnTab::CreateSP(this, &FVoxelEditor::SpawnTab_Properties))
		.SetDisplayName(LOCTEXT("DetailsTab", "Details"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(PaletteTabId, FOnSpawnTab::CreateSP(this, &FVoxelEditor::SpawnTab_Palette))
		.SetDisplayName(LOCTEXT("PaletteTab", "Palette"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "Kismet.Tabs.Palette"));
}

void FVoxelEditor::UnregisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(GraphCanvasTabId);
	InTabManager->UnregisterTabSpawner(PropertiesTabId);
	InTabManager->UnregisterTabSpawner(PaletteTabId);
}

FVoxelEditor::~FVoxelEditor()
{
	GEditor->UnregisterForUndo(this);
}

void FVoxelEditor::InitVoxelEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, UObject* ObjectToEdit)
{
	WorldGenerator = CastChecked<UVoxelGraphGenerator>(ObjectToEdit);

	// Support undo/redo
	WorldGenerator->SetFlags(RF_Transactional);

	GEditor->RegisterForUndo(this);

	FGraphEditorCommands::Register();
	FVoxelGraphEditorCommands::Register();

	BindGraphCommands();

	CreateInternalWidgets();

	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_VoxelEditor_Layout_v3")
		->AddArea
		(
			FTabManager::NewPrimaryArea()->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.1f)
				->AddTab(GetToolbarTabId(), ETabState::OpenedTab)->SetHideTabWell(true)
			)
			->Split
			(
				FTabManager::NewSplitter()->SetOrientation(Orient_Horizontal)->SetSizeCoefficient(0.9f)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.225f)
					->AddTab(PropertiesTabId, ETabState::OpenedTab)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.65f)
					->AddTab(GraphCanvasTabId, ETabState::OpenedTab)->SetHideTabWell(true)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.125f)
					->AddTab(PaletteTabId, ETabState::OpenedTab)
				)
			)
		);

	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;
	FAssetEditorToolkit::InitAssetEditor(Mode, InitToolkitHost, TEXT("VoxelEditorApp"), StandaloneDefaultLayout, bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, ObjectToEdit, false);

	//IAudioEditorModule* AudioEditorModule = &FModuleManager::LoadModuleChecked<IAudioEditorModule>("AudioEditor");
	//AddMenuExtender(AudioEditorModule->GetVoxelMenuExtensibilityManager()->GetAllExtenders(GetToolkitCommands(), GetEditingObjects()));

	ExtendToolbar();
	RegenerateMenusAndToolbars();

	/*if(IsWorldCentricAssetEditor())
	{
		SpawnToolkitTab(GetToolbarTabId(), FString(), EToolkitTabSpot::ToolBar);
		SpawnToolkitTab(GraphCanvasTabId, FString(), EToolkitTabSpot::Viewport);
		SpawnToolkitTab(PropertiesTabId, FString(), EToolkitTabSpot::Details);
	}*/
}

UVoxelGraphGenerator* FVoxelEditor::GetWorldGenerator() const
{
	return WorldGenerator;
}

void FVoxelEditor::SetSelection(TArray<UObject*> SelectedObjects)
{
	if (VoxelProperties.IsValid())
	{
		VoxelProperties->SetObjects(SelectedObjects);
	}
}

bool FVoxelEditor::GetBoundsForSelectedNodes(class FSlateRect& Rect, float Padding)
{
	return VoxelGraphEditor->GetBoundsForSelectedNodes(Rect, Padding);
}

int32 FVoxelEditor::GetNumberOfSelectedNodes() const
{
	return VoxelGraphEditor->GetSelectedNodes().Num();
}

FName FVoxelEditor::GetToolkitFName() const
{
	return FName("VoxelEditor");
}

FText FVoxelEditor::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "Voxel Editor");
}

FString FVoxelEditor::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "Voxel ").ToString();
}

FLinearColor FVoxelEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0.3f, 0.2f, 0.5f, 0.5f);
}

TSharedRef<SDockTab> FVoxelEditor::SpawnTab_GraphCanvas(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == GraphCanvasTabId);

	TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
		.Label(LOCTEXT("VoxelGraphCanvasTitle", "Viewport"));

	if (VoxelGraphEditor.IsValid())
	{
		SpawnedTab->SetContent(VoxelGraphEditor.ToSharedRef());
	}

	return SpawnedTab;
}

TSharedRef<SDockTab> FVoxelEditor::SpawnTab_Properties(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == PropertiesTabId);

	return SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("LevelEditor.Tabs.Details"))
		.Label(LOCTEXT("VoxelDetailsTitle", "Details"))
		[
			VoxelProperties.ToSharedRef()
		];
}

TSharedRef<SDockTab> FVoxelEditor::SpawnTab_Palette(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == PaletteTabId);

	return SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("Kismet.Tabs.Palette"))
		.Label(LOCTEXT("VoxelPaletteTitle", "Palette"))
		[
			Palette.ToSharedRef()
		];
}

void FVoxelEditor::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(WorldGenerator);
}

void FVoxelEditor::PostUndo(bool bSuccess)
{
	if (VoxelGraphEditor.IsValid())
	{
		VoxelGraphEditor->ClearSelectionSet();
		VoxelGraphEditor->NotifyGraphChanged();
	}

}

void FVoxelEditor::NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, class UProperty* PropertyThatChanged)
{
	if (VoxelGraphEditor.IsValid() && PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		VoxelGraphEditor->NotifyGraphChanged();
	}
}

void FVoxelEditor::CreateInternalWidgets()
{
	VoxelGraphEditor = CreateGraphEditorWidget();

	FDetailsViewArgs Args;
	Args.bHideSelectionTip = true;
	Args.NotifyHook = this;

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	VoxelProperties = PropertyModule.CreateDetailView(Args);
	VoxelProperties->SetObject(WorldGenerator);

	Palette = SNew(SVoxelPalette);
}

void FVoxelEditor::ExtendToolbar()
{
	struct Local
	{
		static void FillToolbar(FToolBarBuilder& ToolbarBuilder)
		{
			ToolbarBuilder.BeginSection("Toolbar");
			{
				ToolbarBuilder.AddToolBarButton(FVoxelGraphEditorCommands::Get().CompileToCpp);
			}
			ToolbarBuilder.EndSection();
		}
	};

	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);

	ToolbarExtender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateStatic(&Local::FillToolbar)
	);

	AddToolbarExtender(ToolbarExtender);

	/*IAudioEditorModule* AudioEditorModule = &FModuleManager::LoadModuleChecked<IAudioEditorModule>("AudioEditor");
	AddToolbarExtender(AudioEditorModule->GetVoxelToolBarExtensibilityManager()->GetAllExtenders(GetToolkitCommands(), GetEditingObjects()));*/
}

void FVoxelEditor::BindGraphCommands()
{
	const FVoxelGraphEditorCommands& Commands = FVoxelGraphEditorCommands::Get();

	ToolkitCommands->MapAction(
		Commands.CompileToCpp,
		FExecuteAction::CreateSP(this, &FVoxelEditor::CompileToCpp));

	ToolkitCommands->MapAction(
		FGenericCommands::Get().Undo,
		FExecuteAction::CreateSP(this, &FVoxelEditor::UndoGraphAction));

	ToolkitCommands->MapAction(
		FGenericCommands::Get().Redo,
		FExecuteAction::CreateSP(this, &FVoxelEditor::RedoGraphAction));
}

void FVoxelEditor::AddInput()
{
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();

	// Iterator used but should only contain one node
	for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
	{
		UVoxelGraphNode* SelectedNode = Cast<UVoxelGraphNode>(*NodeIt);

		if (SelectedNode)
		{
			SelectedNode->AddInputPin();
			break;
		}
	}
}

bool FVoxelEditor::CanAddInput() const
{
	return GetSelectedNodes().Num() == 1;
}

void FVoxelEditor::DeleteInput()
{
	UEdGraphPin* SelectedPin = VoxelGraphEditor->GetGraphPinForMenu();

	UVoxelGraphNode* SelectedNode = Cast<UVoxelGraphNode>(SelectedPin->GetOwningNode());

	if (SelectedNode && SelectedNode == SelectedPin->GetOwningNode())
	{
		SelectedNode->RemoveInputPin(SelectedPin);
	}
}

bool FVoxelEditor::CanDeleteInput() const
{
	return true;
}

void FVoxelEditor::OnCreateComment()
{
	FVoxelGraphSchemaAction_NewComment CommentAction;
	CommentAction.PerformAction(WorldGenerator->VoxelGraph, NULL, VoxelGraphEditor->GetPasteLocation());
}

TSharedRef<SGraphEditor> FVoxelEditor::CreateGraphEditorWidget()
{
	if (!GraphEditorCommands.IsValid())
	{
		GraphEditorCommands = MakeShareable(new FUICommandList);

		GraphEditorCommands->MapAction(FVoxelGraphEditorCommands::Get().AddInput,
			FExecuteAction::CreateSP(this, &FVoxelEditor::AddInput),
			FCanExecuteAction::CreateSP(this, &FVoxelEditor::CanAddInput));

		GraphEditorCommands->MapAction(FVoxelGraphEditorCommands::Get().DeleteInput,
			FExecuteAction::CreateSP(this, &FVoxelEditor::DeleteInput),
			FCanExecuteAction::CreateSP(this, &FVoxelEditor::CanDeleteInput));

		// Graph Editor Commands
		GraphEditorCommands->MapAction(FGraphEditorCommands::Get().CreateComment,
			FExecuteAction::CreateSP(this, &FVoxelEditor::OnCreateComment)
		);

		// Editing commands
		GraphEditorCommands->MapAction(FGenericCommands::Get().SelectAll,
			FExecuteAction::CreateSP(this, &FVoxelEditor::SelectAllNodes),
			FCanExecuteAction::CreateSP(this, &FVoxelEditor::CanSelectAllNodes)
		);

		GraphEditorCommands->MapAction(FGenericCommands::Get().Delete,
			FExecuteAction::CreateSP(this, &FVoxelEditor::DeleteSelectedNodes),
			FCanExecuteAction::CreateSP(this, &FVoxelEditor::CanDeleteNodes)
		);

		GraphEditorCommands->MapAction(FGenericCommands::Get().Copy,
			FExecuteAction::CreateSP(this, &FVoxelEditor::CopySelectedNodes),
			FCanExecuteAction::CreateSP(this, &FVoxelEditor::CanCopyNodes)
		);

		GraphEditorCommands->MapAction(FGenericCommands::Get().Cut,
			FExecuteAction::CreateSP(this, &FVoxelEditor::CutSelectedNodes),
			FCanExecuteAction::CreateSP(this, &FVoxelEditor::CanCutNodes)
		);

		GraphEditorCommands->MapAction(FGenericCommands::Get().Paste,
			FExecuteAction::CreateSP(this, &FVoxelEditor::PasteNodes),
			FCanExecuteAction::CreateSP(this, &FVoxelEditor::CanPasteNodes)
		);

		GraphEditorCommands->MapAction(FGenericCommands::Get().Duplicate,
			FExecuteAction::CreateSP(this, &FVoxelEditor::DuplicateNodes),
			FCanExecuteAction::CreateSP(this, &FVoxelEditor::CanDuplicateNodes)
		);
	}

	FGraphAppearanceInfo AppearanceInfo;
	AppearanceInfo.CornerText = LOCTEXT("AppearanceCornerText_Voxel", "VOXEL");

	SGraphEditor::FGraphEditorEvents InEvents;
	InEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(this, &FVoxelEditor::OnSelectedNodesChanged);
	InEvents.OnTextCommitted = FOnNodeTextCommitted::CreateSP(this, &FVoxelEditor::OnNodeTitleCommitted);
	//InEvents.OnNodeDoubleClicked = FSingleNodeEvent::CreateSP(this, &FVoxelEditor::PlaySingleNode);

	return SNew(SGraphEditor)
		.AdditionalCommands(GraphEditorCommands)
		.IsEditable(true)
		.Appearance(AppearanceInfo)
		.GraphToEdit(WorldGenerator->GetGraph())
		.GraphEvents(InEvents)
		.AutoExpandActionMenu(true)
		.ShowGraphStateOverlay(false);
}

FGraphPanelSelectionSet FVoxelEditor::GetSelectedNodes() const
{
	FGraphPanelSelectionSet CurrentSelection;
	if (VoxelGraphEditor.IsValid())
	{
		CurrentSelection = VoxelGraphEditor->GetSelectedNodes();
	}
	return CurrentSelection;
}

void FVoxelEditor::OnSelectedNodesChanged(const TSet<class UObject*>& NewSelection)
{
	TArray<UObject*> Selection;

	if (NewSelection.Num())
	{
		for (TSet<class UObject*>::TConstIterator SetIt(NewSelection); SetIt; ++SetIt)
		{
			if (Cast<UVoxelGraphNode_Root>(*SetIt))
			{
				Selection.Add(GetWorldGenerator());
			}
			else if (UVoxelGraphNode* GraphNode = Cast<UVoxelGraphNode>(*SetIt))
			{
				Selection.Add(GraphNode->VoxelNode);
			}
			else
			{
				Selection.Add(*SetIt);
			}
		}
		//Selection = NewSelection.Array();
	}
	else
	{
		Selection.Add(GetWorldGenerator());
	}

	SetSelection(Selection);
}

void FVoxelEditor::OnNodeTitleCommitted(const FText& NewText, ETextCommit::Type CommitInfo, UEdGraphNode* NodeBeingChanged)
{
	if (NodeBeingChanged)
	{
		const FScopedTransaction Transaction(LOCTEXT("RenameNode", "Rename Node"));
		NodeBeingChanged->Modify();
		NodeBeingChanged->OnRenameNode(NewText.ToString());
	}
}

void FVoxelEditor::SelectAllNodes()
{
	VoxelGraphEditor->SelectAllNodes();
}

bool FVoxelEditor::CanSelectAllNodes() const
{
	return true;
}

void FVoxelEditor::DeleteSelectedNodes()
{
	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "VoxelEditorDeleteSelectedNode", "Delete Selected Voxel Node"));

	VoxelGraphEditor->GetCurrentGraph()->Modify();

	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();

	VoxelGraphEditor->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
	{
		UEdGraphNode* Node = CastChecked<UEdGraphNode>(*NodeIt);

		if (Node->CanUserDeleteNode())
		{
			if (UVoxelGraphNode* SoundGraphNode = Cast<UVoxelGraphNode>(Node))
			{
				UVoxelNode* DelNode = SoundGraphNode->VoxelNode;

				FBlueprintEditorUtils::RemoveNode(NULL, SoundGraphNode, true);

				// Make sure Voxel is updated to match graph
				WorldGenerator->CompileVoxelNodesFromGraphNodes();

				// Remove this node from the Voxel's list of all SoundNodes
				WorldGenerator->AllNodes.Remove(DelNode);
				WorldGenerator->MarkPackageDirty();
			}
			else
			{
				FBlueprintEditorUtils::RemoveNode(NULL, Node, true);
			}
		}
	}
}

bool FVoxelEditor::CanDeleteNodes() const
{
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();

	if (SelectedNodes.Num() == 1)
	{
		for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
		{
			UVoxelGraphNode* GraphNode = Cast<UVoxelGraphNode>(*NodeIt);
			if (GraphNode && !GraphNode->CanUserDeleteNode())
			{
				return false;
			}
		}
	}

	return SelectedNodes.Num() > 0;
}

void FVoxelEditor::DeleteSelectedDuplicatableNodes()
{
	// Cache off the old selection
	const FGraphPanelSelectionSet OldSelectedNodes = GetSelectedNodes();

	// Clear the selection and only select the nodes that can be duplicated
	FGraphPanelSelectionSet RemainingNodes;
	VoxelGraphEditor->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(OldSelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if ((Node != NULL) && Node->CanDuplicateNode())
		{
			VoxelGraphEditor->SetNodeSelection(Node, true);
		}
		else
		{
			RemainingNodes.Add(Node);
		}
	}

	// Delete the duplicatable nodes
	DeleteSelectedNodes();

	// Reselect whatever's left from the original selection after the deletion
	VoxelGraphEditor->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(RemainingNodes); SelectedIter; ++SelectedIter)
	{
		if (UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter))
		{
			VoxelGraphEditor->SetNodeSelection(Node, true);
		}
	}
}

void FVoxelEditor::CutSelectedNodes()
{
	CopySelectedNodes();
	// Cut should only delete nodes that can be duplicated
	DeleteSelectedDuplicatableNodes();
}

bool FVoxelEditor::CanCutNodes() const
{
	return CanCopyNodes() && CanDeleteNodes();
}

void FVoxelEditor::CopySelectedNodes()
{
	// Export the selected nodes and place the text on the clipboard
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();

	FString ExportedText;

	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		if (UVoxelGraphNode* Node = Cast<UVoxelGraphNode>(*SelectedIter))
		{
			Node->PrepareForCopying();
		}
	}

	FEdGraphUtilities::ExportNodesToText(SelectedNodes, /*out*/ ExportedText);
	FPlatformApplicationMisc::ClipboardCopy(*ExportedText);

	// Make sure Voxel remains the owner of the copied nodes
	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		if (UVoxelGraphNode* Node = Cast<UVoxelGraphNode>(*SelectedIter))
		{
			Node->PostCopyNode();
		}
	}
}

bool FVoxelEditor::CanCopyNodes() const
{
	// If any of the nodes can be duplicated then we should allow copying
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if ((Node != NULL) && Node->CanDuplicateNode())
		{
			return true;
		}
	}
	return false;
}

void FVoxelEditor::PasteNodes()
{
	PasteNodesHere(VoxelGraphEditor->GetPasteLocation());
}

void FVoxelEditor::PasteNodesHere(const FVector2D& Location)
{
	// Undo/Redo support
	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "VoxelEditorPaste", "Paste Voxel Node"));
	WorldGenerator->GetGraph()->Modify();
	WorldGenerator->Modify();

	// Clear the selection set (newly pasted stuff will be selected)
	VoxelGraphEditor->ClearSelectionSet();

	// Grab the text to paste from the clipboard.
	FString TextToImport;
	FPlatformApplicationMisc::ClipboardPaste(TextToImport);

	// Import the nodes
	TSet<UEdGraphNode*> PastedNodes;
	FEdGraphUtilities::ImportNodesFromText(WorldGenerator->GetGraph(), TextToImport, /*out*/ PastedNodes);

	//Average position of nodes so we can move them while still maintaining relative distances to each other
	FVector2D AvgNodePosition(0.0f, 0.0f);

	for (TSet<UEdGraphNode*>::TIterator It(PastedNodes); It; ++It)
	{
		UEdGraphNode* Node = *It;
		AvgNodePosition.X += Node->NodePosX;
		AvgNodePosition.Y += Node->NodePosY;
	}

	if (PastedNodes.Num() > 0)
	{
		float InvNumNodes = 1.0f / float(PastedNodes.Num());
		AvgNodePosition.X *= InvNumNodes;
		AvgNodePosition.Y *= InvNumNodes;
	}

	for (TSet<UEdGraphNode*>::TIterator It(PastedNodes); It; ++It)
	{
		UEdGraphNode* Node = *It;

		if (UVoxelGraphNode* SoundGraphNode = Cast<UVoxelGraphNode>(Node))
		{
			WorldGenerator->AllNodes.Add(SoundGraphNode->VoxelNode);
		}

		// Select the newly pasted stuff
		VoxelGraphEditor->SetNodeSelection(Node, true);

		Node->NodePosX = (Node->NodePosX - AvgNodePosition.X) + Location.X;
		Node->NodePosY = (Node->NodePosY - AvgNodePosition.Y) + Location.Y;

		Node->SnapToGrid(SNodePanel::GetSnapGridSize());

		// Give new node a different Guid from the old one
		Node->CreateNewGuid();
	}

	// Force new pasted SoundNodes to have same connections as graph nodes
	WorldGenerator->CompileVoxelNodesFromGraphNodes();

	// Update UI
	VoxelGraphEditor->NotifyGraphChanged();

	WorldGenerator->PostEditChange();
	WorldGenerator->MarkPackageDirty();
}

bool FVoxelEditor::CanPasteNodes() const
{
	FString ClipboardContent;
	FPlatformApplicationMisc::ClipboardPaste(ClipboardContent);

	return FEdGraphUtilities::CanImportNodesFromText(WorldGenerator->VoxelGraph, ClipboardContent);
}

void FVoxelEditor::DuplicateNodes()
{
	// Copy and paste current selection
	CopySelectedNodes();
	PasteNodes();
}

bool FVoxelEditor::CanDuplicateNodes() const
{
	return CanCopyNodes();
}

void FVoxelEditor::CompileToCpp()
{
	/*
	 * Get path
	 */
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

	if (!DesktopPlatform)
	{
		return;
	}

	const FString DefaultPath = FPaths::GetProjectFilePath();

	// show the file browse dialog
	TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().FindWidgetWindow(VoxelGraphEditor.ToSharedRef());
	void* ParentWindowHandle = (ParentWindow.IsValid() && ParentWindow->GetNativeWindow().IsValid()) ? ParentWindow->GetNativeWindow()->GetOSWindowHandle() : nullptr;
	
	const FString DirectoryPath(FPaths::GameSourceDir() + TEXT("GeneratedWorldGenerators/"));
	if (!FPaths::DirectoryExists(DirectoryPath))
	{
		FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*DirectoryPath);
		FString Text;

		Text.Append(TEXT("using UnrealBuildTool;\n\n"));
		Text.Append(TEXT("public class GeneratedWorldGenerators : ModuleRules\n"));
		Text.Append(TEXT("{\n"));
		Text.Append(TEXT("	public GeneratedWorldGenerators(ReadOnlyTargetRules Target) : base(Target)\n"));
		Text.Append(TEXT("	{\n"));
		Text.Append(TEXT("		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;\n\n"));

		Text.Append(TEXT("		PublicDependencyModuleNames.AddRange(new string[] { \"Core\", \"CoreUObject\", \"Engine\", \"InputCore\", \"Voxel\" });\n\n"));
		Text.Append(TEXT("	}\n"));
		Text.Append(TEXT("}"));

		FFileHelper::SaveStringToFile(Text, *(DirectoryPath + TEXT("GeneratedWorldGenerators.Build.cs")));

		Text.Reset();
		Text.Append(TEXT("#pragma once\n\n#include \"CoreMinimal.h\"\n"));
		
		FFileHelper::SaveStringToFile(Text, *(DirectoryPath + TEXT("GeneratedWorldGenerators.h")));

		Text.Reset();
		Text.Append(TEXT("#include \"GeneratedWorldGenerators.h\"\n"));
		Text.Append(TEXT("#include \"Modules/ModuleManager.h\"\n\n"));
		Text.Append(TEXT("IMPLEMENT_MODULE( FDefaultGameModuleImpl, GeneratedWorldGenerators );\n"));
		
		FFileHelper::SaveStringToFile(Text, *(DirectoryPath + TEXT("GeneratedWorldGenerators.cpp")));
	}

	{
		IFileManager& FileManager = IFileManager::Get();
		TArray<FString> FoundFiles;
		FileManager.FindFiles(FoundFiles, *FPaths::ProjectDir(), TEXT(".uproject"));

		for (const auto& File : FoundFiles)
		{
			const FString TargetPath(FPaths::ProjectDir() + File);
			FString TargetContent;
			FFileHelper::LoadFileToString(TargetContent, *TargetPath);
			if (!TargetContent.Contains(TEXT("\"GeneratedWorldGenerators\"")))
			{
				auto Result = FMessageDialog::Open(EAppMsgType::YesNoCancel, FText::FromString(File + TEXT(" doesn't have the GeneratedWorldGenerators module dependency. Add it automatically? (You'll have to restart the editor)\n\nChoose Yes if you don't know")));
				switch (Result)
				{
				case EAppReturnType::No:
					continue;
				case EAppReturnType::Yes:
					break;
				case EAppReturnType::Cancel:
					return;
				default:
					check(false);
					break;
				}

				int32 Position = TargetContent.Find(TEXT("\"Modules\": ["));
				if (Position < 0)
				{
					FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Cannot add the GeneratedWorldGenerators module dependency")));
				}
				else
				{
					FString Text;
					Text.Append(TEXT("\n"));
					Text.Append(TEXT("\t\t{\n"));
					Text.Append(TEXT("\t\t\t\"Name\": \"GeneratedWorldGenerators\",\n"));
					Text.Append(TEXT("\t\t\t\"Type\": \"Runtime\",\n"));
					Text.Append(TEXT("\t\t\t\"LoadingPhase\": \"Default\",\n"));
					Text.Append(TEXT("\t\t\t\"AdditionalDependencies\": [\n"));
					Text.Append(TEXT("\t\t\t\t\"Engine\",\n"));
					Text.Append(TEXT("\t\t\t\t\"CoreUObject\"\n"));
					Text.Append(TEXT("\t\t\t]\n"));
					Text.Append(TEXT("\t\t},"));

					TargetContent.InsertAt(Position + FString(TEXT("\"Modules\": [")).Len(), Text);

					FileManager.Delete(*TargetPath);
					FFileHelper::SaveStringToFile(TargetContent, *TargetPath);
				}
			}
		}
	}

	{
		IFileManager& FileManager = IFileManager::Get();
		TArray<FString> FoundFiles;
		FileManager.FindFiles(FoundFiles, *FPaths::GameSourceDir(), TEXT(".Target.cs"));

		for (const auto& File : FoundFiles)
		{
			const FString TargetPath(FPaths::GameSourceDir() + File);
			FString TargetContent;
			FFileHelper::LoadFileToString(TargetContent, *TargetPath);
			if (!TargetContent.Contains(TEXT("\"GeneratedWorldGenerators\"")))
			{
				auto Result = FMessageDialog::Open(EAppMsgType::YesNoCancel, FText::FromString(File + TEXT(" doesn't have the GeneratedWorldGenerators module dependency. Add it automatically? (You'll have to restart the editor)\n\nChoose Yes if you don't know")));
				switch (Result)
				{
				case EAppReturnType::No:
					continue;
				case EAppReturnType::Yes:
					break;
				case EAppReturnType::Cancel:
					return;
				default:
					check(false);
					break;
				}
				
				int32 Position = TargetContent.Find(TEXT("ExtraModuleNames.AddRange("));
				if (Position < 0)
				{
					FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Cannot add the GeneratedWorldGenerators module dependency")));
				}
				else
				{
					TargetContent.InsertAt(Position, TEXT("ExtraModuleNames.AddRange( new string[] { \"GeneratedWorldGenerators\" } );\n\t\t"));
				
					FileManager.Delete(*TargetPath);
					FFileHelper::SaveStringToFile(TargetContent, *TargetPath);
				}
			}
		}
	}

	TArray<FString> OutFiles;
	if (DesktopPlatform->SaveFileDialog(ParentWindowHandle, TEXT("File to create"), DirectoryPath, WorldGenerator->GetName() + TEXT(".h"), TEXT(".h"), EFileDialogFlags::None, OutFiles))
	{
		check(OutFiles.Num() == 1);

		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

		// Get absolute file path
		FString AbsoluteFilePath = OutFiles[0];

		FString CppText;
		FString ErrorMessage;
		bool bCompilationSuccess = WorldGenerator->CompileToCpp(CppText, FPaths::GetBaseFilename(AbsoluteFilePath), ErrorMessage);

		if (!bCompilationSuccess)
		{
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Compilation failed!\nError: ") + ErrorMessage));
			return;
		}

		bool bSuccess = FFileHelper::SaveStringToFile(CppText, *AbsoluteFilePath);

		if (bSuccess)
		{
			FString Text = AbsoluteFilePath + TEXT(" was successfully created");
			FNotificationInfo Info(FText::FromString(Text));
			Info.ExpireDuration = 10.0f;
			FSlateNotificationManager::Get().AddNotification(Info);
		}
		else
		{
			FString Text = AbsoluteFilePath + TEXT(" was NOT successfully created");
			FNotificationInfo Info(FText::FromString(Text));
			Info.ExpireDuration = 4.0f;
			Info.CheckBoxState = ECheckBoxState::Unchecked;
			FSlateNotificationManager::Get().AddNotification(Info);
		}
	}
}

void FVoxelEditor::UndoGraphAction()
{
	GEditor->UndoTransaction();
}

void FVoxelEditor::RedoGraphAction()
{
	// Clear selection, to avoid holding refs to nodes that go away
	VoxelGraphEditor->ClearSelectionSet();

	GEditor->RedoTransaction();
}

#undef LOCTEXT_NAMESPACE
