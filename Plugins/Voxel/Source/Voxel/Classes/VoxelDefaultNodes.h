// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelNode.h"
#include "VoxelDefaultNodes.generated.h"

#define LOCTEXT_NAMESPACE "VoxelNodes"

#define GENERATED_GETCOMPUTENODE(Name) \
TSharedPtr<FVoxelComputeNode> UVoxelNode_##Name::GetComputeNode() const\
{\
	return MakeShareable(new FVoxelComputeNode_##Name(this));\
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

#define GENERATED_NODE_BODY(MinInputs, MaxInputs, Outputs) \
int32 GetMinInputPins() const override { return MinInputs; } \
int32 GetMaxInputPins() const override { return MaxInputs; } \
int32 GetOutputPinsCount() const override { return Outputs; } \
TSharedPtr<FVoxelComputeNode> GetComputeNode() const override;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

#define GENERATED_COMPUTENODE(CppName)\
class FVoxelComputeNode_##CppName : public FVoxelComputeNode\
{\
public:\
	FVoxelComputeNode_##CppName(const UVoxelNode_##CppName* Node) : FVoxelComputeNode(Node) {}\
\
	void Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const override; \
	void GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const; \
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "If"))
class VOXEL_API UVoxelNode_If : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(2, 2, 2)
public:
	FString GetInputPinCategory(int32 PinIndex) const override { return PinIndex == 0 ? FVoxelPinCategory::PC_Exec : FVoxelPinCategory::PC_Boolean; }
	FString GetOutputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_Exec; }
	FText GetOutputPinName(int32 PinIndex) const override { return PinIndex == 0 ? LOCTEXT("True", "True") : LOCTEXT("False", "False"); }
	FLinearColor GetColor()	const override { return FLinearColor::Red; }
};
class FVoxelComputeNode_If : public FVoxelComputeNode
{
public:
	FVoxelComputeNode_If(const UVoxelNode_If* Node) : FVoxelComputeNode(Node) {}

	virtual int32 GetBranchResult(FVoxelNodeType Inputs[]) const override { return Inputs[0].B ? 0 : 1; }
	virtual FString GetBranchResultCpp(const TArray<FString>& Inputs) const override { return Inputs[0] + TEXT(" ? 0 : 1"); }
};

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "Set Value"))
class VOXEL_API UVoxelNode_SetValue : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(2, 2, 1)
public:
	UPROPERTY(EditAnywhere)
	bool bDisableClamp;

	FString GetInputPinCategory(int32 PinIndex) const override { return PinIndex == 0 ? FVoxelPinCategory::PC_Exec : FVoxelPinCategory::PC_Float; }
	FString GetOutputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_Exec; }
	FLinearColor GetColor()	const override { return FLinearColor::Red; }
};
class FVoxelComputeNode_SetValue : public FVoxelComputeNode
{
public:
	const bool bDisableClamp;

	FVoxelComputeNode_SetValue(const UVoxelNode_SetValue* Node) : FVoxelComputeNode(Node), bDisableClamp(Node->bDisableClamp) {}

	bool IsSetValueNode() const override { return true; }
};

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "Set Material"))
class VOXEL_API UVoxelNode_SetMaterial : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(2, 2, 1)
public:
	FString GetInputPinCategory(int32 PinIndex) const override { return PinIndex == 0 ? FVoxelPinCategory::PC_Exec : FVoxelPinCategory::PC_Material; }
	FString GetOutputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_Exec; }
	FLinearColor GetColor()	const override { return FLinearColor::Red; }
};
class FVoxelComputeNode_SetMaterial : public FVoxelComputeNode
{
public:
	FVoxelComputeNode_SetMaterial(const UVoxelNode_SetMaterial* Node) : FVoxelComputeNode(Node) {}

	bool IsSetMaterialNode() const override { return true; }
};

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "Set Voxel Type"))
class VOXEL_API UVoxelNode_SetVoxelType : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(2, 2, 1)
public:
	FString GetInputPinCategory(int32 PinIndex) const override { return PinIndex == 0 ? FVoxelPinCategory::PC_Exec : FVoxelPinCategory::PC_VoxelType; }
	FString GetOutputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_Exec; }
	FLinearColor GetColor()	const override { return FLinearColor::Red; }
};
class FVoxelComputeNode_SetVoxelType : public FVoxelComputeNode
{
public:
	FVoxelComputeNode_SetVoxelType(const UVoxelNode_SetVoxelType* Node) : FVoxelComputeNode(Node) {}

	bool IsSetVoxelTypeNode() const override { return true; }
};

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "Make Material"))
class VOXEL_API UVoxelNode_MakeMaterial : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(4, 4, 1)
public:
	FString GetInputPinCategory(int32 PinIndex) const override { return PinIndex == 2 ? FVoxelPinCategory::PC_Float : FVoxelPinCategory::PC_Int; }
	FString GetOutputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_Material; }
	FLinearColor GetColor()	const override { return FLinearColor::Blue; }
	FText GetInputPinName(int32 PinIndex) const override
	{
		if (PinIndex == 0)
		{
			return LOCTEXT("A", "A");
		}
		else if (PinIndex == 1)
		{
			return LOCTEXT("B", "B");
		}
		else if (PinIndex == 2)
		{
			return LOCTEXT("Alpha", "Alpha");
		}
		else if (PinIndex == 3)
		{
			return LOCTEXT("Voxel Actor", "Voxel Actor");
		}
		else
		{
			return LOCTEXT("", "");
		}
	}
};

GENERATED_COMPUTENODE(MakeMaterial)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "Make VoxelType"))
class VOXEL_API UVoxelNode_MakeVoxelType : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(2, 2, 1)
public:
	FString GetInputPinCategory(int32 PinIndex) const override { return PinIndex == 0 ? FVoxelPinCategory::PC_ValueType : FVoxelPinCategory::PC_MaterialType; }
	FString GetOutputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_VoxelType; }
	FLinearColor GetColor()	const override { return FLinearColor::Yellow; }
	FText GetInputPinName(int32 PinIndex) const override
	{
		if (PinIndex == 0)
		{
			return LOCTEXT("ValueType", "Value Type");
		}
		else if (PinIndex == 1)
		{
			return LOCTEXT("MaterialType", "Material Type");
		}
		else
		{
			return LOCTEXT("", "");
		}
	}
};

GENERATED_COMPUTENODE(MakeVoxelType)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "Break Material"))
class VOXEL_API UVoxelNode_BreakMaterial : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(1, 1, 3)
public:
	FString GetInputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_Material; }
	FString GetOutputPinCategory(int32 PinIndex) const override { return PinIndex == 2 ? FVoxelPinCategory::PC_Float : FVoxelPinCategory::PC_Int; }
	FLinearColor GetColor()	const override { return FLinearColor::Blue; }
	FText GetOutputPinName(int32 PinIndex) const override
	{
		if (PinIndex == 0)
		{
			return LOCTEXT("A", "A");
		}
		else if (PinIndex == 1)
		{
			return LOCTEXT("B", "B");
		}
		else if (PinIndex == 2)
		{
			return LOCTEXT("Alpha", "Alpha");
		}
		{
			return LOCTEXT("", "");
		}
	}
};

GENERATED_COMPUTENODE(BreakMaterial)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "Break Voxel Type"))
class VOXEL_API UVoxelNode_BreakVoxelType : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(1, 1, 2)
public:
	FString GetInputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_VoxelType; }
	FString GetOutputPinCategory(int32 PinIndex) const override { return PinIndex == 0 ? FVoxelPinCategory::PC_ValueType : FVoxelPinCategory::PC_MaterialType; }
	FLinearColor GetColor()	const override { return FLinearColor::Yellow; }
	FText GetOutputPinName(int32 PinIndex) const override
	{
		if (PinIndex == 0)
		{
			return LOCTEXT("ValueType", "Value Type");
		}
		else if (PinIndex == 1)
		{
			return LOCTEXT("MaterialType", "Material Type");
		}
		else
		{
			return LOCTEXT("", "");
		}
	}
};

GENERATED_COMPUTENODE(BreakVoxelType)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "X (float)"))
class VOXEL_API UVoxelNode_XF : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(0, 0, 1)
public:
	FText GetTitle() const override { return LOCTEXT("X", "X"); }
	FLinearColor GetColor()	const override { return FLinearColor::Green; }
};

GENERATED_COMPUTENODE(XF)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "Y (float)"))
class VOXEL_API UVoxelNode_YF : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(0, 0, 1)
public:
	FText GetTitle() const override { return LOCTEXT("Y", "Y"); }
	FLinearColor GetColor()	const override { return FLinearColor::Green; }
};

GENERATED_COMPUTENODE(YF)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "Z (float)"))
class VOXEL_API UVoxelNode_ZF : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(0, 0, 1)
public:
	FText GetTitle() const override { return LOCTEXT("Z", "Z"); }
	FLinearColor GetColor()	const override { return FLinearColor::Green; }
};

GENERATED_COMPUTENODE(ZF)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "X (int)"))
class VOXEL_API UVoxelNode_XI : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(0, 0, 1)
public:
	FString GetOutputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_Int; }
	FLinearColor GetColor()	const override { return FLinearColor::Blue; }
	FText GetTitle() const override { return LOCTEXT("X", "X"); }
};

GENERATED_COMPUTENODE(XI)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "Y (int)"))
class VOXEL_API UVoxelNode_YI : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(0, 0, 1)
public:
	FString GetOutputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_Int; }
	FLinearColor GetColor()	const override { return FLinearColor::Blue; }
	FText GetTitle() const override { return LOCTEXT("Y", "Y"); }
};

GENERATED_COMPUTENODE(YI)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "Z (int)"))
class VOXEL_API UVoxelNode_ZI : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(0, 0, 1)
public:
	FString GetOutputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_Int; }
	FLinearColor GetColor()	const override { return FLinearColor::Blue; }
	FText GetTitle() const override { return LOCTEXT("Z", "Z"); }
};

GENERATED_COMPUTENODE(ZI)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "Max"))
class VOXEL_API UVoxelNode_Max : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(2, MAX_PINS - 1, 1)
};

GENERATED_COMPUTENODE(Max)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "Min"))
class VOXEL_API UVoxelNode_Min : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(2, MAX_PINS - 1, 1)
};

GENERATED_COMPUTENODE(Min)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "float < float", Keywords = "< less"))
class VOXEL_API UVoxelNode_Less : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(2, 2, 1)
public:
	FString GetOutputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_Boolean; }
	bool IsCompact() const override { return true; }
	FText GetTitle() const override { return LOCTEXT("<", "<"); }
};

GENERATED_COMPUTENODE(Less)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "float <= float", Keywords = "<= less"))
class VOXEL_API UVoxelNode_LessEqual : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(2, 2, 1)
public:
	FString GetOutputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_Boolean; }
	bool IsCompact() const override { return true; }
	FText GetTitle() const override { return LOCTEXT("<=", "<="); }
};

GENERATED_COMPUTENODE(LessEqual)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "float > float", Keywords = "> greater"))
class VOXEL_API UVoxelNode_Greater : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(2, 2, 1)
public:
	FString GetOutputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_Boolean; }
	bool IsCompact() const override { return true; }
	FText GetTitle() const override { return LOCTEXT(">", ">"); }
};

GENERATED_COMPUTENODE(Greater)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "float >= float", Keywords = ">= greater"))
class VOXEL_API UVoxelNode_GreaterEqual : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(2, 2, 1)
public:
	FString GetOutputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_Boolean; }
	bool IsCompact() const override { return true; }
	FText GetTitle() const override { return LOCTEXT(">=", ">="); }
};

GENERATED_COMPUTENODE(GreaterEqual)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "Constant (float)"))
class VOXEL_API UVoxelNode_FConstant : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(0, 0, 1)
public:
	UPROPERTY(EditAnywhere)
		float Value;

	UPROPERTY(EditAnywhere)
		bool bExposeToBP;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bExposeToBP"))
		FString UniqueName;

	FText GetTitle() const override { return FText::FromString(FString::SanitizeFloat(Value)); }
	FLinearColor GetColor()	const override { return bExposeToBP ? FLinearColor::Yellow : FLinearColor::Green; }
};

class FVoxelComputeNode_FConstant : public FVoxelComputeNode
{
public:
	FVoxelComputeNode_FConstant(const UVoxelNode_FConstant* Node)
		: FVoxelComputeNode(Node)
		, Value(Node->Value)
		, bExposeToBP(Node->bExposeToBP)
		, UniqueName(Node->UniqueName.Replace(TEXT(" "), TEXT("")))
	{
	}

	void Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const override;
	void GetExposedVariables(TArray<FVoxelExposedVariable>& Variables) const override;
	void GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const;

private:
	const float Value;
	const bool bExposeToBP;
	const FString UniqueName;
};

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "Constant (int)"))
class VOXEL_API UVoxelNode_IConstant : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(0, 0, 1)
public:
	UPROPERTY(EditAnywhere)
		int32 Value;

	UPROPERTY(EditAnywhere)
		bool bExposeToBP;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bExposeToBP"))
		FString UniqueName;

	FString GetOutputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_Int; }
	FText GetTitle() const override { return FText::FromString(FString::FromInt(Value)); }
	FLinearColor GetColor()	const override { return bExposeToBP ? FLinearColor::Yellow : FLinearColor::Blue; }
};

class FVoxelComputeNode_IConstant : public FVoxelComputeNode
{
public:
	FVoxelComputeNode_IConstant(const UVoxelNode_IConstant* Node)
		: FVoxelComputeNode(Node)
		, Value(Node->Value)
		, bExposeToBP(Node->bExposeToBP)
		, UniqueName(Node->UniqueName.Replace(TEXT(" "), TEXT("")))
	{
	}

	void Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const override;
	void GetExposedVariables(TArray<FVoxelExposedVariable>& Variables) const override;
	void GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const;

private:
	const int32 Value;
	const bool bExposeToBP;
	const FString UniqueName;
};

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

// TODO: implicit when dragging
UCLASS(meta = (DisplayName = "int to float"))
class VOXEL_API UVoxelNode_FloatOfInt : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(1, 1, 1)
public:
	FString GetInputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_Int; }
	bool IsCompact() const override { return true; }
	FText GetTitle() const override { return LOCTEXT("float", "float"); }
};

GENERATED_COMPUTENODE(FloatOfInt)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

// TODO: implicit when dragging
UCLASS(meta = (DisplayName = "Round"))
class VOXEL_API UVoxelNode_Round : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(1, 1, 1)
public:
	FString GetInputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_Float; }
	FString GetOutputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_Int; }
	bool IsCompact() const override { return true; }
	FText GetTitle() const override { return LOCTEXT("Round", "Round"); }
};

GENERATED_COMPUTENODE(Round)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "Lerp"))
class VOXEL_API UVoxelNode_Lerp : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(3, 3, 1)
public:
	FText GetInputPinName(int32 PinIndex) const override
	{
		if (PinIndex == 0)
		{
			return LOCTEXT("A", "A");
		}
		else if (PinIndex == 1)
		{
			return LOCTEXT("B", "B");
		}
		else if (PinIndex == 2)
		{
			return LOCTEXT("Alpha", "Alpha");
		}
		{
			return LOCTEXT("", "");
		}
	}
};

GENERATED_COMPUTENODE(Lerp)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "Clamp"))
class VOXEL_API UVoxelNode_Clamp : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(3, 3, 1)
public:
	FText GetInputPinName(int32 PinIndex) const override
	{
		if (PinIndex == 0)
		{
			return LOCTEXT("Value", "Value");
		}
		else if (PinIndex == 1)
		{
			return LOCTEXT("Min", "Min");
		}
		else if (PinIndex == 2)
		{
			return LOCTEXT("Max", "Max");
		}
		{
			return LOCTEXT("", "");
		}
	}
};

GENERATED_COMPUTENODE(Clamp)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "float + float", Keywords = "+ add plus"))
class VOXEL_API UVoxelNode_FAdd : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(2, MAX_PINS - 1, 1)
public:
	bool IsCompact() const override { return true; }
	FText GetTitle() const override { return LOCTEXT("+", "+"); }
};

GENERATED_COMPUTENODE(FAdd)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "float - float", Keywords = "- subtract minus"))
class VOXEL_API UVoxelNode_FSubstract : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(2, 2, 1)
public:
	bool IsCompact() const override { return true; }
	FText GetTitle() const override { return LOCTEXT("-", "-"); }
};

GENERATED_COMPUTENODE(FSubstract)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "float * float", Keywords = "* multiply"))
class VOXEL_API UVoxelNode_FMultiply : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(2, MAX_PINS - 1, 1)
public:
	bool IsCompact() const override { return true; }
	FText GetTitle() const override { return LOCTEXT("*", "*"); }
};

GENERATED_COMPUTENODE(FMultiply)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "float / float", Keywords = "/ divide division"))
class VOXEL_API UVoxelNode_FDivide : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(2, 2, 1)
public:
	bool IsCompact() const override { return true; }
	FText GetTitle() const override { return LOCTEXT("/", "/"); }
};

GENERATED_COMPUTENODE(FDivide)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "int + int", Keywords = "+ add plus"))
class VOXEL_API UVoxelNode_IAdd : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(2, MAX_PINS - 1, 1)
public:
	bool IsCompact() const override { return true; }
	FText GetTitle() const override { return LOCTEXT("+", "+"); }
	FString GetInputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_Int; }
	FString GetOutputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_Int; }
};

GENERATED_COMPUTENODE(IAdd)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "int - int", Keywords = "- subtract minus"))
class VOXEL_API UVoxelNode_ISubstract : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(2, 2, 1)
public:
	bool IsCompact() const override { return true; }
	FText GetTitle() const override { return LOCTEXT("-", "-"); }
	FString GetInputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_Int; }
	FString GetOutputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_Int; }
};

GENERATED_COMPUTENODE(ISubstract)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "int * int", Keywords = "* multiply"))
class VOXEL_API UVoxelNode_IMultiply : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(2, MAX_PINS - 1, 1)
public:
	bool IsCompact() const override { return true; }
	FText GetTitle() const override { return LOCTEXT("*", "*"); }
	FString GetInputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_Int; }
	FString GetOutputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_Int; }
};

GENERATED_COMPUTENODE(IMultiply)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "int / int", Keywords = "/ divide division"))
class VOXEL_API UVoxelNode_IDivide : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(2, 2, 1)
public:
	bool IsCompact() const override { return true; }
	FText GetTitle() const override { return LOCTEXT("/", "/"); }
	FString GetInputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_Int; }
	FString GetOutputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_Int; }
};

GENERATED_COMPUTENODE(IDivide)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "1 - X"))
class VOXEL_API UVoxelNode_1MinusX : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(1, 1, 1)
public:
	bool IsCompact() const override { return true; }
	FText GetTitle() const override { return LOCTEXT("1 - X", "1 - X"); }
};

GENERATED_COMPUTENODE(1MinusX)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "Sqrt"))
class VOXEL_API UVoxelNode_Sqrt : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(1, 1, 1)
public:
	bool IsCompact() const override { return true; }
	FText GetTitle() const override { return LOCTEXT("SQRT", "SQRT"); }
};

GENERATED_COMPUTENODE(Sqrt)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "Pow"))
class VOXEL_API UVoxelNode_Pow : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(2, 2, 1)
public:
	bool IsCompact() const override { return true; }
	FText GetTitle() const override { return LOCTEXT("POW", "POW"); }
};

GENERATED_COMPUTENODE(Pow)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "% (int)", Keywords = "% modulus"))
class VOXEL_API UVoxelNode_IMod : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(2, 2, 1)
public:
	bool IsCompact() const override { return true; }
	FText GetTitle() const override { return LOCTEXT("%", "%"); }
	FString GetInputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_Int; }
	FString GetOutputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_Int; }
};

GENERATED_COMPUTENODE(IMod)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "Absolute (float)"))
class VOXEL_API UVoxelNode_FAbs : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(1, 1, 1)
public:
	bool IsCompact() const override { return true; }
	FText GetTitle() const override { return LOCTEXT("ABS", "ABS"); }
};

GENERATED_COMPUTENODE(FAbs)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "Absolute (int)"))
class VOXEL_API UVoxelNode_IAbs : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(1, 1, 1)
public:
	bool IsCompact() const override { return true; }
	FText GetTitle() const override { return LOCTEXT("ABS", "ABS"); }
	FString GetInputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_Int; }
	FString GetOutputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_Int; }
};

GENERATED_COMPUTENODE(IAbs)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "World Generator Sampler"))
class VOXEL_API UVoxelNode_WorldGeneratorSampler : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(3, 3, 3)
public:
	UPROPERTY(EditAnywhere)
		FVoxelWorldGeneratorPicker WorldGenerator;

	UPROPERTY(EditAnywhere)
		FString UniqueName;

	FText GetTitle() const override { return FText::FromString(WorldGenerator.GetName()); }
	FString GetInputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_Float; }
	FString GetOutputPinCategory(int32 PinIndex) const override { return PinIndex == 0 ? FVoxelPinCategory::PC_Float : PinIndex == 1 ? FVoxelPinCategory::PC_Material : FVoxelPinCategory::PC_VoxelType; }
	FText GetInputPinName(int32 PinIndex) const override { return PinIndex == 0 ? LOCTEXT("X", "X") : PinIndex == 1 ? LOCTEXT("Y", "Y") : LOCTEXT("Z", "Z"); }
	FText GetOutputPinName(int32 PinIndex) const override { return PinIndex == 0 ? LOCTEXT("Value", "Value") : PinIndex == 1 ? LOCTEXT("Material", "Material") : LOCTEXT("VoxelType", "Voxel Type"); }
};

class FVoxelComputeNode_WorldGeneratorSampler : public FVoxelComputeNode
{
public:
	FVoxelComputeNode_WorldGeneratorSampler(const UVoxelNode_WorldGeneratorSampler* Node)
		: FVoxelComputeNode(Node)
		, WorldGenerator(Node->WorldGenerator.GetWorldGenerator())
		, WorldGeneratorUniqueName(Node->UniqueName.Replace(TEXT(" "), TEXT("")))
	{
	}

	void Init(const AVoxelWorld* VoxelWorld) override;
	void Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const override;

	void GetExposedVariables(TArray<FVoxelExposedVariable>& Variables) const override;
	void GetSetVoxelWorld(const FString& VoxelWorld, FString& OutCpp) const override;
	void GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const;

private:
	TSharedRef<FVoxelWorldGeneratorInstance> const WorldGenerator;
	FString WorldGeneratorUniqueName;
};

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "Merge Values"))
class VOXEL_API UVoxelNode_MergeValues : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(3, 3, 1)
public:
	FString GetInputPinCategory(int32 PinIndex) const override { return PinIndex == 1 ? FVoxelPinCategory::PC_ValueType : FVoxelPinCategory::PC_Float; }
	FText GetInputPinName(int32 PinIndex) const override
	{
		if (PinIndex == 0)
		{
			return LOCTEXT("A", "A");
		}
		else if (PinIndex == 1)
		{
			return LOCTEXT("ValueType", "Value Type");
		}
		else if (PinIndex == 2)
		{
			return LOCTEXT("DefaultValue", "Default Value");
		}
		else
		{
			return LOCTEXT("", "");
		}
	}
};

GENERATED_COMPUTENODE(MergeValues)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "Merge Materials"))
class VOXEL_API UVoxelNode_MergeMaterials : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(3, 3, 1)
public:
	FString GetInputPinCategory(int32 PinIndex) const override { return PinIndex == 1 ? FVoxelPinCategory::PC_MaterialType : FVoxelPinCategory::PC_Material; }
	FString GetOutputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_Material; }
	FText GetInputPinName(int32 PinIndex) const override
	{
		if (PinIndex == 0)
		{
			return LOCTEXT("A", "A");
		}
		else if (PinIndex == 1)
		{
			return LOCTEXT("MaterialType", "Material Type");
		}
		else if (PinIndex == 2)
		{
			return LOCTEXT("DefaultMaterial", "Default Material");
		}
		else
		{
			return LOCTEXT("", "");
		}
	}
};

GENERATED_COMPUTENODE(MergeMaterials)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "AND Boolean", Keywords = "& and"))
class VOXEL_API UVoxelNode_BAnd : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(2, MAX_PINS - 1, 1)
public:
	bool IsCompact() const override { return true; }
	FText GetTitle() const override { return LOCTEXT("AND", "AND"); }
	FString GetInputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_Boolean; }
	FString GetOutputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_Boolean; }
};

GENERATED_COMPUTENODE(BAnd)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "OR Boolean", Keywords = "| or"))
class VOXEL_API UVoxelNode_BOr : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(2, MAX_PINS - 1, 1)
public:
	bool IsCompact() const override { return true; }
	FText GetTitle() const override { return LOCTEXT("OR", "OR"); }
	FString GetInputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_Boolean; }
	FString GetOutputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_Boolean; }
};

GENERATED_COMPUTENODE(BOr)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "NOT Boolean", Keywords = "! not"))
class VOXEL_API UVoxelNode_BNot : public UVoxelNode
{
	GENERATED_BODY()

public:
	GENERATED_NODE_BODY(1, 1, 1)
public:
	bool IsCompact() const override { return true; }
	FText GetTitle() const override { return LOCTEXT("NOT", "NOT"); }
	FString GetInputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_Boolean; }
	FString GetOutputPinCategory(int32 PinIndex) const override { return FVoxelPinCategory::PC_Boolean; }
};

GENERATED_COMPUTENODE(BNot)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
