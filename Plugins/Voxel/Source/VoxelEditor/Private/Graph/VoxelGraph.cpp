// Copyright 2018 Phyronnaz

#include "VoxelGraph.h"
#include "EdGraph/EdGraphPin.h"
#include "ScopedTransaction.h"
#include "Engine/Font.h"
#include "Editor/EditorEngine.h"
#include "BlueprintEditorUtils.h"
#include "VoxelGraphSchema.h"
#include "VoxelEditor.h"
#include "EdGraph/EdGraphNode.h"
#include "GenericCommands.h"
#include "VoxelNode.h"
#include "VoxelGraphNode.h"
#include "VoxelGraphNode_Root.h"
#include "VoxelGraphNode_Knot.h"
#include "Runtime/Launch/Resources/Version.h"

UVoxelGraph::UVoxelGraph(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (!UVoxelGraphGenerator::GetVoxelGraphEditor().IsValid())
	{
		UVoxelGraphGenerator::SetVoxelGraphEditor(TSharedPtr<IVoxelGraphEditor>(new FVoxelGraphEditor()));
	}
}

/////////////////////////////////////////////////////////////////////////

UEdGraph* FVoxelGraphEditor::CreateNewVoxelGraph(UVoxelGraphGenerator* InSoundCue)
{
	UVoxelGraph* VoxelGraph = CastChecked<UVoxelGraph>(FBlueprintEditorUtils::CreateNewGraph(InSoundCue, NAME_None, UVoxelGraph::StaticClass(), UVoxelGraphSchema::StaticClass()));

	return VoxelGraph;
}

void FVoxelGraphEditor::SetupVoxelNode(UEdGraph* SoundCueGraph, UVoxelNode* InSoundNode, bool bSelectNewNode)
{
	FGraphNodeCreator<UVoxelGraphNode> NodeCreator(*SoundCueGraph);
	UVoxelGraphNode* GraphNode = NodeCreator.CreateNode(bSelectNewNode);
	InSoundNode->GraphNode = GraphNode;
	GraphNode->SetVoxelNode(InSoundNode);
	NodeCreator.Finalize();
}

void FVoxelGraphEditor::CompileVoxelNodesFromGraphNodes(UVoxelGraphGenerator* WorldGenerator)
{
	WorldGenerator->Modify();
	WorldGenerator->FirstNode = nullptr;
	WorldGenerator->AllNodes.Empty();
	WorldGenerator->PostEditChange();

	TArray<UVoxelNode*> AllNodes;
	for (auto Node : WorldGenerator->VoxelGraph->Nodes)
	{
		UVoxelGraphNode* GraphNode = Cast<UVoxelGraphNode>(Node);
		if (GraphNode && GraphNode->VoxelNode)
		{
			AllNodes.Add(GraphNode->VoxelNode);

			TArray<FVoxelPin> InputVoxelPins;
			{
				TArray<UEdGraphPin*> InputPins;
				GraphNode->GetInputPins(InputPins);
				for (auto InputPin : InputPins)
				{
					FVoxelPin NewPin;

#if ENGINE_MINOR_VERSION < 19
					FString InputPinCategory = InputPin->PinType.PinCategory;
#else
					FString InputPinCategory = InputPin->PinType.PinCategory.ToString();
#endif

					if (InputPinCategory == FVoxelPinCategory::PC_Exec)
					{
						// Only outputs other node are used for exec pins
						NewPin = FVoxelPin(InputPin->PinId, InputPinCategory);
					}
					else
					{
						check(InputPin->LinkedTo.Num() <= 1);
						if (InputPin->LinkedTo.Num() > 0 && InputPin->LinkedTo[0])
						{
							auto Knot = Cast<UVoxelGraphNode_Knot>(InputPin->LinkedTo[0]->GetOwningNode());
							if (Knot)
							{
								auto NewInputPin = Knot->GetRealInputPin();
								if (NewInputPin)
								{
									auto GraphChildNode = CastChecked<UVoxelGraphNode>(NewInputPin->GetOwningNode());
									NewPin = FVoxelPin(InputPin->PinId, InputPinCategory, GraphChildNode->VoxelNode, NewInputPin->PinId, InputPin->DefaultValue);
								}
								else
								{
									// If we are linked to empty knots
									NewPin = FVoxelPin(InputPin->PinId, InputPinCategory, nullptr, FGuid(), InputPin->DefaultValue);
								}
							}
							else
							{
								auto GraphChildNode = Cast<UVoxelGraphNode>(InputPin->LinkedTo[0]->GetOwningNode());
								// If we are linked to a child that is not Start (not exec) and not a knot, then it must be a valid node
								check(GraphChildNode);

								NewPin = FVoxelPin(InputPin->PinId, InputPinCategory, GraphChildNode->VoxelNode, InputPin->LinkedTo[0]->PinId, InputPin->DefaultValue);
							}
						}
						else
						{
							NewPin = FVoxelPin(InputPin->PinId, InputPinCategory, nullptr, FGuid(), InputPin->DefaultValue);
						}
					}

					InputVoxelPins.Add(NewPin);
				}
			}

			TArray<FVoxelPin> OutputVoxelPins;
			{
				TArray<UEdGraphPin*> OutputPins;
				GraphNode->GetOutputPins(OutputPins);
				for (auto OutputPin : OutputPins)
				{
					FVoxelPin NewPin;
					
#if ENGINE_MINOR_VERSION < 19
					FString OutputPinCategory = OutputPin->PinType.PinCategory;
#else
					FString OutputPinCategory = OutputPin->PinType.PinCategory.ToString();
#endif

					if (OutputPinCategory != FVoxelPinCategory::PC_Exec)
					{
						// Only inputs other node are used for non exec pins
						NewPin = FVoxelPin(OutputPin->PinId, OutputPinCategory);
					}
					else
					{
						check(OutputPin->LinkedTo.Num() <= 1);
						if (OutputPin->LinkedTo.Num() > 0 && OutputPin->LinkedTo[0])
						{
							auto Knot = Cast<UVoxelGraphNode_Knot>(OutputPin->LinkedTo[0]->GetOwningNode());
							if (Knot)
							{
								auto NewOuputPin = Knot->GetRealOutputPin();
								if (NewOuputPin)
								{
									auto GraphChildNode = CastChecked<UVoxelGraphNode>(NewOuputPin->GetOwningNode());
									NewPin = FVoxelPin(OutputPin->PinId, OutputPinCategory, GraphChildNode->VoxelNode, NewOuputPin->PinId, OutputPin->DefaultValue);
								}
								else
								{
									// If we are linked to empty knots
									NewPin = FVoxelPin(OutputPin->PinId, OutputPinCategory, nullptr, FGuid(), OutputPin->DefaultValue);
								}
							}
							else
							{
								auto GraphChildNode = Cast<UVoxelGraphNode>(OutputPin->LinkedTo[0]->GetOwningNode());
								// If we are linked to a child that is not Start (not exec) and not a knot, then it must be a valid node
								check(GraphChildNode);

								NewPin = FVoxelPin(OutputPin->PinId, OutputPinCategory, GraphChildNode->VoxelNode, OutputPin->LinkedTo[0]->PinId, OutputPin->DefaultValue);
							}
						}
						else
						{
							NewPin = FVoxelPin(OutputPin->PinId, OutputPinCategory, nullptr, FGuid(), OutputPin->DefaultValue);
						}
					}
					OutputVoxelPins.Add(NewPin);
				}
			}

			GraphNode->VoxelNode->SetFlags(RF_Transactional);
			GraphNode->VoxelNode->Modify();
			GraphNode->VoxelNode->InputPins = InputVoxelPins;
			GraphNode->VoxelNode->OutputPins = OutputVoxelPins;
			GraphNode->VoxelNode->PostEditChange();
		}
		else
		{
			UVoxelGraphNode_Root* GraphNodeRoot = Cast<UVoxelGraphNode_Root>(Node);
			if (GraphNodeRoot)
			{
				TArray<UEdGraphPin*> OutputPins;
				GraphNodeRoot->GetOutputPins(OutputPins);

				check(OutputPins.Num() == 1);
				check(OutputPins[0]->LinkedTo.Num() <= 1);
				if (OutputPins[0]->LinkedTo.Num() == 1)
				{
					auto Knot = Cast<UVoxelGraphNode_Knot>(OutputPins[0]->LinkedTo[0]->GetOwningNode());

					if (Knot)
					{
						auto NewOutputPin = Knot->GetRealOutputPin();
						if (NewOutputPin)
						{
							WorldGenerator->FirstNode = CastChecked<UVoxelGraphNode>(NewOutputPin->GetOwningNode())->VoxelNode;
						}
					}
					else
					{
						WorldGenerator->FirstNode = CastChecked<UVoxelGraphNode>(OutputPins[0]->LinkedTo[0]->GetOwningNode())->VoxelNode;
					}
				}
			}
		}
	}

	WorldGenerator->Modify();
	WorldGenerator->AllNodes = AllNodes;
	WorldGenerator->PostEditChange();
}

void FVoxelGraphEditor::CreateInputPin(UEdGraphNode* VoxelNode)
{
	CastChecked<UVoxelGraphNode>(VoxelNode)->CreateInputPin();
}

void FVoxelGraphEditor::CreateOutputPin(UEdGraphNode* VoxelNode)
{
	CastChecked<UVoxelGraphNode>(VoxelNode)->CreateOutputPin();
}
