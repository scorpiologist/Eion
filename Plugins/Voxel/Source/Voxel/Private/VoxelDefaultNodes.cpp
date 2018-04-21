// Copyright 2018 Phyronnaz

#include "VoxelDefaultNodes.h"
#include "VoxelUtilities.h"

GENERATED_GETCOMPUTENODE(If)

GENERATED_GETCOMPUTENODE(SetValue)
GENERATED_GETCOMPUTENODE(SetMaterial)
GENERATED_GETCOMPUTENODE(SetVoxelType)

GENERATED_GETCOMPUTENODE(MakeMaterial)
GENERATED_GETCOMPUTENODE(MakeVoxelType)
GENERATED_GETCOMPUTENODE(BreakMaterial)
GENERATED_GETCOMPUTENODE(BreakVoxelType)


GENERATED_GETCOMPUTENODE(XF)
GENERATED_GETCOMPUTENODE(YF)
GENERATED_GETCOMPUTENODE(ZF)

GENERATED_GETCOMPUTENODE(XI)
GENERATED_GETCOMPUTENODE(YI)
GENERATED_GETCOMPUTENODE(ZI)

GENERATED_GETCOMPUTENODE(Max)
GENERATED_GETCOMPUTENODE(Min)

GENERATED_GETCOMPUTENODE(Less)
GENERATED_GETCOMPUTENODE(LessEqual)
GENERATED_GETCOMPUTENODE(Greater)
GENERATED_GETCOMPUTENODE(GreaterEqual)

GENERATED_GETCOMPUTENODE(FConstant)
GENERATED_GETCOMPUTENODE(IConstant)

GENERATED_GETCOMPUTENODE(FloatOfInt)
GENERATED_GETCOMPUTENODE(Round)

GENERATED_GETCOMPUTENODE(Lerp)
GENERATED_GETCOMPUTENODE(Clamp)

GENERATED_GETCOMPUTENODE(FAdd)
GENERATED_GETCOMPUTENODE(FSubstract)
GENERATED_GETCOMPUTENODE(FMultiply)
GENERATED_GETCOMPUTENODE(FDivide)

GENERATED_GETCOMPUTENODE(IAdd)
GENERATED_GETCOMPUTENODE(ISubstract)
GENERATED_GETCOMPUTENODE(IMultiply)
GENERATED_GETCOMPUTENODE(IDivide)

GENERATED_GETCOMPUTENODE(1MinusX)
GENERATED_GETCOMPUTENODE(Sqrt)
GENERATED_GETCOMPUTENODE(Pow)
GENERATED_GETCOMPUTENODE(IMod)
GENERATED_GETCOMPUTENODE(FAbs)
GENERATED_GETCOMPUTENODE(IAbs)

GENERATED_GETCOMPUTENODE(WorldGeneratorSampler)

GENERATED_GETCOMPUTENODE(MergeValues)
GENERATED_GETCOMPUTENODE(MergeMaterials)

GENERATED_GETCOMPUTENODE(BAnd)
GENERATED_GETCOMPUTENODE(BOr)
GENERATED_GETCOMPUTENODE(BNot)

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_MakeMaterial::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	Outputs[0].M.Index1 = FMath::Clamp<int>(Inputs[0].I, 0, 255);
	Outputs[0].M.Index2 = FMath::Clamp<int>(Inputs[1].I, 0, 255);
	Outputs[0].M.Alpha = FMath::Clamp<int>(255 * Inputs[2].F, 0, 255);
	Outputs[0].M.VoxelActor = FMath::Clamp<int>(Inputs[3].I, 0, 255);
}

void FVoxelComputeNode_MakeMaterial::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = FVoxelMaterial("));
	OutCpp.Append(TEXT("FMath::Clamp<int>(") + Inputs[0] + TEXT(", 0, 255), "));
	OutCpp.Append(TEXT("FMath::Clamp<int>(") + Inputs[1] + TEXT(", 0, 255), "));
	OutCpp.Append(TEXT("FMath::Clamp<int>(255 * ") + Inputs[2] + TEXT(", 0, 255), "));
	OutCpp.Append(TEXT("FMath::Clamp<int>(") + Inputs[3] + TEXT(", 0, 255));"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_MakeVoxelType::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	Outputs[0].VT = FVoxelType_internal(Inputs[0].VVT, Inputs[1].VMT);
}

void FVoxelComputeNode_MakeVoxelType::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = FVoxelType("));
	OutCpp.Append(Inputs[0] + TEXT(", "));
	OutCpp.Append(Inputs[1] + TEXT(");"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_BreakMaterial::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	Outputs[0].I = Inputs[0].M.Index1;
	Outputs[1].I = Inputs[0].M.Index1;
	Outputs[2].F = Inputs[0].M.Alpha;
}

void FVoxelComputeNode_BreakMaterial::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = ") + Inputs[0] + TEXT(".Index1;"));
	OutCpp.Append(Outputs[1] + TEXT(" = ") + Inputs[0] + TEXT(".Index2;"));
	OutCpp.Append(Outputs[2] + TEXT(" = ") + Inputs[0] + TEXT(".Alpha;"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_BreakVoxelType::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	Outputs[0].VVT = Inputs[0].VT.GetValueType();
	Outputs[1].VMT = Inputs[0].VT.GetMaterialType();
}

void FVoxelComputeNode_BreakVoxelType::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = ") + Inputs[0] + TEXT(".GetValueType();"));
	OutCpp.Append(Outputs[1] + TEXT(" = ") + Inputs[0] + TEXT(".GetMaterialType();"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_XF::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	Outputs[0].F = Context.X;
}

void FVoxelComputeNode_XF::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = ") + Context + TEXT(".X") + TEXT(";"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_YF::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	Outputs[0].F = Context.Y;
}

void FVoxelComputeNode_YF::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = ") + Context + TEXT(".Y") + TEXT(";"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_ZF::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	Outputs[0].F = Context.Z;
}

void FVoxelComputeNode_ZF::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = ") + Context + TEXT(".Z") + TEXT(";"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_XI::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	Outputs[0].I = Context.X;
}

void FVoxelComputeNode_XI::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = ") + Context + TEXT(".X") + TEXT(";"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_YI::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	Outputs[0].I = Context.Y;
}

void FVoxelComputeNode_YI::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = ") + Context + TEXT(".Y") + TEXT(";"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_ZI::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	Outputs[0].I = Context.Z;
}

void FVoxelComputeNode_ZI::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = ") + Context + TEXT(".Z") + TEXT(";"));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_FConstant::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	Outputs[0].F = Value;
}

void FVoxelComputeNode_FConstant::GetExposedVariables(TArray<FVoxelExposedVariable>& Variables) const
{
	if (bExposeToBP)
	{
		FVoxelExposedVariable V;
		V.VariableType = TEXT("float");
		V.VariableName = UniqueName;
		V.VariableDefaultValue = FString::SanitizeFloat(Value);
		Variables.Add(V);
	}
}

void FVoxelComputeNode_FConstant::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	if (bExposeToBP)
	{
		OutCpp.Append(Outputs[0] + TEXT(" = ") + UniqueName + TEXT(";"));
	}
	else
	{
		OutCpp.Append(Outputs[0] + TEXT(" = ") + FString::SanitizeFloat(Value) + TEXT(";"));
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_IConstant::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	Outputs[0].I = Value;
}

void FVoxelComputeNode_IConstant::GetExposedVariables(TArray<FVoxelExposedVariable>& Variables) const
{
	if (bExposeToBP)
	{
		FVoxelExposedVariable V;
		V.VariableType = TEXT("int");
		V.VariableName = UniqueName;
		V.VariableDefaultValue = FString::FromInt(Value);
		Variables.Add(V);
	}
}

void FVoxelComputeNode_IConstant::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	if (bExposeToBP)
	{
		OutCpp.Append(Outputs[0] + TEXT(" = ") + UniqueName + TEXT(";"));
	}
	else
	{
		OutCpp.Append(Outputs[0] + TEXT(" = ") + FString::FromInt(Value) + TEXT(";"));
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_FloatOfInt::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	Outputs[0].F = (float)Inputs[0].I;
}

void FVoxelComputeNode_FloatOfInt::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = (float)") + Inputs[0] + TEXT(";"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_Round::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	Outputs[0].I = FMath::RoundToInt(Inputs[0].F);
}

void FVoxelComputeNode_Round::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = FMath::RoundToInt(") + Inputs[0] + TEXT(");"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_Max::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	float X = Inputs[0].F;
	for (int i = 1; i < InputCount; i++)
	{
		X = FMath::Max(X, Inputs[i].F);
	}

	Outputs[0].F = X;
}

void FVoxelComputeNode_Max::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = "));
	for (int i = 0; i < InputCount - 1; i++)
	{
		OutCpp.Append(TEXT("FMath::Max<float>(") + Inputs[i] + TEXT(", "));
	}

	OutCpp.Append(Inputs[InputCount - 1]);

	for (int i = 0; i < InputCount - 1; i++)
	{
		OutCpp.Append(TEXT(")"));
	}
	OutCpp.Append(TEXT(";"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_Min::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	float X = Inputs[0].F;
	for (int i = 1; i < InputCount; i++)
	{
		X = FMath::Min(X, Inputs[i].F);
	}

	Outputs[0].F = X;
}

void FVoxelComputeNode_Min::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = "));
	for (int i = 0; i < InputCount - 1; i++)
	{
		OutCpp.Append(TEXT("FMath::Min<float>(") + Inputs[i] + TEXT(", "));
	}

	OutCpp.Append(Inputs[InputCount - 1]);

	for (int i = 0; i < InputCount - 1; i++)
	{
		OutCpp.Append(TEXT(")"));
	}
	OutCpp.Append(TEXT(";"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_Less::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	Outputs[0].B = Inputs[0].F < Inputs[1].F;
}

void FVoxelComputeNode_Less::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = ") + Inputs[0] + TEXT(" < ") + Inputs[1] + TEXT(";"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_LessEqual::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	Outputs[0].B = Inputs[0].F <= Inputs[1].F;
}

void FVoxelComputeNode_LessEqual::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = ") + Inputs[0] + TEXT(" <= ") + Inputs[1] + TEXT(";"));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_Greater::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	Outputs[0].B = Inputs[0].F > Inputs[1].F;
}

void FVoxelComputeNode_Greater::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = ") + Inputs[0] + TEXT(" > ") + Inputs[1] + TEXT(";"));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_GreaterEqual::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	Outputs[0].B = Inputs[0].F >= Inputs[1].F;
}

void FVoxelComputeNode_GreaterEqual::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = ") + Inputs[0] + TEXT(" >= ") + Inputs[1] + TEXT(";"));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_Lerp::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	Outputs[0].F = FMath::Lerp(Inputs[0].F, Inputs[1].F, Inputs[2].F);
}

void FVoxelComputeNode_Lerp::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = ") + TEXT("FMath::Lerp<float>(") + Inputs[0] + TEXT(", ") + Inputs[1] + TEXT(", ") + Inputs[2] + TEXT(");"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_Clamp::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	Outputs[0].F = FMath::Clamp(Inputs[0].F, Inputs[1].F, Inputs[2].F);
}

void FVoxelComputeNode_Clamp::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = ") + TEXT("FMath::Clamp<float>(") + Inputs[0] + TEXT(", ") + Inputs[1] + TEXT(", ") + Inputs[2] + TEXT(");"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_FAdd::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	float X = Inputs[0].F;
	for (int i = 1; i < InputCount; i++)
	{
		X += Inputs[i].F;
	}

	Outputs[0].F = X;
}

void FVoxelComputeNode_FAdd::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = "));
	for (int i = 0; i < InputCount - 1; i++)
	{
		OutCpp.Append(Inputs[i] + TEXT(" + "));
	}
	OutCpp.Append(Inputs[InputCount - 1] + TEXT(";"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_FSubstract::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	Outputs[0].F = Inputs[0].F - Inputs[1].F;
}

void FVoxelComputeNode_FSubstract::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = ") + Inputs[0] + TEXT(" - ") + Inputs[1] + TEXT(";"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_FMultiply::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	float X = Inputs[0].F;
	for (int i = 1; i < InputCount; i++)
	{
		X *= Inputs[i].F;
	}

	Outputs[0].F = X;
}

void FVoxelComputeNode_FMultiply::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = "));
	for (int i = 0; i < InputCount - 1; i++)
	{
		OutCpp.Append(Inputs[i] + TEXT(" * "));
	}
	OutCpp.Append(Inputs[InputCount - 1] + TEXT(";"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_FDivide::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	Outputs[0].F = Inputs[0].F / Inputs[1].F;
}

void FVoxelComputeNode_FDivide::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = ") + Inputs[0] + TEXT(" / ") + Inputs[1] + TEXT(";"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_IAdd::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	int32 X = Inputs[0].I;
	for (int i = 1; i < InputCount; i++)
	{
		X += Inputs[i].I;
	}

	Outputs[0].I = X;
}

void FVoxelComputeNode_IAdd::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = "));
	for (int i = 0; i < InputCount - 1; i++)
	{
		OutCpp.Append(Inputs[i] + TEXT(" + "));
	}
	OutCpp.Append(Inputs[InputCount - 1] + TEXT(";"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_ISubstract::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	Outputs[0].I = Inputs[0].I - Inputs[1].I;
}

void FVoxelComputeNode_ISubstract::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = ") + Inputs[0] + TEXT(" - ") + Inputs[1] + TEXT(";"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_IMultiply::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	int32 X = Inputs[0].I;
	for (int i = 1; i < InputCount; i++)
	{
		X *= Inputs[i].I;
	}

	Outputs[0].I = X;
}

void FVoxelComputeNode_IMultiply::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = "));
	for (int i = 0; i < InputCount - 1; i++)
	{
		OutCpp.Append(Inputs[i] + TEXT(" * "));
	}
	OutCpp.Append(Inputs[InputCount - 1] + TEXT(";"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_IDivide::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	Outputs[0].I = Inputs[0].I / Inputs[1].I;
}

void FVoxelComputeNode_IDivide::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = ") + Inputs[0] + TEXT(" / ") + Inputs[1] + TEXT(";"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_1MinusX::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	Outputs[0].F = 1 - Inputs[0].F;
}

void FVoxelComputeNode_1MinusX::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = 1 - ") + Inputs[0] + TEXT(";"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_Sqrt::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	Outputs[0].F = FMath::Sqrt(Inputs[0].F);
}

void FVoxelComputeNode_Sqrt::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = FMath::Sqrt(") + Inputs[0] + TEXT(");"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_Pow::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	Outputs[0].F = FMath::Pow(Inputs[0].F, Inputs[1].F);
}

void FVoxelComputeNode_Pow::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = FMath::Pow(") + Inputs[0] + TEXT(", ") + Inputs[1] + TEXT(");"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_IMod::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	Outputs[0].I = Inputs[0].I % Inputs[1].I;
}

void FVoxelComputeNode_IMod::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = ") + Inputs[0] + TEXT(" % ") + Inputs[1] + TEXT(";"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_FAbs::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	Outputs[0].F = FMath::Abs(Inputs[0].F);
}

void FVoxelComputeNode_FAbs::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = FMath::Abs(") + Inputs[0] + TEXT(");"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_IAbs::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	Outputs[0].I = FMath::Abs(Inputs[0].I);
}

void FVoxelComputeNode_IAbs::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = FMath::Abs(") + Inputs[0] + TEXT(");"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_WorldGeneratorSampler::Init(const AVoxelWorld* VoxelWorld)
{
	WorldGenerator->SetVoxelWorld(VoxelWorld);
}

void FVoxelComputeNode_WorldGeneratorSampler::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	float Value;
	FVoxelMaterial Material;
	FVoxelType Type;
	WorldGenerator->GetValueAndMaterialAndVoxelType(Inputs[0].F, Inputs[1].F, Inputs[2].F, Value, Material, Type);
	Outputs[0].F = Value;
	Outputs[1].M = Material;
	Outputs[2].VT = Type;
}

void FVoxelComputeNode_WorldGeneratorSampler::GetExposedVariables(TArray<FVoxelExposedVariable>& Variables) const
{
	FVoxelExposedVariable V;
	V.VariableType = TEXT("FVoxelWorldGeneratorPicker");
	V.VariableInstanceType = TEXT("TSharedRef<FVoxelWorldGeneratorInstance>");
	V.VariableName = WorldGeneratorUniqueName;
	V.VariableCustomAccessor = WorldGeneratorUniqueName + TEXT(".GetWorldGenerator()");
	Variables.Add(V);
}

void FVoxelComputeNode_WorldGeneratorSampler::GetSetVoxelWorld(const FString& VoxelWorld, FString& OutCpp) const
{
	OutCpp.Append(WorldGeneratorUniqueName + TEXT("->SetVoxelWorld(") + VoxelWorld + TEXT(");"));
}

void FVoxelComputeNode_WorldGeneratorSampler::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(WorldGeneratorUniqueName + TEXT("->GetValueAndMaterialAndVoxelType(") +
		Inputs[0] + TEXT(", ") + Inputs[1] + TEXT(", ") + Inputs[2] + TEXT(", ") +
		Outputs[0] + TEXT(", ") + Outputs[1] + TEXT(", ") + Outputs[2] + TEXT(");"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_MergeValues::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	Outputs[0].F =
		Inputs[1].VVT == EVoxelValueType::UseValue ?
		Inputs[0].F :
		Inputs[1].VVT == EVoxelValueType::IgnoreValue ?
		Inputs[2].F :
		FVoxelUtilities::HaveSameSign(Inputs[0].F, Inputs[2].F) ?
		Inputs[0].F :
		Inputs[2].F;
}

void FVoxelComputeNode_MergeValues::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = "));
	OutCpp.Append(Inputs[1] + TEXT(" == EVoxelValueType::UseValue ? "));
	OutCpp.Append(Inputs[0] + TEXT(":"));
	OutCpp.Append(Inputs[1] + TEXT(" == EVoxelValueType::IgnoreValue ? "));
	OutCpp.Append(Inputs[2] + TEXT(":"));
	OutCpp.Append(TEXT("FVoxelUtils::HaveSameSign(") + Inputs[0] + TEXT(", ") + Inputs[2] + TEXT(") ?"));
	OutCpp.Append(Inputs[0] + TEXT(":"));
	OutCpp.Append(Inputs[2] + TEXT(";"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_MergeMaterials::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	Outputs[0].M = Inputs[1].VMT == EVoxelMaterialType::UseMaterial ? Inputs[0].M : Inputs[2].M;
}

void FVoxelComputeNode_MergeMaterials::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = "));
	OutCpp.Append(Inputs[1] + TEXT(" == EVoxelMaterialType::UseMaterial ? "));
	OutCpp.Append(Inputs[0] + TEXT(":"));
	OutCpp.Append(Inputs[2] + TEXT(";"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_BAnd::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	bool X = Inputs[0].B;
	for (int i = 1; i < InputCount; i++)
	{
		X = X && Inputs[i].B;
	}

	Outputs[0].B = X;
}

void FVoxelComputeNode_BAnd::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = "));
	for (int i = 0; i < InputCount - 1; i++)
	{
		OutCpp.Append(Inputs[i] + TEXT(" && "));
	}
	OutCpp.Append(Inputs[InputCount - 1] + TEXT(";"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_BOr::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	bool X = Inputs[0].B;
	for (int i = 1; i < InputCount; i++)
	{
		X = X || Inputs[i].B;
	}

	Outputs[0].B = X;
}

void FVoxelComputeNode_BOr::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = "));
	for (int i = 0; i < InputCount - 1; i++)
	{
		OutCpp.Append(Inputs[i] + TEXT(" || "));
	}
	OutCpp.Append(Inputs[InputCount - 1] + TEXT(";"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelComputeNode_BNot::Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const
{
	Outputs[0].B = !Inputs[0].B;
}

void FVoxelComputeNode_BNot::GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const
{
	OutCpp.Append(Outputs[0] + TEXT(" = !") + Inputs[0] + TEXT(";"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
