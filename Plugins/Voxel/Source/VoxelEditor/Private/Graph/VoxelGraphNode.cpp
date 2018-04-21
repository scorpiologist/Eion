// Copyright 2018 Phyronnaz

#include "VoxelGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraph/EdGraphNode.h"
#include "ScopedTransaction.h"
#include "Engine/Font.h"
#include "Editor/EditorEngine.h"
#include "GenericCommands.h"

#include "VoxelGraph.h"
#include "VoxelEditor.h"
#include "MultiBoxBuilder.h"
#include "Runtime/Launch/Resources/Version.h"


void UVoxelGraphNode::SetVoxelNode(UVoxelNode* InNode)
{
	VoxelNode = InNode;
}

void UVoxelGraphNode::PostCopyNode()
{
	// Make sure the SoundNode goes back to being owned by the SoundCue after copying.
	ResetVoxelNodeOwner();
}

void UVoxelGraphNode::CreateInputPin()
{
	UEdGraphPin* NewPin = CreatePin(EGPD_Input, VoxelNode->GetInputPinCategory(GetInputCount()), FString(), nullptr, VoxelNode->GetInputPinName(GetInputCount()).ToString());
#if ENGINE_MINOR_VERSION < 19
	if (NewPin->PinName.IsEmpty())
#else
	if (NewPin->PinName.ToString().IsEmpty())
#endif
	{
		// Makes sure pin has a name for lookup purposes but user will never see it
		NewPin->PinName = CreateUniquePinName(TEXT("Input"));
		NewPin->PinFriendlyName = FText::FromString(TEXT(" "));
	}
}

void UVoxelGraphNode::CreateOutputPin()
{
	UEdGraphPin* NewPin = CreatePin(EGPD_Output, VoxelNode->GetOutputPinCategory(GetOutputCount()), FString(), nullptr, VoxelNode->GetOutputPinName(GetOutputCount()).ToString());
#if ENGINE_MINOR_VERSION < 19
	if (NewPin->PinName.IsEmpty())
#else
	if (NewPin->PinName.ToString().IsEmpty())
#endif
	{
		// Makes sure pin has a name for lookup purposes but user will never see it
		NewPin->PinName = CreateUniquePinName(TEXT("Output"));
		NewPin->PinFriendlyName = FText::FromString(TEXT(" "));
	}
}

void UVoxelGraphNode::AddInputPin()
{
	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "VoxelEditorAddInput", "Add Voxel Input"));
	Modify();
	VoxelNode->AddInputPin();
	CreateInputPin();

	UVoxelGraphGenerator* Generator = CastChecked<UVoxelGraph>(GetGraph())->GetWorldGenerator();
	Generator->CompileVoxelNodesFromGraphNodes();
	Generator->MarkPackageDirty();

	// Refresh the current graph, so the pins can be updated
	GetGraph()->NotifyGraphChanged();
}

void UVoxelGraphNode::RemoveInputPin(UEdGraphPin* InGraphPin)
{
	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "VoxelEditorAddInput", "Delete Voxel Input"));
	Modify();

	TArray<UEdGraphPin*> InputPins;
	GetInputPins(InputPins);

	for (int32 InputIndex = 0; InputIndex < InputPins.Num(); InputIndex++)
	{
		if (InGraphPin == InputPins[InputIndex])
		{
			InGraphPin->MarkPendingKill();
			Pins.Remove(InGraphPin);
			// also remove the SoundNode child node so ordering matches
			VoxelNode->Modify();
			VoxelNode->RemoveInputPin();
			break;
		}
	}

	UVoxelGraphGenerator* Generator = CastChecked<UVoxelGraph>(GetGraph())->GetWorldGenerator();
	Generator->CompileVoxelNodesFromGraphNodes();
	Generator->MarkPackageDirty();

	// Refresh the current graph, so the pins can be updated
	GetGraph()->NotifyGraphChanged();
}

int32 UVoxelGraphNode::EstimateNodeWidth() const
{
	const int32 EstimatedCharWidth = 6;
	FString NodeTitle = GetNodeTitle(ENodeTitleType::FullTitle).ToString();
	UFont* Font = GetDefault<UEditorEngine>()->EditorFont;
	int32 Result = NodeTitle.Len()*EstimatedCharWidth;

	if (Font)
	{
		Result = Font->GetStringSize(*NodeTitle);
	}

	return Result;
}

bool UVoxelGraphNode::CanAddInputPin() const
{
	if (VoxelNode)
	{
		// Check if adding another input would exceed max child nodes.
		return GetInputCount() < VoxelNode->GetMaxInputPins();
	}
	else
	{
		return false;
	}
}

bool UVoxelGraphNode::IsCompact() const
{
	return VoxelNode && VoxelNode->IsCompact();
}

void UVoxelGraphNode::CreateInputPins()
{
	if (VoxelNode)
	{
		while (GetInputCount() < VoxelNode->GetInputPinsCount())
		{
			CreateInputPin();
		}
	}
}

void UVoxelGraphNode::CreateOutputPins()
{
	if (VoxelNode)
	{
		while (GetOutputCount() < VoxelNode->GetOutputPinsCount())
		{
			CreateOutputPin();
		}
	}
}

FText UVoxelGraphNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (VoxelNode)
	{
		return VoxelNode->GetTitle();
	}
	else
	{
		return Super::GetNodeTitle(TitleType);
	}
}

FLinearColor UVoxelGraphNode::GetNodeTitleColor() const
{
	if (VoxelNode)
	{
		return VoxelNode->GetColor();
	}
	else
	{
		return FLinearColor::White;
	}
}

void UVoxelGraphNode::PrepareForCopying()
{
	if (VoxelNode)
	{
		// Temporarily take ownership of the SoundNode, so that it is not deleted when cutting
		VoxelNode->Rename(NULL, this, REN_DontCreateRedirectors);
	}
}

void UVoxelGraphNode::GetContextMenuActions(const FGraphNodeContextMenuBuilder& Context) const
{
	if (Context.Pin)
	{
		// If on an input that can be deleted, show option
		if (Context.Pin->Direction == EGPD_Input && VoxelNode->GetInputPinsCount() > VoxelNode->GetMinInputPins())
		{
			Context.MenuBuilder->AddMenuEntry(FVoxelGraphEditorCommands::Get().DeleteInput);
		}
	}
	else if (Context.Node)
	{
		Context.MenuBuilder->BeginSection("VoxelGraphNodeEdit");
		{
			Context.MenuBuilder->AddMenuEntry(FGenericCommands::Get().Delete);
			Context.MenuBuilder->AddMenuEntry(FGenericCommands::Get().Cut);
			Context.MenuBuilder->AddMenuEntry(FGenericCommands::Get().Copy);
			Context.MenuBuilder->AddMenuEntry(FGenericCommands::Get().Duplicate);
		}
		Context.MenuBuilder->EndSection();
	}
}

FText UVoxelGraphNode::GetTooltipText() const
{
	FText Tooltip;
	if (VoxelNode)
	{
		Tooltip = VoxelNode->GetClass()->GetToolTipText();
	}
	if (Tooltip.IsEmpty())
	{
		Tooltip = GetNodeTitle(ENodeTitleType::ListView);
	}
	return Tooltip;
}

FString UVoxelGraphNode::GetDocumentationExcerptName() const
{
	// Default the node to searching for an excerpt named for the C++ node class name, including the U prefix.
	// This is done so that the excerpt name in the doc file can be found by find-in-files when searching for the full class name.
	UClass* MyClass = (VoxelNode != NULL) ? VoxelNode->GetClass() : this->GetClass();
	return FString::Printf(TEXT("%s%s"), MyClass->GetPrefixCPP(), *MyClass->GetName());
}

void UVoxelGraphNode::PostLoad()
{
	Super::PostLoad();

	// Fixup any SoundNode back pointers that may be out of date
	if (VoxelNode)
	{
		VoxelNode->GraphNode = this;
	}

	for (int32 Index = 0; Index < Pins.Num(); ++Index)
	{
		UEdGraphPin* Pin = Pins[Index];
#if ENGINE_MINOR_VERSION < 19
		if (Pin->PinName.IsEmpty())
#else
		if (Pin->PinName.ToString().IsEmpty())
#endif
		{
			// Makes sure pin has a name for lookup purposes but user will never see it
			if (Pin->Direction == EGPD_Input)
			{
				Pin->PinName = CreateUniquePinName(TEXT("Input"));
			}
			else
			{
				Pin->PinName = CreateUniquePinName(TEXT("Output"));
			}
			Pin->PinFriendlyName = FText::FromString(TEXT(" "));
		}
	}
}

void UVoxelGraphNode::PostEditImport()
{
	// Make sure this SoundNode is owned by the SoundCue it's being pasted into.
	ResetVoxelNodeOwner();
}

void UVoxelGraphNode::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);

	if (!bDuplicateForPIE)
	{
		CreateNewGuid();
	}
}

void UVoxelGraphNode::ResetVoxelNodeOwner()
{
	if (VoxelNode)
	{
		UVoxelGraphGenerator* Generator = CastChecked<UVoxelGraph>(GetGraph())->GetWorldGenerator();

		if (VoxelNode->GetOuter() != Generator)
		{
			// Ensures SoundNode is owned by the SoundCue
			VoxelNode->Rename(NULL, Generator, REN_DontCreateRedirectors);
		}

		// Set up the back pointer for newly created sound nodes
		VoxelNode->GraphNode = this;
	}
}
