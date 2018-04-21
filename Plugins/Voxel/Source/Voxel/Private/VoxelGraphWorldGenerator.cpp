// Copyright 2018 Phyronnaz

#include "VoxelGraphWorldGenerator.h"
#include "EdGraph/EdGraphSchema.h"
#include "VoxelPrivate.h"
#include "VoxelWorldGenerators/EmptyWorldGenerator.h"
#include "VoxelDefaultNodes.h"


FVoxelComputeNodeTree::FVoxelComputeNodeTree()
	: NodesCount(0)
	, ChildsCount(0)
	, Nodes(nullptr)
{

}

FVoxelComputeNodeTree::~FVoxelComputeNodeTree()
{
	for (int i = 0; i < ChildsCount; i++)
	{
		delete Childs[i];
	}
	if (Nodes)
	{
		delete[] Nodes;
	}
}

FVoxelComputeNodeTree* FVoxelComputeNodeTree::AddChild()
{
	Childs[ChildsCount] = new FVoxelComputeNodeTree();
	ChildsCount++;
	return Childs[ChildsCount - 1];
}

void FVoxelComputeNodeTree::SetNodes(const TArray<TSharedPtr<FVoxelComputeNode>>& InNodes)
{
	NodesCount = InNodes.Num();
	Nodes = new TSharedPtr<FVoxelComputeNode>[NodesCount];

	for (int i = 0; i < NodesCount; i++)
	{
		Nodes[i] = InNodes[i];
	}
}

void FVoxelComputeNodeTree::Compute(FVoxelNodeType Variables[], const FVoxelContext& Context, float& Value, FVoxelMaterial& Material, FVoxelType& VoxelType) const
{
	FVoxelNodeType Inputs[MAX_PINS];
	FVoxelNodeType Outputs[MAX_PINS];

	for (int NodeIndex = 0; NodeIndex < NodesCount - (ChildsCount ? 1 : 0); NodeIndex++)
	{
		auto& Node = Nodes[NodeIndex];
		for (int InputIndex = 0; InputIndex < Node->InputCount; InputIndex++)
		{
			int32 Id = Node->GetInputId(InputIndex);
			if (Id == -1)
			{
				Inputs[InputIndex] = Node->GetDefaultValue(InputIndex);
			}
			else
			{
				Inputs[InputIndex] = Variables[Id];
			}
		}
		if (Node->IsSetValueNode())
		{
			Value = static_cast<FVoxelComputeNode_SetValue*>(Node.Get())->bDisableClamp ? Inputs[0].F : FMath::Clamp(Inputs[0].F, -1.f, 1.f);
		}
		else if (Node->IsSetMaterialNode())
		{
			Material = Inputs[0].M;
		}
		else if (Node->IsSetVoxelTypeNode())
		{
			VoxelType = Inputs[0].VT;
		}
		else
		{
			Node->Compute(Inputs, Outputs, Context);
			for (int OutputIndex = 0; OutputIndex < Node->OutputCount; OutputIndex++)
			{
				Variables[Node->GetOutputId(OutputIndex)] = Outputs[OutputIndex];
			}
		}
	}

	if (ChildsCount)
	{
		auto& Node = Nodes[NodesCount - 1];
		for (int InputIndex = 0; InputIndex < Node->InputCount; InputIndex++)
		{
			int32 Id = Node->GetInputId(InputIndex);
			if (Id == -1)
			{
				Inputs[InputIndex] = Node->GetDefaultValue(InputIndex);
			}
			else
			{
				Inputs[InputIndex] = Variables[Id];
			}
		}
		
		if (Node->IsSetValueNode())
		{
			Value = static_cast<FVoxelComputeNode_SetValue*>(Node.Get())->bDisableClamp ? Inputs[0].F : FMath::Clamp(Inputs[0].F, -1.f, 1.f);
			Childs[0]->Compute(Variables, Context, Value, Material, VoxelType);
		}
		else if (Node->IsSetMaterialNode())
		{
			Material = Inputs[0].M;
			Childs[0]->Compute(Variables, Context, Value, Material, VoxelType);
		}
		else if (Node->IsSetVoxelTypeNode())
		{
			VoxelType = Inputs[0].VT;
			Childs[0]->Compute(Variables, Context, Value, Material, VoxelType);
		}
		else
		{
			int32 BranchId = Node->GetBranchResult(Inputs);
			check(BranchId >= 0);
			if (BranchId < ChildsCount)
			{
				Childs[BranchId]->Compute(Variables, Context, Value, Material, VoxelType);
			}
		}
	}
}

void FVoxelComputeNodeTree::Init(const AVoxelWorld* VoxelWorld) const
{
	for (int i = 0; i < NodesCount; i++)
	{
		Nodes[i]->Init(VoxelWorld);
	}

	for (int i = 0; i < ChildsCount; i++)
	{
		Childs[i]->Init(VoxelWorld);
	}
}

void FVoxelComputeNodeTree::GetMain(const TArray<FString>& Variables, const FString& Context, const FString& Value, const FString& Material, const FString& VoxelType, FString& OutCpp)
{
	TArray<FString> Inputs;
	TArray<FString> Outputs;
	Inputs.SetNum(MAX_PINS);
	Outputs.SetNum(MAX_PINS);

	for (int NodeIndex = 0; NodeIndex < NodesCount - (ChildsCount ? 1 : 0); NodeIndex++)
	{
		auto& Node = Nodes[NodeIndex];
		for (int InputIndex = 0; InputIndex < Node->InputCount; InputIndex++)
		{
			int32 Id = Node->GetInputId(InputIndex);
			if (Id == -1)
			{
				Inputs[InputIndex] = Node->GetDefaultValueString(InputIndex);
			}
			else
			{
				Inputs[InputIndex] = Variables[Id];
			}
		}
		if (Node->IsSetValueNode())
		{
			if (static_cast<FVoxelComputeNode_SetValue*>(Node.Get())->bDisableClamp)
			{
				OutCpp.Append(Value + TEXT(" = ") + Inputs[0] + TEXT(";"));
			}
			else
			{
				OutCpp.Append(Value + TEXT(" = FMath::Clamp<float>(") + Inputs[0] + TEXT(", -1.f, 1.f);"));
			}
		}
		else if (Node->IsSetMaterialNode())
		{
			OutCpp.Append(Material + TEXT(" = ") + Inputs[0] + TEXT(";"));
		}
		else if (Node->IsSetVoxelTypeNode())
		{
			OutCpp.Append(VoxelType + TEXT(" = ") + Inputs[0] + TEXT(";"));
		}
		else
		{
			for (int OutputIndex = 0; OutputIndex < Node->OutputCount; OutputIndex++)
			{
				FString Name = Variables[Node->GetOutputId(OutputIndex)];
				FString Type = Node->GetOutputType(OutputIndex);
				Outputs[OutputIndex] = Name;
				OutCpp.Append(Type + TEXT(" ") + Name + TEXT(";\n"));
			}

			Node->GetMain(Inputs, Outputs, Context, OutCpp);
			OutCpp.Append(TEXT("\n"));
		}
	}

	if (ChildsCount)
	{
		auto& Node = Nodes[NodesCount - 1];
		for (int InputIndex = 0; InputIndex < Node->InputCount; InputIndex++)
		{
			int32 Id = Node->GetInputId(InputIndex);
			if (Id == -1)
			{
				Inputs[InputIndex] = Node->GetDefaultValueString(InputIndex);
			}
			else
			{
				Inputs[InputIndex] = Variables[Id];
			}
		}

		if (Node->IsSetValueNode())
		{
			if (static_cast<FVoxelComputeNode_SetValue*>(Node.Get())->bDisableClamp)
			{
				OutCpp.Append(Value + TEXT(" = ") + Inputs[0] + TEXT(";"));
			}
			else
			{
				OutCpp.Append(Value + TEXT(" = FMath::Clamp<float>(") + Inputs[0] + TEXT(", -1.f, 1.f);"));
			}
			Childs[0]->GetMain(Variables, Context, Value, Material, VoxelType, OutCpp);
		}
		else if (Node->IsSetMaterialNode())
		{
			OutCpp.Append(Material + TEXT(" = ") + Inputs[0] + TEXT(";"));
			Childs[0]->GetMain(Variables, Context, Value, Material, VoxelType, OutCpp);
		}
		else if (Node->IsSetVoxelTypeNode())
		{
			OutCpp.Append(VoxelType + TEXT(" = ") + Inputs[0] + TEXT(";"));
			Childs[0]->GetMain(Variables, Context, Value, Material, VoxelType, OutCpp);
		}
		else
		{
			for (int OutputIndex = 0; OutputIndex < Node->OutputCount; OutputIndex++)
			{
				Outputs[OutputIndex] = Variables[Node->GetOutputId(OutputIndex)];
			}

			OutCpp.Append(TEXT("\nswitch("));
			OutCpp.Append(Node->GetBranchResultCpp(Inputs));
			OutCpp.Append(TEXT(")\n"));
			OutCpp.Append(TEXT("{\n"));
			for (int i = 0; i < ChildsCount; i++)
			{
				OutCpp.Append(TEXT("case ") + FString::FromInt(i) + TEXT(":\n"));
				OutCpp.Append(TEXT("{\n"));
				Childs[i]->GetMain(Variables, Context, Value, Material, VoxelType, OutCpp);
				OutCpp.Append(TEXT("\nbreak;\n"));
				OutCpp.Append(TEXT("}\n"));
			}
			OutCpp.Append(TEXT("}\n"));
		}
	}
}


void FVoxelComputeNodeTree::GetAdditionalHeaders(TArray<FString>& OutAdditionalHeaders) const
{
	for (int i = 0; i < NodesCount; i++)
	{
		Nodes[i]->GetAdditionalHeaders(OutAdditionalHeaders);
	}

	for (int i = 0; i < ChildsCount; i++)
	{
		Childs[i]->GetAdditionalHeaders(OutAdditionalHeaders);
	}
}

void FVoxelComputeNodeTree::GetExposedVariables(TArray<FVoxelExposedVariable>& Variables) const
{
	for (int i = 0; i < NodesCount; i++)
	{
		Nodes[i]->GetExposedVariables(Variables);
	}

	for (int i = 0; i < ChildsCount; i++)
	{
		Childs[i]->GetExposedVariables(Variables);
	}
}

void FVoxelComputeNodeTree::GetVariables(TArray<FVoxelVariable>& Variables) const
{
	for (int i = 0; i < NodesCount; i++)
	{
		Nodes[i]->GetVariables(Variables);
	}

	for (int i = 0; i < ChildsCount; i++)
	{
		Childs[i]->GetVariables(Variables);
	}
}

void FVoxelComputeNodeTree::GetSetVoxelWorld(const FString& VoxelWorld, FString& OutCpp) const
{
	for (int i = 0; i < NodesCount; i++)
	{
		Nodes[i]->GetSetVoxelWorld(VoxelWorld, OutCpp);
	}

	for (int i = 0; i < ChildsCount; i++)
	{
		Childs[i]->GetSetVoxelWorld(VoxelWorld, OutCpp);
	}
}

/////////////////////////////////////////////////////////////////////////////////

FVoxelNodeTree::~FVoxelNodeTree()
{
	for (auto Child : Childs)
	{
		delete Child;
	}
}

FVoxelNodeTree* FVoxelNodeTree::AddChild()
{
	Childs.Add(new FVoxelNodeTree());
	return Childs[Childs.Num() - 1];
}

void FVoxelNodeTree::Init(const TArray<UVoxelNode*>& AllNodes, const TArray<TArray<int32>>& Dependencies, const int32 NodeIndex)
{
	Nodes.Append(Dependencies[NodeIndex]);
	Nodes.Add(NodeIndex);

	for (auto OutputPin : AllNodes[NodeIndex]->OutputPins)
	{
		if (OutputPin.PinCategory == EVoxelPinCategory::Exec)
		{
			FVoxelNodeTree* Child = AddChild();
			if (OutputPin.OtherNode)
			{
				int32 ChildNodeIndex = AllNodes.IndexOfByKey(OutputPin.OtherNode);
				check(ChildNodeIndex >= 0);
				Child->Init(AllNodes, Dependencies, ChildNodeIndex);
			}
		}
	}
}

void FVoxelNodeTree::CleanupDependencies(TArray<FVoxelNodeTree*> Parents /*= TArray<FVoxelNodeTree*>()*/)
{
	// Remove all nodes already in parents
	Nodes.RemoveAll([Parents](int32 i)
	{
		for (auto Parent : Parents)
		{
			if (Parent->Nodes.Contains(i))
			{
				return true;
			}
		}
		return false;
	});

	// Propagate to childs
	Parents.Add(this);
	for (auto Child : Childs)
	{
		Child->CleanupDependencies(Parents);
	}
}

void FVoxelNodeTree::SetOutputIds(int32 StartId, int32& OutEndId, const TArray<UVoxelNode*>& AllNodes)
{
	int32 Id = StartId;
	for (auto NodeIndex : Nodes)
	{
		UVoxelNode* Node = AllNodes[NodeIndex];
		Node->OutputIds.Reset();

		for (auto Pin : Node->OutputPins)
		{
			if (Pin.PinCategory != EVoxelPinCategory::Exec)
			{
				Node->OutputIds.Add(Id);
				Id++;
			}
		}
	}
	OutEndId = Id + 1;

	for (auto Child : Childs)
	{
		Child->SetOutputIds(OutEndId, OutEndId, AllNodes);
	}
}

void FVoxelNodeTree::SetInputIds(const TArray<UVoxelNode*>& AllNodes)
{
	for (auto NodeIndex : Nodes)
	{
		UVoxelNode* Node = AllNodes[NodeIndex];
		Node->InputIds.Reset();

		for (auto Pin : Node->InputPins)
		{
			if (Pin.PinCategory != EVoxelPinCategory::Exec)
			{
				if (!Pin.OtherNode)
				{
					Node->InputIds.Add(-1);
				}
				else
				{
					int32 PinIndex = Pin.OtherNode->GetOutputPinIndex(Pin.OtherPinId);
					check(PinIndex != -1);
					int32 Id = Pin.OtherNode->OutputIds[PinIndex];
					Node->InputIds.Add(Id);
				}
			}
		}
	}

	for (auto Child : Childs)
	{
		Child->SetInputIds(AllNodes);
	}
}

void FVoxelNodeTree::ConvertToComputeNodeTree(FVoxelComputeNodeTree& OutTree, const TArray<UVoxelNode*>& AllNodes)
{
	TArray<TSharedPtr<FVoxelComputeNode>> ComputeNodes;
	for (auto Node : Nodes)
	{
		ComputeNodes.Add(AllNodes[Node]->GetComputeNode());
	}
	OutTree.SetNodes(ComputeNodes);

	for (auto Child : Childs)
	{
		FVoxelComputeNodeTree* TreeChild = OutTree.AddChild();
		Child->ConvertToComputeNodeTree(*TreeChild, AllNodes);
	}
}


/////////////////////////////////////////////////////////////////////////////////


UVoxelNode * UVoxelGraphGenerator::ConstructNewNode(UClass* NewNodeClass, bool bSelectNewNode /*= true*/)
{
	UVoxelNode* VoxelNode = NewObject<UVoxelNode>(this, NewNodeClass, NAME_None, RF_Transactional);
#if WITH_EDITOR
	AllNodes.Add(VoxelNode);
	SetupVoxelNode(VoxelNode, bSelectNewNode);
#endif // WITH_EDITORONLY_DATA

	return VoxelNode;
}

bool UVoxelGraphGenerator::CompileToCpp(FString& OutCpp, const FString& Filename, FString& OutErrorMessage)
{
	FVoxelComputeNodeTree Tree;
	int32 MaxId;
	bool bSuccess = CreateComputeTree(Tree, MaxId);

	if (!bSuccess)
	{
		OutErrorMessage = TEXT("Start node not connected");
		return false;
	}

	TArray<FString> Variables;
	for (int i = 0; i < MaxId; i++)
	{
		Variables.Add(TEXT("___") + FString::FromInt(i) + TEXT("___"));
	}

	TArray<FVoxelExposedVariable> ExposedVariables;
	Tree.GetExposedVariables(ExposedVariables);
	// Check that there is not duplicates in names
	{
		TSet<FString> Names;
		for (auto Variable : ExposedVariables)
		{
			check(!Variable.VariableName.Contains(TEXT(" ")));

			if (Variable.VariableName.IsEmpty())
			{
				OutErrorMessage = TEXT("Empty name for variable of type: ") + Variable.VariableType;
				return false;
			}
			else if (Names.Contains(Variable.VariableName))
			{
				OutErrorMessage = TEXT("Duplicate name: ") + Variable.VariableName;
				return false;
			}
			else
			{
				Names.Add(Variable.VariableName);
			}
		}
	}

	OutCpp.Reset();
	OutCpp.Append(TEXT("// Copyright 2018 Phyronnaz\n"));
	OutCpp.Append(TEXT("\n"));
	OutCpp.Append(TEXT("#pragma once\n"));
	OutCpp.Append(TEXT("\n"));

	// Headers
	{
		TArray<FString> Headers;
		Headers.Add(TEXT("\"CoreMinimal.h\""));
		Headers.Add(TEXT("\"VoxelUtilities.h\""));
		Headers.Add(TEXT("\"VoxelType.h\""));
		Headers.Add(TEXT("\"VoxelMaterial.h\""));
		Headers.Add(TEXT("\"VoxelWorldGenerator.h\""));
		Headers.Add(TEXT("\"VoxelAsset.h\""));

		TArray<FString> AdditionalHeaders;
		Tree.GetAdditionalHeaders(AdditionalHeaders);
		for (auto H : AdditionalHeaders)
		{
			Headers.AddUnique(H);
		}
		for (auto H : Headers)
		{
			OutCpp.Append(TEXT("#include " + H + "\n"));
		}

		OutCpp.Append(TEXT("#include \"" + Filename + ".generated.h\"\n"));
	}
	
	const FString FClassName(TEXT("F") + Filename + TEXT("Instance"));

	// Class declaration
	{
		OutCpp.Append(TEXT("\n"));
		OutCpp.Append(TEXT("\n"));
		OutCpp.Append(TEXT("class ") + FClassName + TEXT(" : public FVoxelWorldGeneratorInstance\n"));
		OutCpp.Append(TEXT("{\n"));
		OutCpp.Append(TEXT("public:\n"));
		OutCpp.Append(TEXT("	struct FVoxelContext { int32 X; int32 Y; int32 Z; }; \n"));
	}

	// Constructor
	{
		// Declaration
		{
			OutCpp.Append(TEXT("	") + FClassName + TEXT("(\n"));
			bool bFirst = true;
			for (auto Variable : ExposedVariables)
			{
				if (!bFirst)
				{
					OutCpp.Append(TEXT(",\n"));
				}
				OutCpp.Append(TEXT("			") + Variable.GetInstanceType() + TEXT(" ") + Variable.VariableName);
				bFirst = false;
			}
			OutCpp.Append(")\n");
		}
		// Definition
		{
			bool bFirst = true;
			for (auto Variable : ExposedVariables)
			{
				if (bFirst)
				{
					OutCpp.Append(TEXT("	: "));
				}
				else
				{
					OutCpp.Append(TEXT("	, "));
				}
				OutCpp.Append(Variable.VariableName + TEXT("(") + Variable.VariableName + TEXT(")\n"));
				bFirst = false;
			}
			OutCpp.Append(TEXT("	{\n\n"));
			OutCpp.Append(TEXT("	}\n"));
		}
	}

	// GetValuesAndMaterialsAndVoxelTypes
	{
		OutCpp.Append(TEXT("	virtual void GetValuesAndMaterialsAndVoxelTypes(float Values[], FVoxelMaterial Materials[], FVoxelType VoxelTypes[], const FIntVector& Start, const FIntVector& StartIndex, const int Step, const FIntVector& Size, const FIntVector& ArraySize) const override\n"));
		OutCpp.Append(TEXT("	{\n"));
		OutCpp.Append(TEXT("		for (int K = 0; K < Size.Z; K++)\n"));
		OutCpp.Append(TEXT("		{\n"));
		OutCpp.Append(TEXT("			const int Z = Start.Z + K * Step;\n"));
		OutCpp.Append(TEXT("			for (int J = 0; J < Size.Y; J++)\n"));
		OutCpp.Append(TEXT("			{\n"));
		OutCpp.Append(TEXT("				const int Y = Start.Y + J * Step;\n"));
		OutCpp.Append(TEXT("				for (int I = 0; I < Size.X; I++)\n"));
		OutCpp.Append(TEXT("				{\n"));
		OutCpp.Append(TEXT("					const int X = Start.X + I * Step;\n"));
		OutCpp.Append(TEXT("					const int Index = (StartIndex.X + I) + ArraySize.X * (StartIndex.Y + J) + ArraySize.X * ArraySize.Y * (StartIndex.Z + K);\n"));

		// Main
		{
			const FString Value(TEXT("___Value___"));
			const FString Material(TEXT("___Material___"));
			const FString VoxelType(TEXT("___VoxelType___"));
			const FString Context(TEXT("___Context___"));

			OutCpp.Append(TEXT("float ") + Value + TEXT(" = 1;\n"));
			OutCpp.Append(TEXT("FVoxelMaterial ") + Material + TEXT("(0, 0, 0, 0);\n"));
			OutCpp.Append(TEXT("FVoxelType ") + VoxelType + TEXT(" = FVoxelType::UseAll();\n"));
			OutCpp.Append(TEXT("FVoxelContext ") + Context + TEXT(";\n"));
			OutCpp.Append(Context + TEXT(".X = X;\n"));
			OutCpp.Append(Context + TEXT(".Y = Y;\n"));
			OutCpp.Append(Context + TEXT(".Z = Z;\n"));

			Tree.GetMain(Variables, Context, Value, Material, VoxelType, OutCpp);

			OutCpp.Append(TEXT("					if (Values)\n"));
			OutCpp.Append(TEXT("					{\n"));
			OutCpp.Append(TEXT("						Values[Index] = ") + Value + TEXT(";\n"));
			OutCpp.Append(TEXT("					}\n"));
			OutCpp.Append(TEXT("					if (Materials)\n"));
			OutCpp.Append(TEXT("					{\n"));
			OutCpp.Append(TEXT("						Materials[Index] = ") + Material + TEXT(";\n"));
			OutCpp.Append(TEXT("					}\n"));
			OutCpp.Append(TEXT("					if (VoxelTypes)\n"));
			OutCpp.Append(TEXT("					{\n"));
			OutCpp.Append(TEXT("						VoxelTypes[Index] = ") + VoxelType + TEXT(";\n"));
			OutCpp.Append(TEXT("					}\n"));
		}
		OutCpp.Append(TEXT("				}\n"));
		OutCpp.Append(TEXT("			}\n"));
		OutCpp.Append(TEXT("		}\n"));
		OutCpp.Append(TEXT("	}\n"));
		OutCpp.Append(TEXT("	\n"));
	}
	
	// Set voxel world
	{
		OutCpp.Append(TEXT("	virtual void SetVoxelWorld(const AVoxelWorld* ___VoxelWorld___) override\n"));
		OutCpp.Append(TEXT("	{\n"));

		Tree.GetSetVoxelWorld(TEXT("___VoxelWorld___"), OutCpp);

		OutCpp.Append(TEXT("	}\n"));
		OutCpp.Append(TEXT("	\n"));
	}
	

	// IsEmpty
	{
		const FString IsEmptySign(TEXT("__IsEmptySign__"));

		OutCpp.Append(TEXT("	virtual bool IsEmpty(const FIntVector& Start, const int Step, const FIntVector& Size) const override\n"));
		OutCpp.Append(TEXT("	{\n"));
		OutCpp.Append(TEXT("		int ") + IsEmptySign + TEXT(" = 0;\n"));
		OutCpp.Append(TEXT("		for (int K = 0; K < Size.Z; K++)\n"));
		OutCpp.Append(TEXT("		{\n"));
		OutCpp.Append(TEXT("			const int Z = Start.Z + K * Step;\n"));
		OutCpp.Append(TEXT("			for (int J = 0; J < Size.Y; J++)\n"));
		OutCpp.Append(TEXT("			{\n"));
		OutCpp.Append(TEXT("				const int Y = Start.Y + J * Step;\n"));
		OutCpp.Append(TEXT("				for (int I = 0; I < Size.X; I++)\n"));
		OutCpp.Append(TEXT("				{\n"));
		OutCpp.Append(TEXT("					const int X = Start.X + I * Step;\n"));

		// Main
		{
			const FString Value(TEXT("___Value___"));
			const FString Material(TEXT("___Material___"));
			const FString VoxelType(TEXT("___VoxelType___"));
			const FString Context(TEXT("___Context___"));

			OutCpp.Append(TEXT("float ") + Value + TEXT(" = 1;\n"));
			OutCpp.Append(TEXT("FVoxelMaterial ") + Material + TEXT("(0, 0, 0, 0);\n"));
			OutCpp.Append(TEXT("FVoxelType ") + VoxelType + TEXT(" = FVoxelType::UseAll();\n"));
			OutCpp.Append(TEXT("FVoxelContext ") + Context + TEXT(";\n"));
			OutCpp.Append(Context + TEXT(".X = X;\n"));
			OutCpp.Append(Context + TEXT(".Y = Y;\n"));
			OutCpp.Append(Context + TEXT(".Z = Z;\n"));

			Tree.GetMain(Variables, Context, Value, Material, VoxelType, OutCpp);

			OutCpp.Append(TEXT("					if (-1 + KINDA_SMALL_NUMBER < ") + Value + TEXT(" && ") + Value + TEXT(" < 1 - KINDA_SMALL_NUMBER)\n"));
			OutCpp.Append(TEXT("					{\n"));
			OutCpp.Append(TEXT("						return false;\n"));
			OutCpp.Append(TEXT("					}\n\n"));

			OutCpp.Append(TEXT("					if (") + IsEmptySign + TEXT(" == 0)\n"));
			OutCpp.Append(TEXT("					{\n"));
			OutCpp.Append(TEXT("						") + IsEmptySign + TEXT(" = ") + Value + TEXT(" > 0 ? 1 : -1;\n"));
			OutCpp.Append(TEXT("					}\n"));
			OutCpp.Append(TEXT("					else if (") + IsEmptySign + TEXT(" == 1)\n"));
			OutCpp.Append(TEXT("					{\n"));
			OutCpp.Append(TEXT("						if (") + Value + TEXT(" < 0)\n"));
			OutCpp.Append(TEXT("						{\n"));
			OutCpp.Append(TEXT("							return false;\n"));
			OutCpp.Append(TEXT("						}\n"));
			OutCpp.Append(TEXT("					}\n"));
			OutCpp.Append(TEXT("					else\n"));
			OutCpp.Append(TEXT("					{\n"));
			OutCpp.Append(TEXT("						check(") + IsEmptySign + TEXT(" == -1);\n"));
			OutCpp.Append(TEXT("						if (") + Value + TEXT(" > 0)\n"));
			OutCpp.Append(TEXT("						{\n"));
			OutCpp.Append(TEXT("							return false;\n"));
			OutCpp.Append(TEXT("						}\n"));
			OutCpp.Append(TEXT("					}\n"));
		}

		OutCpp.Append(TEXT("				}\n"));
		OutCpp.Append(TEXT("			}\n"));
		OutCpp.Append(TEXT("		}\n"));
		OutCpp.Append(TEXT("		return true;\n"));
		OutCpp.Append(TEXT("	}\n"));
		OutCpp.Append(TEXT("	\n"));
	}

	// Public variables
	{
		OutCpp.Append(TEXT("public:\n"));

		for (auto Variable : ExposedVariables)
		{
			OutCpp.Append(TEXT("	") + Variable.GetInstanceType() + TEXT(" const ") + Variable.VariableName + TEXT(";\n"));
		}
	}
	// Private variables
	{
		OutCpp.Append(TEXT("private:\n"));

		TArray<FVoxelVariable> ClassVariables;

		Tree.GetVariables(ClassVariables);

		for (auto Variable : ClassVariables)
		{
			OutCpp.Append(TEXT("	") + Variable.VariableType + TEXT(" ") + Variable.VariableName + TEXT(";\n"));
		}
	}

	OutCpp.Append(TEXT("\n};\n\n"));

	OutCpp.Append(TEXT("UCLASS(Blueprintable)\n"));
	const FString UClassName(TEXT("U") + Filename);
	OutCpp.Append(TEXT("class " + UClassName + " : public UVoxelWorldGenerator\n"));
	OutCpp.Append(TEXT("{\n"));
	OutCpp.Append(TEXT("	GENERATED_BODY()\n"));
	OutCpp.Append(TEXT("\n"));
	OutCpp.Append(TEXT("public:\n"));
	// Exposed variables
	{
		for (auto Variable : ExposedVariables)
		{
			OutCpp.Append(TEXT("	UPROPERTY(EditAnywhere, BlueprintReadWrite)\n"));
			OutCpp.Append(TEXT("	") + Variable.VariableType + TEXT(" ") + Variable.VariableName + TEXT(";\n"));
		}
	}
	// Constructor
	{
		OutCpp.Append(TEXT("	") + UClassName + TEXT("()\n"));
		bool bFirst = true;
		for (auto Variable : ExposedVariables)
		{
			if (!Variable.VariableDefaultValue.IsEmpty())
			{
				if (bFirst)
				{
					OutCpp.Append(TEXT("	: "));
				}
				else
				{
					OutCpp.Append(TEXT("	, "));
				}
				OutCpp.Append(Variable.VariableName + TEXT("(") + Variable.VariableDefaultValue + TEXT(")\n"));
				bFirst = false;
			}
		}
		OutCpp.Append(TEXT("	{\n\n"));
		OutCpp.Append(TEXT("	}\n"));
	}
	// Instance constructor
	{
		OutCpp.Append(TEXT("	TSharedRef<FVoxelWorldGeneratorInstance> GetWorldGenerator() override\n"));
		OutCpp.Append(TEXT("	{\n"));
		OutCpp.Append(TEXT("		return MakeShareable(new ") + FClassName + TEXT("(\n"));
		bool bFirst = true;
		for (auto Variable : ExposedVariables)
		{
			if (!bFirst)
			{
				OutCpp.Append(TEXT(",\n"));
			}
			OutCpp.Append(TEXT("			") + Variable.GetAccessor());
			bFirst = false;
		}
		OutCpp.Append(TEXT("));\n"));
		OutCpp.Append(TEXT("	}\n"));
	}
	OutCpp.Append(TEXT("\n};"));

	return true;
}

bool UVoxelGraphGenerator::CreateComputeTree(FVoxelComputeNodeTree& OutTree, int32& OutMaxId)
{
	if (!FirstNode)
	{
		return false;
	}

	TArray<UVoxelNode*> Nodes;
	for (auto Node : AllNodes)
	{
		Nodes.AddUnique(Node);
	}

	Nodes.Remove(nullptr);

	/**
	 * Create dependencies. For each i, Dependencies[i] is sorted by compute order: Dependencies[i][1] needs Dependencies[i][0], Dependencies[i][2] needs Dependencies[i][0] and Dependencies[i][1] ...
	 */
	TArray<TArray<int32>> Dependencies;
	{
		Dependencies.SetNumZeroed(Nodes.Num());

		for (int i = 0; i < Dependencies.Num(); i++)
		{
			for (auto Pin : Nodes[i]->InputPins)
			{
				if (Pin.OtherNode)
				{
					int32 OtherNodeIndex = Nodes.IndexOfByKey(Pin.OtherNode);
					check(OtherNodeIndex >= 0);
					Dependencies[i].AddUnique(OtherNodeIndex);
				}
			}
		}

		// Propagate dependencies
		bool bContinue = true;
		while (bContinue)
		{
			bContinue = false;
			for (int i = 0; i < Dependencies.Num(); i++)
			{
				TArray<int32> NewDeps;
				for (int DepsIndex = 0; DepsIndex < Dependencies[i].Num(); DepsIndex++)
				{
					int j = Dependencies[i][DepsIndex];
					
					TArray<int32> TmpDeps;
					for (auto k : Dependencies[j])
					{
						if(!NewDeps.Contains(k)) // If k is in new deps, it's at the right position (NewDeps < j)
						{
							int DepsOfDepsIndex;
							if (Dependencies[i].Find(k, DepsOfDepsIndex))
							{
								if (DepsIndex < DepsOfDepsIndex) // If DepsOfDepsIndex < DepsIndex it's already in the right order
								{
									Dependencies[i].RemoveAt(DepsOfDepsIndex);
									TmpDeps.AddUnique(k);
								}
							}
							else
							{
								TmpDeps.AddUnique(k);
							}
						}
					}
					NewDeps.Insert(TmpDeps, 0);
				}
				if (NewDeps.Num() != 0)
				{
					bContinue = true;
					Dependencies[i].Insert(NewDeps, 0);
				}
			}
		}
	}

	/**
	 * Create the tree
	 */
	{
		int32 FirstNodeIndex = Nodes.IndexOfByKey(FirstNode);
		check(FirstNodeIndex >= 0);

		TSharedPtr<FVoxelNodeTree> MainTree = MakeShareable(new FVoxelNodeTree());
		MainTree->Init(Nodes, Dependencies, FirstNodeIndex);
		MainTree->CleanupDependencies();

		MainTree->SetOutputIds(0, OutMaxId, Nodes);
		MainTree->SetInputIds(Nodes);

		MainTree->ConvertToComputeNodeTree(OutTree, Nodes);
	}

	return true;
}

TSharedRef<FVoxelWorldGeneratorInstance> UVoxelGraphGenerator::GetWorldGenerator()
{
	TSharedRef<FVoxelComputeNodeTree> Tree = MakeShareable(new FVoxelComputeNodeTree());
	int32 MaxId;
	bool bValid = CreateComputeTree(*Tree, MaxId);
	if (bValid)
	{
		return MakeShareable(new FVoxelGraphWorldGeneratorInstance(MaxId, Tree));
	}
	else
	{
		return MakeShareable(new FEmptyWorldGeneratorInstance());
	}
}

#if WITH_EDITOR

void UVoxelGraphGenerator::PostInitProperties()
{
	Super::PostInitProperties();
	if (!HasAnyFlags(RF_ClassDefaultObject | RF_NeedLoad))
	{
		CreateGraph();
	}
}

void UVoxelGraphGenerator::SetupVoxelNode(UVoxelNode* InVoxelNode, bool bSelectNewNode /*= true*/)
{
	// Create the graph node
	check(InVoxelNode->GraphNode == NULL);

	UVoxelGraphGenerator::GetVoxelGraphEditor()->SetupVoxelNode(VoxelGraph, InVoxelNode, bSelectNewNode);
}

void UVoxelGraphGenerator::CreateGraph()
{
	if (VoxelGraph == nullptr)
	{
		VoxelGraph = UVoxelGraphGenerator::GetVoxelGraphEditor()->CreateNewVoxelGraph(this);
		VoxelGraph->bAllowDeletion = false;

		// Give the schema a chance to fill out any required nodes (like the results node)
		const UEdGraphSchema* Schema = VoxelGraph->GetSchema();
		Schema->CreateDefaultNodesForGraph(*VoxelGraph);
	}
}

UEdGraph* UVoxelGraphGenerator::GetGraph()
{
	return VoxelGraph;
}

void UVoxelGraphGenerator::CompileVoxelNodesFromGraphNodes()
{
	UVoxelGraphGenerator::GetVoxelGraphEditor()->CompileVoxelNodesFromGraphNodes(this);
	AllNodes.Remove(nullptr);
}

void UVoxelGraphGenerator::SetVoxelGraphEditor(TSharedPtr<IVoxelGraphEditor> InVoxelGraphEditor)
{
	check(!VoxelGraphEditor.IsValid());
	VoxelGraphEditor = InVoxelGraphEditor;
}

TSharedPtr<IVoxelGraphEditor> UVoxelGraphGenerator::GetVoxelGraphEditor()
{
	return VoxelGraphEditor;
}

TSharedPtr<IVoxelGraphEditor> UVoxelGraphGenerator::VoxelGraphEditor = nullptr;

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FVoxelGraphWorldGeneratorInstance::FVoxelGraphWorldGeneratorInstance(int32 MaxId, TSharedRef<FVoxelComputeNodeTree> ComputeTree)
	: MaxId(MaxId)
	, ComputeTree(ComputeTree)
{

}

void FVoxelGraphWorldGeneratorInstance::GetValuesAndMaterialsAndVoxelTypes(float Values[], FVoxelMaterial Materials[], FVoxelType VoxelTypes[], const FIntVector& Start, const FIntVector& StartIndex, const int Step, const FIntVector& Size, const FIntVector& ArraySize) const
{
	FVoxelNodeType* Variables = new FVoxelNodeType[MaxId];

	for (int K = 0; K < Size.Z; K++)
	{
		const int Z = Start.Z + K * Step;
		for (int J = 0; J < Size.Y; J++)
		{
			const int Y = Start.Y + J * Step;
			for (int I = 0; I < Size.X; I++)
			{
				const int X = Start.X + I * Step;

				const int Index = (StartIndex.X + I) + ArraySize.X * (StartIndex.Y + J) + ArraySize.X * ArraySize.Y * (StartIndex.Z + K);

				float Value = 1;
				FVoxelMaterial Material(0, 0, 0, 0);
				FVoxelType VoxelType = FVoxelType::UseAll();
				FVoxelContext Context;
				Context.X = X;
				Context.Y = Y;
				Context.Z = Z;

				ComputeTree->Compute(Variables, Context, Value, Material, VoxelType);

				if (Values)
				{
					Values[Index] = Value;
				}
				if (Materials)
				{
					Materials[Index] = Material;
				}
				if (VoxelTypes)
				{
					VoxelTypes[Index] = VoxelType;
				}
			}
		}
	}

	delete[] Variables;
}

void FVoxelGraphWorldGeneratorInstance::SetVoxelWorld(const AVoxelWorld* VoxelWorld)
{
	ComputeTree->Init(VoxelWorld);
}

bool FVoxelGraphWorldGeneratorInstance::IsEmpty(const FIntVector& Start, const int Step, const FIntVector& Size) const
{
	int Sign = 0;

	FVoxelNodeType* Variables = new FVoxelNodeType[MaxId];

	for (int K = 0; K < Size.Z; K++)
	{
		const int Z = Start.Z + K * Step;
		for (int J = 0; J < Size.Y; J++)
		{
			const int Y = Start.Y + J * Step;
			for (int I = 0; I < Size.X; I++)
			{
				const int X = Start.X + I * Step;

				float Value = 1;
				FVoxelMaterial Material(0, 0, 0, 0);
				FVoxelType VoxelType = FVoxelType::UseAll();
				FVoxelContext Context;
				Context.X = X;
				Context.Y = Y;
				Context.Z = Z;

				ComputeTree->Compute(Variables, Context, Value, Material, VoxelType);

				if (-1 + KINDA_SMALL_NUMBER < Value && Value < 1 - KINDA_SMALL_NUMBER)
				{
					delete[] Variables;
					return false;
				}

				if (Sign == 0)
				{
					Sign = Value > 0 ? 1 : -1;
				}
				else if (Sign == 1)
				{
					if (Value < 0)
					{
						delete[] Variables;
						return false;
					}
				}
				else
				{
					check(Sign == -1);
					if (Value > 0)
					{
						delete[] Variables;
						return false;
					}
				}
			}
		}
	}

	delete[] Variables;
	return true;
}
