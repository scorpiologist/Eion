// Copyright 2018 Phyronnaz

#include "VoxelNode.h"
#include "VoxelGraphWorldGenerator.h"

const FString FVoxelPinCategory::PC_Exec(TEXT("exec"));
const FString FVoxelPinCategory::PC_Boolean(TEXT("bool"));
const FString FVoxelPinCategory::PC_Int(TEXT("int"));
const FString FVoxelPinCategory::PC_Float(TEXT("float"));
const FString FVoxelPinCategory::PC_Material(TEXT("FVoxelMaterial"));
const FString FVoxelPinCategory::PC_ValueType(TEXT("EVoxelValueType"));
const FString FVoxelPinCategory::PC_MaterialType(TEXT("EVoxelMaterialType"));
const FString FVoxelPinCategory::PC_VoxelType(TEXT("FVoxelType"));

const EVoxelPinCategory FVoxelPinCategory::FromString(const FString& String)
{
	if (String == PC_Exec)
	{
		return EVoxelPinCategory::Exec;
	}
	else if (String == PC_Boolean)
	{
		return EVoxelPinCategory::Boolean;
	}
	else if (String == PC_Int)
	{
		return EVoxelPinCategory::Int;
	}
	else if (String == PC_Float)
	{
		return EVoxelPinCategory::Float;
	}
	else if (String == PC_Material)
	{
		return EVoxelPinCategory::Material;
	}
	else if (String == PC_ValueType)
	{
		return EVoxelPinCategory::ValueType;
	}
	else if (String == PC_MaterialType)
	{
		return EVoxelPinCategory::MaterialType;
	}
	else if (String == PC_VoxelType)
	{
		return EVoxelPinCategory::VoxelType;
	}
	else
	{
		check(false);
		return EVoxelPinCategory::Exec;
	}
}

const FString FVoxelPinCategory::ToString(EVoxelPinCategory Category)
{
	switch (Category)
	{
	case EVoxelPinCategory::Exec:
		return PC_Exec;
	case EVoxelPinCategory::Boolean:
		return PC_Boolean;
	case EVoxelPinCategory::Int:
		return PC_Int;
	case EVoxelPinCategory::Float:
		return PC_Float;
	case EVoxelPinCategory::Material:
		return PC_Material;
	case EVoxelPinCategory::ValueType:
		return PC_ValueType;
	case EVoxelPinCategory::MaterialType:
		return PC_MaterialType;
	case EVoxelPinCategory::VoxelType:
		return PC_VoxelType;
	default:
		check(false);
		return FString();
	}
}

/////////////////////////////////////////////////////////////////////

int32 UVoxelNode::GetOutputPinIndex(const FGuid& PinId)
{
	for (int i = 0; i < OutputPins.Num(); i++)
	{
		if (OutputPins[i].PinId == PinId)
		{
			return i;
		}
	}
	return -1;
}

void UVoxelNode::CreateStartingConnectors()
{
#if WITH_EDITOR
	InputPinCount = GetMinInputPins();

	// Only add input
	for (int i = 0; i < GetInputPinsCount(); i++)
	{
		UVoxelGraphGenerator::GetVoxelGraphEditor()->CreateInputPin(GraphNode);
	}
#endif //WITH_EDITOR
}

void UVoxelNode::RemoveInputPin()
{
	check(InputPinCount > 0);
	InputPinCount--;
}

void UVoxelNode::AddInputPin()
{
	InputPinCount++;
}

int32 UVoxelNode::GetInputPinsWithoutExecCount() const
{
	int32 Count = 0;
	for (auto Pin : InputPins)
	{
		if (Pin.PinCategory != EVoxelPinCategory::Exec)
		{
			Count++;
		}
	}

	return Count;
}

int32 UVoxelNode::GetOutputPinsWithoutExecCount() const
{
	int32 Count = 0;
	for (auto Pin : OutputPins)
	{
		if (Pin.PinCategory != EVoxelPinCategory::Exec)
		{
			Count++;
		}
	}

	return Count;
}

bool UVoxelNode::HasInputPinWithCategory(const FString& Category) const
{
	for (int i = 0; i < GetMinInputPins(); i++)
	{
		if (GetInputPinCategory(i) == Category)
		{
			return true;
		}
	}
	return false;
}

bool UVoxelNode::HasOutputPinWithCategory(const FString& Category) const
{
	for (int i = 0; i < GetOutputPinsCount(); i++)
	{
		if (GetOutputPinCategory(i) == Category)
		{
			return true;
		}
	}
	return false;
}

FText UVoxelNode::GetTitle() const
{
#if WITH_EDITOR
	return GetClass()->GetDisplayNameText();
#else
	return FText::GetEmpty();
#endif
}

TSharedPtr<FVoxelComputeNode> UVoxelNode::GetComputeNode() const
{
	check(false);
	return TSharedPtr<FVoxelComputeNode>();
}

#if WITH_EDITOR
void UVoxelNode::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	MarkPackageDirty();
}

void UVoxelNode::PostLoad()
{
	Super::PostLoad();
	// Make sure voxel nodes are transactional (so they work with undo system)
	SetFlags(RF_Transactional);
}
#endif

/////////////////////////////////////////////////////////////////////

FVoxelComputeNode::FVoxelComputeNode(const UVoxelNode* InNode)
	: InputCount(InNode->GetInputPinsWithoutExecCount())
	, OutputCount(InNode->GetOutputPinsWithoutExecCount())
	, Node(InNode)
{
	check(InputCount < MAX_PINS);
	check(OutputCount < MAX_PINS);

	for (int i = 0; i < Node->InputIds.Num(); i++)
	{
		InputIds[i] = Node->InputIds[i];
	}
	for (int i = 0; i < Node->OutputIds.Num(); i++)
	{
		OutputIds[i] = Node->OutputIds[i];
	}

	int k = 0;
	for (auto Pin : InNode->InputPins)
	{
		FVoxelNodeType Value;
		FString ValueString;

		switch (Pin.PinCategory)
		{
		case EVoxelPinCategory::Exec:
			// Don't consider exec pins
			continue;
		case EVoxelPinCategory::Boolean:
			Value.B = Pin.DefaultValue.ToBool();
			ValueString = Value.B ? TEXT("true") : TEXT("false");
			break;
		case EVoxelPinCategory::Int:
			Value.I = FCString::Atoi(*Pin.DefaultValue);
			ValueString = FString::FromInt(Value.I);
			break;
		case EVoxelPinCategory::Float:
			Value.F = FCString::Atof(*Pin.DefaultValue);
			ValueString = FString::SanitizeFloat(Value.F);
			break;
		case EVoxelPinCategory::Material:
			Value.M = FVoxelMaterial_internal(0, 0, 0, 0);
			ValueString = TEXT("FVoxelMaterial(0, 0, 0, 0)");
			break;
		case EVoxelPinCategory::ValueType:
		{
			if (Pin.DefaultValue == "IgnoreValue")
			{
				Value.VVT = EVoxelValueType::IgnoreValue;
				ValueString = TEXT("EVoxelValueType::IgnoreValue");
			}
			else if (Pin.DefaultValue == "UseValueIfSameSign")
			{
				Value.VVT = EVoxelValueType::UseValueIfSameSign;
				ValueString = TEXT("EVoxelValueType::UseValueIfSameSign");
			}
			else if (Pin.DefaultValue == "UseValue")
			{
				Value.VVT = EVoxelValueType::UseValue;
				ValueString = TEXT("EVoxelValueType::UseValue");
			}
			else
			{
				Value.VVT = EVoxelValueType::UseValue;
				ValueString = TEXT("EVoxelValueType::UseValue");
			}
			break;
		}
		case EVoxelPinCategory::MaterialType:
		{
			if (Pin.DefaultValue == "UseMaterial")
			{
				Value.VMT = EVoxelMaterialType::UseMaterial;
				ValueString = TEXT("EVoxelMaterialType::UseMaterial");
			}
			else if (Pin.DefaultValue == "IgnoreMaterial")
			{
				Value.VMT = EVoxelMaterialType::IgnoreMaterial;
				ValueString = TEXT("EVoxelMaterialType::IgnoreMaterial");
			}
			else
			{				
				Value.VMT = EVoxelMaterialType::UseMaterial;
				ValueString = TEXT("EVoxelMaterialType::UseMaterial");
			}
			break;
		}
		case EVoxelPinCategory::VoxelType:
			Value.VT = FVoxelType::UseAll();
			ValueString = TEXT("FVoxelType::UseAll()");
			break;
		default:
			Value.B = false;
			check(false);
		}

		DefaultValues[k] = Value;
		DefaultValueStrings[k] = ValueString;
		k++;
	}
	int l = 0;
	for (auto Pin : InNode->OutputPins)
	{
		OutputType[l] = FVoxelPinCategory::ToString(Pin.PinCategory);
		l++;
	}
}
