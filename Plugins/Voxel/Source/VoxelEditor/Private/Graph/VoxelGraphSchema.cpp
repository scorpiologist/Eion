// Copyright 2018 Phyronnaz

#include "VoxelGraphSchema.h"
#include "ScopedTransaction.h"
#include "Internationalization.h"
#include "EdGraphNode_Comment.h"
#include "VoxelEditorUtilities.h"
#include "SlateRect.h"

#include "VoxelNoiseNodes.h"
#include "VoxelGraphWorldGenerator.h"
#include "VoxelGraph.h"
#include "VoxelGraphNode_Root.h"
#include "GraphEditorSettings.h"
#include "VoxelGraphNode_Knot.h"
#include "UObjectIterator.h"
#include "Runtime/Launch/Resources/Version.h"

#define LOCTEXT_NAMESPACE "VoxelGraphSchema"

FVoxelGraphSchemaAction_NewNode::FVoxelGraphSchemaAction_NewNode()
	: Super()
{

}

FVoxelGraphSchemaAction_NewNode::FVoxelGraphSchemaAction_NewNode(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping)
	: FEdGraphSchemaAction(MoveTemp(InNodeCategory), MoveTemp(InMenuDesc), MoveTemp(InToolTip), InGrouping)
	, VoxelNodeClass(NULL)
{
}

UEdGraphNode* FVoxelGraphSchemaAction_NewNode::PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode/* = true*/)
{
	check(VoxelNodeClass);

	UVoxelGraph* VoxelGraph = CastChecked<UVoxelGraph>(ParentGraph);

	UVoxelGraphGenerator* WorldGenerator = VoxelGraph->GetWorldGenerator();
	const FScopedTransaction Transaction(LOCTEXT("VoxelEditorNewNode", "Voxel Editor: New Node"));
	ParentGraph->Modify();
	WorldGenerator->Modify();

	UVoxelNode* NewNode = WorldGenerator->ConstructNewNode(VoxelNodeClass);

	NewNode->CreateStartingConnectors();

	NewNode->GraphNode->NodePosX = Location.X;
	NewNode->GraphNode->NodePosY = Location.Y;

	NewNode->GraphNode->AutowireNewNode(FromPin);

	WorldGenerator->PostEditChange();
	WorldGenerator->MarkPackageDirty();

	return NewNode->GraphNode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

FVoxelGraphSchemaAction_NewKnotNode::FVoxelGraphSchemaAction_NewKnotNode()
{

}

FVoxelGraphSchemaAction_NewKnotNode::FVoxelGraphSchemaAction_NewKnotNode(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping)
	: FEdGraphSchemaAction(MoveTemp(InNodeCategory), MoveTemp(InMenuDesc), MoveTemp(InToolTip), InGrouping)
{

}

UEdGraphNode* FVoxelGraphSchemaAction_NewKnotNode::PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode /*= true*/)
{
	const FScopedTransaction Transaction(LOCTEXT("VoxelEditorNewKnotNode", "Voxel Editor: New Knot Node"));
	ParentGraph->Modify();

	FGraphNodeCreator<UVoxelGraphNode_Knot> KnotNodeCreator(*ParentGraph);
	UVoxelGraphNode_Knot* KnotNode = KnotNodeCreator.CreateNode();
	KnotNodeCreator.Finalize();
	//SetNodeMetaData(KnotNode, FNodeMetadata::DefaultGraphNode);

	KnotNode->NodePosX = Location.X;
	KnotNode->NodePosY = Location.Y;
	
	KnotNode->AutowireNewNode(FromPin);

	KnotNode->PropagatePinType();

	return KnotNode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

FVoxelGraphSchemaAction_NewComment::FVoxelGraphSchemaAction_NewComment(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping)
	: FEdGraphSchemaAction(MoveTemp(InNodeCategory), MoveTemp(InMenuDesc), MoveTemp(InToolTip), InGrouping)
{

}

FVoxelGraphSchemaAction_NewComment::FVoxelGraphSchemaAction_NewComment()
	: Super()
{

}

UEdGraphNode* FVoxelGraphSchemaAction_NewComment::PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode /*= true*/)
{
	// Add menu item for creating comment boxes
	UEdGraphNode_Comment* CommentTemplate = NewObject<UEdGraphNode_Comment>();

	FVector2D SpawnLocation = Location;

	FSlateRect Bounds;
	if (FVoxelEditorUtilities::GetBoundsForSelectedNodes(ParentGraph, Bounds, 50.0f))
	{
		CommentTemplate->SetBounds(Bounds);
		SpawnLocation.X = CommentTemplate->NodePosX;
		SpawnLocation.Y = CommentTemplate->NodePosY;
	}

	return FEdGraphSchemaAction_NewNode::SpawnNodeFromTemplate<UEdGraphNode_Comment>(ParentGraph, CommentTemplate, SpawnLocation);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

FVoxelGraphSchemaAction_Paste::FVoxelGraphSchemaAction_Paste(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping)
	: FEdGraphSchemaAction(MoveTemp(InNodeCategory), MoveTemp(InMenuDesc), MoveTemp(InToolTip), InGrouping)
{

}

FVoxelGraphSchemaAction_Paste::FVoxelGraphSchemaAction_Paste()
	: Super()
{

}

UEdGraphNode* FVoxelGraphSchemaAction_Paste::PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode /*= true*/)
{
	FVoxelEditorUtilities::PasteNodesHere(ParentGraph, Location);
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

UVoxelGraphSchema::UVoxelGraphSchema(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool UVoxelGraphSchema::ConnectionCausesLoop(const UEdGraphPin* InputPin, const UEdGraphPin* OutputPin) const
{
	UEdGraphNode* const StartNode = OutputPin->GetOwningNode();

	TSet<UEdGraphNode*> ProcessedNodes;

	TArray<UEdGraphNode*> NodesToProcess;
	NodesToProcess.Add(InputPin->GetOwningNode());
	
	while (NodesToProcess.Num() > 0)
	{
		UEdGraphNode* Node = NodesToProcess.Pop(false);

		ProcessedNodes.Add(Node);

		for (UEdGraphPin* Pin : Node->GetAllPins())
		{
			if (Pin->Direction == EGPD_Output)
			{
				for (auto LPin : Pin->LinkedTo)
				{
					check(LPin->Direction == EGPD_Input);

					UEdGraphNode* NewNode = LPin->GetOwningNode();
					check(NewNode);

					if (StartNode == NewNode)
					{
						return true;
					}
					NodesToProcess.Add(NewNode);
				}
			}
		}
	}

	return false;
}

void UVoxelGraphSchema::GetPaletteActions(FGraphActionMenuBuilder& ActionMenuBuilder) const
{
	GetAllVoxelNodeActions(ActionMenuBuilder);
	GetCommentAction(ActionMenuBuilder);
}

void UVoxelGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	GetAllVoxelNodeActions(ContextMenuBuilder);
	GetCommentAction(ContextMenuBuilder, ContextMenuBuilder.CurrentGraph);

	if (!ContextMenuBuilder.FromPin && FVoxelEditorUtilities::CanPasteNodes(ContextMenuBuilder.CurrentGraph))
	{
		TSharedPtr<FVoxelGraphSchemaAction_Paste> NewAction(new FVoxelGraphSchemaAction_Paste(FText::GetEmpty(), LOCTEXT("PasteHereAction", "Paste here"), FText::GetEmpty(), 0));
		ContextMenuBuilder.AddAction(NewAction);
	}
}

bool UVoxelGraphSchema::TryCreateConnection(UEdGraphPin* A, UEdGraphPin* B) const
{
	bool bModified = UEdGraphSchema::TryCreateConnection(A, B);

	if (bModified)
	{
		CastChecked<UVoxelGraph>(A->GetOwningNode()->GetGraph())->GetWorldGenerator()->CompileVoxelNodesFromGraphNodes();
	}

	auto AK = Cast<UVoxelGraphNode_Knot>(A->GetOwningNode());
	auto BK = Cast<UVoxelGraphNode_Knot>(A->GetOwningNode());
	if (AK)
	{
		AK->PropagatePinType();
	}
	if (BK)
	{
		BK->PropagatePinType();
	}

	return bModified;
}

void UVoxelGraphSchema::TrySetDefaultValue(UEdGraphPin& Pin, const FString& NewDefaultValue) const
{
	FString DefaultValue;
	/*if (Pin.PinType.PinCategory == FVoxelPinCategory::PC_Int)
	{
		DefaultValue = FString::FromInt(FMath::Clamp(FCString::Atoi(*NewDefaultValue), 0, 255));
	}
	else*/
	{
		DefaultValue = NewDefaultValue;
	}

	Super::TrySetDefaultValue(Pin, DefaultValue);

	CastChecked<UVoxelGraph>(Pin.GetOwningNode()->GetGraph())->GetWorldGenerator()->CompileVoxelNodesFromGraphNodes();
}

TArray<UClass*> UVoxelGraphSchema::VoxelNodeClasses;
bool UVoxelGraphSchema::bVoxelNodeClassesInitialized = false;

FLinearColor UVoxelGraphSchema::GetPinTypeColor(const FEdGraphPinType& PinType) const
{
#if ENGINE_MINOR_VERSION < 19
	const FString& TypeString = PinType.PinCategory;
#else 
	const FString& TypeString = PinType.PinCategory.ToString();
#endif
	const UGraphEditorSettings* Settings = GetDefault<UGraphEditorSettings>();

	if (TypeString == FVoxelPinCategory::PC_Exec)
	{
		return Settings->ExecutionPinTypeColor;
	}
	else if (TypeString == FVoxelPinCategory::PC_Float)
	{
		return Settings->FloatPinTypeColor;
	}
	else if (TypeString == FVoxelPinCategory::PC_Boolean)
	{
		return Settings->BooleanPinTypeColor;
	}
	else if (TypeString == FVoxelPinCategory::PC_Int)
	{
		return Settings->IntPinTypeColor;
	}
	else if (TypeString == FVoxelPinCategory::PC_Material)
	{
		return Settings->ObjectPinTypeColor;
	}

	// Type does not have a defined color!
	return Settings->DefaultPinTypeColor;
}

TSharedPtr<FEdGraphSchemaAction> UVoxelGraphSchema::GetCreateCommentAction() const
{
	return TSharedPtr<FEdGraphSchemaAction>(static_cast<FEdGraphSchemaAction*>(new FVoxelGraphSchemaAction_NewComment));
}

void UVoxelGraphSchema::CreateDefaultNodesForGraph(UEdGraph& Graph) const
{
	const int32 RootNodeHeightOffset = -58;

	// Create the result node
	FGraphNodeCreator<UVoxelGraphNode_Root> StartNodeCreator(Graph);
	UVoxelGraphNode_Root* StartNode = StartNodeCreator.CreateNode();
	StartNodeCreator.Finalize();
	SetNodeMetaData(StartNode, FNodeMetadata::DefaultGraphNode);
}

void UVoxelGraphSchema::BreakNodeLinks(UEdGraphNode& TargetNode) const
{
	Super::BreakNodeLinks(TargetNode);

	CastChecked<UVoxelGraph>(TargetNode.GetGraph())->GetWorldGenerator()->CompileVoxelNodesFromGraphNodes();
}

void UVoxelGraphSchema::BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotifcation) const
{
	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "GraphEd_BreakPinLinks", "Break Pin Links"));

	auto OldLinkedTo = TargetPin.LinkedTo;
	Super::BreakPinLinks(TargetPin, bSendsNodeNotifcation);

	// if this would notify the node then we need to compile the SoundCue
	if (bSendsNodeNotifcation)
	{
		CastChecked<UVoxelGraph>(TargetPin.GetOwningNode()->GetGraph())->GetWorldGenerator()->CompileVoxelNodesFromGraphNodes();
	}	

	auto AK = Cast<UVoxelGraphNode_Knot>(TargetPin.GetOwningNode());
	if (AK)
	{
		AK->PropagatePinType();
	}
	for (auto Pin : OldLinkedTo)
	{
		auto BK = Cast<UVoxelGraphNode_Knot>(Pin->GetOwningNode());
		if (BK)
		{
			BK->PropagatePinType();
		}
	}
}

void UVoxelGraphSchema::OnPinConnectionDoubleCicked(UEdGraphPin* PinA, UEdGraphPin* PinB, const FVector2D& GraphPosition) const
{
	const FScopedTransaction Transaction(LOCTEXT("CreateRerouteNodeOnWire", "Create Reroute Node"));

	const FVector2D NodeSpacerSize(42.0f, 24.0f);
	const FVector2D KnotTopLeft = GraphPosition - (NodeSpacerSize * 0.5f);

	// Create a new knot
	UEdGraph* ParentGraph = PinA->GetOwningNode()->GetGraph();

	FVoxelGraphSchemaAction_NewKnotNode Action;
	UVoxelGraphNode_Knot* NewKnot = Cast<UVoxelGraphNode_Knot>(Action.PerformAction(ParentGraph, NULL, KnotTopLeft, true));

	// Move the connections across (only notifying the knot, as the other two didn't really change)
	PinA->BreakLinkTo(PinB);
	PinA->MakeLinkTo((PinA->Direction == EGPD_Output) ? NewKnot->GetInputPin() : NewKnot->GetOutputPin());
	PinB->MakeLinkTo((PinB->Direction == EGPD_Output) ? NewKnot->GetInputPin() : NewKnot->GetOutputPin());
	NewKnot->PropagatePinType();

	// Recompile
	CastChecked<UVoxelGraph>(PinA->GetOwningNode()->GetGraph())->GetWorldGenerator()->CompileVoxelNodesFromGraphNodes();
}

const FPinConnectionResponse UVoxelGraphSchema::CanCreateConnection(const UEdGraphPin* PinA, const UEdGraphPin* PinB) const
{
	// Make sure the pins are not on the same node
	if (PinA->GetOwningNode() == PinB->GetOwningNode())
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("ConnectionSameNode", "Both are on the same node"));
	}

	// Compare the directions
	const UEdGraphPin* InputPin = NULL;
	const UEdGraphPin* OutputPin = NULL;

	if (!CategorizePinsByDirection(PinA, PinB, /*out*/ InputPin, /*out*/ OutputPin))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("ConnectionIncompatible", "Directions are not compatible"));
	}

	check(InputPin);
	check(OutputPin);
	if (InputPin->PinType.PinCategory != OutputPin->PinType.PinCategory && InputPin->PinType.PinCategory != WILDCARD && OutputPin->PinType.PinCategory != WILDCARD)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("TypesIncompatible", "Types are not compatible"));
	}

	if (ConnectionCausesLoop(InputPin, OutputPin))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("ConnectionLoop", "Connection would cause loop"));
	}

	// Break existing connections on inputs only except for exec - multiple output connections are acceptable
#if ENGINE_MINOR_VERSION < 19
	if (InputPin->LinkedTo.Num() > 0 && InputPin->PinType.PinCategory != FVoxelPinCategory::PC_Exec)
#else
	if (InputPin->LinkedTo.Num() > 0 && InputPin->PinType.PinCategory.ToString() != FVoxelPinCategory::PC_Exec)
#endif
	{
		ECanCreateConnectionResponse ReplyBreakOutputs;
		if (InputPin == PinA)
		{
			ReplyBreakOutputs = CONNECT_RESPONSE_BREAK_OTHERS_A;
		}
		else
		{
			ReplyBreakOutputs = CONNECT_RESPONSE_BREAK_OTHERS_B;
		}
		return FPinConnectionResponse(ReplyBreakOutputs, LOCTEXT("ConnectionReplace", "Replace existing connections"));
	}
	
#if ENGINE_MINOR_VERSION < 19
	if (OutputPin->PinType.PinCategory == FVoxelPinCategory::PC_Exec && OutputPin->LinkedTo.Num() > 0)
#else
	if (OutputPin->PinType.PinCategory.ToString() == FVoxelPinCategory::PC_Exec && OutputPin->LinkedTo.Num() > 0)
#endif
	{
		ECanCreateConnectionResponse ReplyBreakOutputs;
		if (OutputPin == PinA)
		{
			ReplyBreakOutputs = CONNECT_RESPONSE_BREAK_OTHERS_A;
		}
		else
		{
			ReplyBreakOutputs = CONNECT_RESPONSE_BREAK_OTHERS_B;
		}
		return FPinConnectionResponse(ReplyBreakOutputs, LOCTEXT("ConnectionReplace", "Replace existing connections"));
	}

	return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, TEXT(""));
}

void UVoxelGraphSchema::GetAllVoxelNodeActions(FGraphActionMenuBuilder& ActionMenuBuilder) const
{
	InitVoxelNodeClasses();

	for (auto NodeClass : VoxelNodeClasses)
	{
		const FText Name = FText::FromString(NodeClass->GetDescription());

		FFormatNamedArguments Arguments;
		Arguments.Add(TEXT("Name"), Name);
		const FText AddToolTip = FText::Format(LOCTEXT("NewVoxelNodeTooltip", "Adds {Name} node here"), Arguments);

		bool bValid = !ActionMenuBuilder.FromPin;
		if (!bValid)
		{
			UVoxelNode* TmpNode = NewObject<UVoxelNode>((UObject*)GetTransientPackage(), NodeClass);
			bValid = ActionMenuBuilder.FromPin->Direction == EGPD_Output
#if ENGINE_MINOR_VERSION < 19
				? TmpNode->HasInputPinWithCategory(ActionMenuBuilder.FromPin->PinType.PinCategory)
				: TmpNode->HasOutputPinWithCategory(ActionMenuBuilder.FromPin->PinType.PinCategory);
#else
				? TmpNode->HasInputPinWithCategory(ActionMenuBuilder.FromPin->PinType.PinCategory.ToString())
				: TmpNode->HasOutputPinWithCategory(ActionMenuBuilder.FromPin->PinType.PinCategory.ToString());
#endif
		}

		if (bValid)
		{
			if (NodeClass->IsChildOf(UVoxelNode_NoiseNode::StaticClass()))
			{
				TSharedPtr<FVoxelGraphSchemaAction_NewNode> NewNodeAction(new FVoxelGraphSchemaAction_NewNode(LOCTEXT("VoxelNodeAction", "Voxel Noise Node"), Name, AddToolTip, 0));
				NewNodeAction->VoxelNodeClass = NodeClass;
				ActionMenuBuilder.AddAction(NewNodeAction);
			}
			else
			{
				TSharedPtr<FVoxelGraphSchemaAction_NewNode> NewNodeAction(new FVoxelGraphSchemaAction_NewNode(LOCTEXT("VoxelNodeAction", "Voxel Node"), Name, AddToolTip, 0));
				NewNodeAction->VoxelNodeClass = NodeClass;
				ActionMenuBuilder.AddAction(NewNodeAction);
			}
		}
	}

	const FText MenuDescription = LOCTEXT("AddKnotNoteAction", "Add reroute node...");
	const FText ToolTip = LOCTEXT("CreateKnotNodeToolTip", "Creates a reroute node.");
	TSharedPtr<FVoxelGraphSchemaAction_NewKnotNode> NewNodeAction(new FVoxelGraphSchemaAction_NewKnotNode(FText::GetEmpty(), MenuDescription, ToolTip, 0));
	ActionMenuBuilder.AddAction(NewNodeAction);
}

void UVoxelGraphSchema::GetCommentAction(FGraphActionMenuBuilder& ActionMenuBuilder, const UEdGraph* CurrentGraph /*= NULL*/) const
{
	if (!ActionMenuBuilder.FromPin)
	{
		const bool bIsManyNodesSelected = CurrentGraph ? (FVoxelEditorUtilities::GetNumberOfSelectedNodes(CurrentGraph) > 0) : false;
		const FText MenuDescription = bIsManyNodesSelected ? LOCTEXT("CreateCommentAction", "Create Comment from Selection") : LOCTEXT("AddCommentAction", "Add Comment...");
		const FText ToolTip = LOCTEXT("CreateCommentToolTip", "Creates a comment.");

		TSharedPtr<FVoxelGraphSchemaAction_NewComment> NewAction(new FVoxelGraphSchemaAction_NewComment(FText::GetEmpty(), MenuDescription, ToolTip, 0));
		ActionMenuBuilder.AddAction(NewAction);
	}

}

void UVoxelGraphSchema::InitVoxelNodeClasses()
{
	if (bVoxelNodeClassesInitialized)
	{
		return;
	}

	// Construct list of non-abstract sound node classes.
	for (TObjectIterator<UClass> It; It; ++It)
	{
		if (It->IsChildOf(UVoxelNode::StaticClass()) && !It->HasAnyClassFlags(CLASS_Abstract))
		{
			VoxelNodeClasses.Add(*It);
		}
	}

	VoxelNodeClasses.Sort();

	bVoxelNodeClassesInitialized = true;
}

#undef LOCTEXT_NAMESPACE
