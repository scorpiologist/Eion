// Copyright 2018 Phyronnaz

#include "SGraphNodeVoxel.h"
#include "Widgets/SBoxPanel.h"
#include "VoxelGraphNode.h"
#include "ScopedTransaction.h"
#include "GraphEditorSettings.h"
#include "IDocumentation.h"
#include "TutorialMetaData.h"
#include "SImage.h"
#include "SCommentBubble.h"
#include "SToolTip.h"
#include "SBox.h"
#include "SGraphPin.h"

void SGraphNodeVoxel::Construct(const FArguments& InArgs, class UVoxelGraphNode_Base* InNode)
{
	this->GraphNode = InNode;
	this->VoxelNode = InNode;

	this->SetCursor(EMouseCursor::CardinalCross);

	this->UpdateGraphNode();
}

void SGraphNodeVoxel::UpdateGraphNode()
{
	if (CastChecked<UVoxelGraphNode_Base>(GraphNode)->IsCompact())
	{
		UpdateCompactNode();
	}
	else
	{
		UpdateStandardNode();
	}
}

void SGraphNodeVoxel::CreateOutputSideAddButton(TSharedPtr<SVerticalBox> OutputBox)
{
	TSharedRef<SWidget> AddPinButton = AddPinButtonContent(
		NSLOCTEXT("VoxelNode", "VoxelNodeAddPinButton", "Add input"),
		NSLOCTEXT("VoxelNode", "VoxelNodeAddPinButton_Tooltip", "Adds an input to the Voxel node")
	);

	FMargin AddPinPadding = Settings->GetOutputPinPadding();
	AddPinPadding.Top += 6.0f;

	OutputBox->AddSlot()
		.AutoHeight()
		.VAlign(VAlign_Center)
		.Padding(AddPinPadding)
		[
			AddPinButton
		];
}

EVisibility SGraphNodeVoxel::IsAddPinButtonVisible() const
{
	EVisibility ButtonVisibility = SGraphNode::IsAddPinButtonVisible();
	if (ButtonVisibility == EVisibility::Visible)
	{
		if (!VoxelNode->CanAddInputPin())
		{
			ButtonVisibility = EVisibility::Collapsed;
		}
	}
	return ButtonVisibility;
}

FReply SGraphNodeVoxel::OnAddPin()
{
	VoxelNode->AddInputPin();

	return FReply::Handled();
}

void SGraphNodeVoxel::UpdateStandardNode()
{
	SGraphNode::UpdateGraphNode();
	// clear the default tooltip, to make room for our custom "complex" tooltip
	SetToolTip(NULL);
}

void SGraphNodeVoxel::UpdateCompactNode()
{
	InputPins.Empty();
	OutputPins.Empty();

	// error handling set-up
	SetupErrorReporting();

	// Reset variables that are going to be exposed, in case we are refreshing an already setup node.
	RightNodeBox.Reset();
	LeftNodeBox.Reset();

	TSharedPtr< SToolTip > NodeToolTip = SNew(SToolTip);
	if (!GraphNode->GetTooltipText().IsEmpty())
	{
		NodeToolTip = IDocumentation::Get()->CreateToolTip(TAttribute< FText >(this, &SGraphNode::GetNodeTooltip), NULL, GraphNode->GetDocumentationLink(), GraphNode->GetDocumentationExcerptName());
	}

	// Setup a meta tag for this node
	FGraphNodeMetaData TagMeta(TEXT("Graphnode"));
	PopulateMetaTag(&TagMeta);

	TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode).Text(this, &SGraphNodeVoxel::GetNodeCompactTitle);

	TSharedRef<SOverlay> NodeOverlay = SNew(SOverlay);

	// add optional node specific widget to the overlay:
	TSharedPtr<SWidget> OverlayWidget = GraphNode->CreateNodeImage();
	if (OverlayWidget.IsValid())
	{
		NodeOverlay->AddSlot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.WidthOverride(70.f)
			.HeightOverride(70.f)
			[
				OverlayWidget.ToSharedRef()
			]
			];
	}

	NodeOverlay->AddSlot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(45.f, 0.f, 45.f, 0.f)
		[
			// MIDDLE
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
		.HAlign(HAlign_Center)
		.AutoHeight()
		[
			SNew(STextBlock)
			.TextStyle(FEditorStyle::Get(), "Graph.CompactNode.Title")
		.Text(NodeTitle.Get(), &SNodeTitle::GetHeadTitle)
		.WrapTextAt(128.0f)
		]
	+ SVerticalBox::Slot()
		.AutoHeight()
		[
			NodeTitle.ToSharedRef()
		]
		];

	NodeOverlay->AddSlot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.Padding(0.f, 0.f, 55.f, 0.f)
		[
			// LEFT
			SAssignNew(LeftNodeBox, SVerticalBox)
		];

	NodeOverlay->AddSlot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Center)
		.Padding(55.f, 0.f, 0.f, 0.f)
		[
			// RIGHT
			SAssignNew(RightNodeBox, SVerticalBox)
		];

	//
	//             ______________________
	//            | (>) L |      | R (>) |
	//            | (>) E |      | I (>) |
	//            | (>) F |   +  | G (>) |
	//            | (>) T |      | H (>) |
	//            |       |      | T (>) |
	//            |_______|______|_______|
	//
	this->ContentScale.Bind(this, &SGraphNode::GetContentScale);
	this->GetOrAddSlot(ENodeZone::Center)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
		[
			// NODE CONTENT AREA
			SNew(SOverlay)
			+ SOverlay::Slot()
		[
			SNew(SImage)
			.Image(FEditorStyle::GetBrush("Graph.VarNode.Body"))
		]
	+ SOverlay::Slot()
		[
			SNew(SImage)
			.Image(FEditorStyle::GetBrush("Graph.VarNode.Gloss"))
		]
	+ SOverlay::Slot()
		.Padding(FMargin(0, 3))
		[
			NodeOverlay
		]
		]
	+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(5.0f, 1.0f))
		[
			ErrorReporting->AsWidget()
		]
		];

	CreatePinWidgets();

	// Hide pin labels
	for (auto InputPin : InputPins)
	{
		if (InputPin->GetPinObj()->ParentPin == nullptr)
		{
			InputPin->SetShowLabel(false);
		}
	}

	for (auto OutputPin : OutputPins)
	{
		if (OutputPin->GetPinObj()->ParentPin == nullptr)
		{
			OutputPin->SetShowLabel(false);
		}
	}

	// Create comment bubble
	TSharedPtr<SCommentBubble> CommentBubble;
	const FSlateColor CommentColor = GetDefault<UGraphEditorSettings>()->DefaultCommentNodeTitleColor;

	SAssignNew(CommentBubble, SCommentBubble)
		.GraphNode(GraphNode)
		.Text(this, &SGraphNode::GetNodeComment)
		.OnTextCommitted(this, &SGraphNode::OnCommentTextCommitted)
		.ColorAndOpacity(CommentColor)
		.AllowPinning(true)
		.EnableTitleBarBubble(true)
		.EnableBubbleCtrls(true)
		.GraphLOD(this, &SGraphNode::GetCurrentLOD)
		.IsGraphNodeHovered(this, &SGraphNodeVoxel::IsHovered);

	GetOrAddSlot(ENodeZone::TopCenter)
		.SlotOffset(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetOffset))
		.SlotSize(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetSize))
		.AllowScaling(TAttribute<bool>(CommentBubble.Get(), &SCommentBubble::IsScalingAllowed))
		.VAlign(VAlign_Top)
		[
			CommentBubble.ToSharedRef()
		];

	CreateInputSideAddButton(LeftNodeBox);
	CreateOutputSideAddButton(RightNodeBox);
}

FText SGraphNodeVoxel::GetNodeCompactTitle() const
{
	return GraphNode->GetNodeTitle(ENodeTitleType::FullTitle);
}
