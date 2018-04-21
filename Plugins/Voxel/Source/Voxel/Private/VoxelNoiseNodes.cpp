// Copyright 2018 Phyronnaz

#include "VoxelNoiseNodes.h"

#define LOCTEXT_NAMESPACE "VoxelNodes"

FText UVoxelNode_NoiseNode::GetInputPinName(int32 PinIndex) const
{
	if (PinIndex == 0)
	{
		return LOCTEXT("X", "X");
	}
	else if (PinIndex == 1)
	{
		return LOCTEXT("Y", "Y");
	}
	else if (PinIndex == 2)
	{
		return LOCTEXT("Z", "Z");
	}
	else
	{
		return LOCTEXT("", "");
	}
}

FVoxelComputeNode_NoiseNode::FVoxelComputeNode_NoiseNode(const UVoxelNode_NoiseNode* Node)
	: FVoxelComputeNode(Node)
	, Scale(Node->Scale)
	, NoiseName(Node->GetName() + TEXT("___Noise___") + FString::FromInt(FMath::Rand()))
	, Seed(Node->Seed)
	, Frequency(Node->Frequency)
	, Interpolation(Node->Interpolation)
{

}

void FVoxelComputeNode_NoiseNode::Init(const AVoxelWorld* VoxelWorld)
{
	Noise.SetSeed(Seed);
	Noise.SetFrequency(Frequency);
	Noise.SetInterp((FastNoise::Interp)Interpolation);
}

void FVoxelComputeNode_NoiseNode::GetAdditionalHeaders(TArray<FString>& OutAdditionalHeaders) const
{
	OutAdditionalHeaders.Add(TEXT("\"FastNoise.h\""));
}

void FVoxelComputeNode_NoiseNode::GetVariables(TArray<FVoxelVariable>& Variables) const
{
	Variables.Add(FVoxelVariable(TEXT("FastNoise"), NoiseName));
}

void FVoxelComputeNode_NoiseNode::GetSetVoxelWorld(const FString& VoxelWorld, FString& OutCpp) const
{
	OutCpp.Append(NoiseName + TEXT(".SetSeed(" + FString::FromInt(Seed) + TEXT(");\n")));
	OutCpp.Append(NoiseName + TEXT(".SetFrequency(" + FString::SanitizeFloat(Frequency) + TEXT(");\n")));
	OutCpp.Append(NoiseName + TEXT(".SetInterp((FastNoise::Interp)" + FString::FromInt((int)Interpolation) + TEXT(");\n")));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

FVoxelComputeNode_NoiseNodeFractal::FVoxelComputeNode_NoiseNodeFractal(const UVoxelNode_NoiseNodeFractal* Node)
	: FVoxelComputeNode_NoiseNode(Node)
	, FractalOctaves(Node->FractalOctaves)
	, FractalLacunarity(Node->FractalLacunarity)
	, FractalGain(Node->FractalGain)
	, FractalType(Node->FractalType)
{

}

void FVoxelComputeNode_NoiseNodeFractal::Init(const AVoxelWorld* VoxelWorld)
{
	FVoxelComputeNode_NoiseNode::Init(VoxelWorld);
	Noise.SetFractalOctaves(FractalOctaves);
	Noise.SetFractalLacunarity(FractalLacunarity);
	Noise.SetFractalGain(FractalGain);
	Noise.SetFractalType((FastNoise::FractalType) FractalType);
}

void FVoxelComputeNode_NoiseNodeFractal::GetSetVoxelWorld(const FString& VoxelWorld, FString& OutCpp) const
{
	FVoxelComputeNode_NoiseNode::GetSetVoxelWorld(VoxelWorld, OutCpp);

	OutCpp.Append(NoiseName + TEXT(".SetFractalOctaves(" + FString::FromInt(FractalOctaves) + TEXT(");\n")));
	OutCpp.Append(NoiseName + TEXT(".SetFractalLacunarity(" + FString::SanitizeFloat(FractalLacunarity) + TEXT(");\n")));
	OutCpp.Append(NoiseName + TEXT(".SetFractalGain(" + FString::SanitizeFloat(FractalGain) + TEXT(");\n")));
	OutCpp.Append(NoiseName + TEXT(".SetFractalType((FastNoise::FractalType)" + FString::FromInt((int)FractalType) + TEXT(");\n")));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

FVoxelComputeNode_GradientPerturb::FVoxelComputeNode_GradientPerturb(const UVoxelNode_GradientPerturb* Node)
	: FVoxelComputeNode_NoiseNode(Node)
	, GradientPerturbAmplitude(Node->GradientPerturbAmplitude)
{

}

void FVoxelComputeNode_GradientPerturb::Init(const AVoxelWorld* VoxelWorld)
{
	FVoxelComputeNode_NoiseNode::Init(VoxelWorld);

	Noise.SetGradientPerturbAmp(GradientPerturbAmplitude);
}

void FVoxelComputeNode_GradientPerturb::GetSetVoxelWorld(const FString& VoxelWorld, FString& OutCpp) const
{
	FVoxelComputeNode_NoiseNode::GetSetVoxelWorld(VoxelWorld, OutCpp);

	OutCpp.Append(NoiseName + TEXT(".SetGradientPerturbAmp(" + FString::SanitizeFloat(GradientPerturbAmplitude) + TEXT(");\n")));
}

//////////////////////////////////////////////////////////////////////////////////////

FVoxelComputeNode_GradientPerturbFractal::FVoxelComputeNode_GradientPerturbFractal(const UVoxelNode_GradientPerturbFractal* Node)
	: FVoxelComputeNode_NoiseNodeFractal(Node)
	, GradientPerturbAmplitude(Node->GradientPerturbAmplitude)
{

}

void FVoxelComputeNode_GradientPerturbFractal::Init(const AVoxelWorld* VoxelWorld)
{
	FVoxelComputeNode_NoiseNodeFractal::Init(VoxelWorld);

	Noise.SetGradientPerturbAmp(GradientPerturbAmplitude);
}

void FVoxelComputeNode_GradientPerturbFractal::GetSetVoxelWorld(const FString& VoxelWorld, FString& OutCpp) const
{
	FVoxelComputeNode_NoiseNodeFractal::GetSetVoxelWorld(VoxelWorld, OutCpp);

	OutCpp.Append(NoiseName + TEXT(".SetGradientPerturbAmp(" + FString::SanitizeFloat(GradientPerturbAmplitude) + TEXT(");\n")));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

GENERATED_GETCOMPUTENODE(2DValueNoise)

GENERATED_GETCOMPUTENODE(2DValueNoiseFractal)

GENERATED_GETCOMPUTENODE(2DPerlinNoise)

GENERATED_GETCOMPUTENODE(2DPerlinNoiseFractal)

GENERATED_GETCOMPUTENODE(2DSimplexNoise)

GENERATED_GETCOMPUTENODE(2DSimplexNoiseFractal)

GENERATED_GETCOMPUTENODE(2DCubicNoise)

GENERATED_GETCOMPUTENODE(2DCubicNoiseFractal)

GENERATED_GETCOMPUTENODE(2DWhiteNoise)

///////////////////////////////////////////////////////////////////////////////////////////////////////////

GENERATED_GETCOMPUTENODE(3DValueNoise)

GENERATED_GETCOMPUTENODE(3DValueNoiseFractal)

GENERATED_GETCOMPUTENODE(3DPerlinNoise)

GENERATED_GETCOMPUTENODE(3DPerlinNoiseFractal)

GENERATED_GETCOMPUTENODE(3DSimplexNoise)

GENERATED_GETCOMPUTENODE(3DSimplexNoiseFractal)

GENERATED_GETCOMPUTENODE(3DCubicNoise)

GENERATED_GETCOMPUTENODE(3DCubicNoiseFractal)

GENERATED_GETCOMPUTENODE(3DWhiteNoise)

///////////////////////////////////////////////////////////////////////////////////////////////////////////

GENERATED_GETCOMPUTENODE(2DCellularNoise)
GENERATED_GETCOMPUTENODE(3DCellularNoise)

///////////////////////////////////////////////////////////////////////////////////////////////////////////

GENERATED_GETCOMPUTENODE(2DGradientPerturb)

GENERATED_GETCOMPUTENODE(2DGradientPerturbFractal)

GENERATED_GETCOMPUTENODE(3DGradientPerturb)

GENERATED_GETCOMPUTENODE(3DGradientPerturbFractal)

#undef LOCTEXT_NAMESPACE
