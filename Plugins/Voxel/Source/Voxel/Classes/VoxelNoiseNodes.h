// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelNode.h"
#include "FastNoise.h"
#include "VoxelDefaultNodes.h"
#include "VoxelNoiseNodes.generated.h"

UENUM(BlueprintType)
enum class EFractalType : uint8
{
	FBM,
	Billow,
	RigidMulti
};

UENUM(BlueprintType)
enum class EInterp : uint8
{
	Linear,
	Hermite,
	Quintic
};

UENUM(BlueprintType)
enum class ECellularDistanceFunction : uint8
{
	Euclidean,
	Manhattan,
	Natural
};

UENUM(BlueprintType)
enum class ECellularReturnType : uint8
{
	CellValue UMETA(Hidden),
	NoiseLookup UMETA(Hidden),
	Distance, 
	Distance2, 
	Distance2Add, 
	Distance2Sub, 
	Distance2Mul,
	Distance2Div
};


UCLASS(abstract)
class VOXEL_API UVoxelNode_NoiseNode : public UVoxelNode
{
	GENERATED_BODY()

public:
	// Position = Input / Scale
	UPROPERTY(EditAnywhere)
	float Scale = 1;

	UPROPERTY(EditAnywhere)
	int Seed = 1337;

	UPROPERTY(EditAnywhere)
	float Frequency = 0.02;

	UPROPERTY(EditAnywhere)
	EInterp Interpolation = EInterp::Quintic;

	FText GetInputPinName(int32 PinIndex) const override;
};

class FVoxelComputeNode_NoiseNode : public FVoxelComputeNode
{
public:
	FVoxelComputeNode_NoiseNode(const UVoxelNode_NoiseNode* Node);


	void Init(const AVoxelWorld* VoxelWorld) override;

	void GetAdditionalHeaders(TArray<FString>& OutAdditionalHeaders) const override;
	void GetVariables(TArray<FVoxelVariable>& Variables) const override;
	void GetSetVoxelWorld(const FString& VoxelWorld, FString& OutCpp) const override;

protected:
	const float Scale;
	FString const NoiseName;
	FastNoise Noise;

private:
	const int Seed;
	const float Frequency;
	const EInterp Interpolation;
};

//////////////////////////////////////////////////////////////////////////////////////

UCLASS(abstract)
class VOXEL_API UVoxelNode_NoiseNodeFractal : public UVoxelNode_NoiseNode
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	int FractalOctaves = 3;

	UPROPERTY(EditAnywhere)
	float FractalLacunarity = 2;

	UPROPERTY(EditAnywhere)
	float FractalGain = 0.5;

	UPROPERTY(EditAnywhere)
	EFractalType FractalType = EFractalType::FBM;
};


class FVoxelComputeNode_NoiseNodeFractal : public FVoxelComputeNode_NoiseNode
{
public:
	FVoxelComputeNode_NoiseNodeFractal(const UVoxelNode_NoiseNodeFractal* Node);

	void Init(const AVoxelWorld* VoxelWorld) override;
	void GetSetVoxelWorld(const FString& VoxelWorld, FString& OutCpp) const override;

private:
	int FractalOctaves;
	float FractalLacunarity;
	float FractalGain;
	EFractalType FractalType;
};

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(abstract)
class VOXEL_API UVoxelNode_GradientPerturb : public UVoxelNode_NoiseNode
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
		float GradientPerturbAmplitude = 1;
};


class FVoxelComputeNode_GradientPerturb : public FVoxelComputeNode_NoiseNode
{
public:
	FVoxelComputeNode_GradientPerturb(const UVoxelNode_GradientPerturb* Node);

	void Init(const AVoxelWorld* VoxelWorld) override;
	void GetSetVoxelWorld(const FString& VoxelWorld, FString& OutCpp) const override;

private:
	float GradientPerturbAmplitude;
};

//////////////////////////////////////////////////////////////////////////////////////

UCLASS(abstract)
class VOXEL_API UVoxelNode_GradientPerturbFractal : public UVoxelNode_NoiseNodeFractal
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	float GradientPerturbAmplitude = 1;
};


class FVoxelComputeNode_GradientPerturbFractal : public FVoxelComputeNode_NoiseNodeFractal
{
public:
	FVoxelComputeNode_GradientPerturbFractal(const UVoxelNode_GradientPerturbFractal* Node);

	void Init(const AVoxelWorld* VoxelWorld) override;
	void GetSetVoxelWorld(const FString& VoxelWorld, FString& OutCpp) const override;

private:
	float GradientPerturbAmplitude;
};

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

#define GENERATE_2D_NOISE_COMPUTENODE_AUX(FunctionName, CppName, Fractal)\
class FVoxelComputeNode_##CppName : public FVoxelComputeNode_NoiseNode##Fractal\
{\
public:\
	FVoxelComputeNode_##CppName(const UVoxelNode_##CppName* Node) : FVoxelComputeNode_NoiseNode##Fractal(Node) {}\
\
	void Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const override\
	{\
		Outputs[0].F = Noise.FunctionName(Inputs[0].F / Scale, Inputs[1].F / Scale);\
	}\
\
	void GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const override\
	{\
		FString ScaleName = FString::SanitizeFloat(Scale);\
\
		OutCpp.Append(Outputs[0] + TEXT(" = "));\
		OutCpp.Append(NoiseName + TEXT(".") + #FunctionName + TEXT("("));\
		OutCpp.Append(Inputs[0] + TEXT(" / ") + ScaleName + TEXT(", ")\
				    + Inputs[1] + TEXT(" / ") + ScaleName + TEXT(");"));\
	}\
};

#define GENERATE_2D_NOISE_COMPUTENODE(FunctionName, CppName) GENERATE_2D_NOISE_COMPUTENODE_AUX(FunctionName, CppName,)
#define GENERATE_2D_NOISE_COMPUTENODE_FRACTAL(FunctionName, CppName) GENERATE_2D_NOISE_COMPUTENODE_AUX(FunctionName, CppName, Fractal)

//////////////////////////////////////////////////////////////////////////////////////

#define GENERATE_3D_NOISE_COMPUTENODE_AUX(FunctionName, CppName, Fractal)\
class FVoxelComputeNode_##CppName : public FVoxelComputeNode_NoiseNode##Fractal\
{\
public:\
	FVoxelComputeNode_##CppName(const UVoxelNode_##CppName* Node) : FVoxelComputeNode_NoiseNode##Fractal(Node) {}\
\
	void Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const override\
	{\
		Outputs[0].F = Noise.FunctionName(Inputs[0].F / Scale, Inputs[1].F / Scale, Inputs[2].F / Scale);\
	}\
	void GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const override\
	{\
		FString ScaleName = FString::SanitizeFloat(Scale);\
\
		OutCpp.Append(Outputs[0] + TEXT(" = "));\
		OutCpp.Append(NoiseName + TEXT(".") + #FunctionName + TEXT("("));\
		OutCpp.Append(Inputs[0] + TEXT(" / ") + ScaleName + TEXT(", ")\
				    + Inputs[1] + TEXT(" / ") + ScaleName + TEXT(", ")\
					+ Inputs[2] + TEXT(" / ") + ScaleName + TEXT(");"));\
	}\
};

#define GENERATE_3D_NOISE_COMPUTENODE(FunctionName, CppName) GENERATE_3D_NOISE_COMPUTENODE_AUX(FunctionName, CppName,)
#define GENERATE_3D_NOISE_COMPUTENODE_FRACTAL(FunctionName, CppName) GENERATE_3D_NOISE_COMPUTENODE_AUX(FunctionName, CppName, Fractal)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "2D Value Noise"))
class VOXEL_API UVoxelNode_2DValueNoise : public UVoxelNode_NoiseNode
{
	GENERATED_BODY()
public:
	GENERATED_NODE_BODY(2, 2, 1)
};

GENERATE_2D_NOISE_COMPUTENODE(GetValue, 2DValueNoise)

UCLASS(meta = (DisplayName = "2D Value Noise Fractal"))
class VOXEL_API UVoxelNode_2DValueNoiseFractal : public UVoxelNode_NoiseNodeFractal
{
	GENERATED_BODY()
public:
	GENERATED_NODE_BODY(2, 2, 1)
};

GENERATE_2D_NOISE_COMPUTENODE_FRACTAL(GetValueFractal, 2DValueNoiseFractal)

//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "2D Perlin Noise"))
class VOXEL_API UVoxelNode_2DPerlinNoise : public UVoxelNode_NoiseNode
{
	GENERATED_BODY()
public:
	GENERATED_NODE_BODY(2, 2, 1)
};

GENERATE_2D_NOISE_COMPUTENODE(GetPerlin, 2DPerlinNoise)

UCLASS(meta = (DisplayName = "2D Perlin Noise Fractal"))
class VOXEL_API UVoxelNode_2DPerlinNoiseFractal : public UVoxelNode_NoiseNodeFractal
{
	GENERATED_BODY()
public:
	GENERATED_NODE_BODY(2, 2, 1)
};

GENERATE_2D_NOISE_COMPUTENODE_FRACTAL(GetPerlinFractal, 2DPerlinNoiseFractal)

//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "2D Simplex Noise"))
class VOXEL_API UVoxelNode_2DSimplexNoise : public UVoxelNode_NoiseNode
{
	GENERATED_BODY()
public:
	GENERATED_NODE_BODY(2, 2, 1)
};

GENERATE_2D_NOISE_COMPUTENODE(GetSimplex, 2DSimplexNoise)

UCLASS(meta = (DisplayName = "2D Simplex Noise Fractal"))
class VOXEL_API UVoxelNode_2DSimplexNoiseFractal : public UVoxelNode_NoiseNodeFractal
{
	GENERATED_BODY()
public:
	GENERATED_NODE_BODY(2, 2, 1)
};

GENERATE_2D_NOISE_COMPUTENODE_FRACTAL(GetSimplexFractal, 2DSimplexNoiseFractal)

//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "2D Cubic Noise"))
class VOXEL_API UVoxelNode_2DCubicNoise : public UVoxelNode_NoiseNode
{
	GENERATED_BODY()
public:
	GENERATED_NODE_BODY(2, 2, 1)
};

GENERATE_2D_NOISE_COMPUTENODE(GetCubic, 2DCubicNoise)

UCLASS(meta = (DisplayName = "2D Cubic Noise Fractal"))
class VOXEL_API UVoxelNode_2DCubicNoiseFractal : public UVoxelNode_NoiseNodeFractal
{
	GENERATED_BODY()
public:
	GENERATED_NODE_BODY(2, 2, 1)
};

GENERATE_2D_NOISE_COMPUTENODE_FRACTAL(GetCubicFractal, 2DCubicNoiseFractal)

//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "2D White Noise"))
class VOXEL_API UVoxelNode_2DWhiteNoise : public UVoxelNode_NoiseNode
{
	GENERATED_BODY()
public:
	GENERATED_NODE_BODY(2, 2, 1)
};

GENERATE_2D_NOISE_COMPUTENODE(GetWhiteNoise, 2DWhiteNoise)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "3D Value Noise"))
class VOXEL_API UVoxelNode_3DValueNoise : public UVoxelNode_NoiseNode
{
	GENERATED_BODY()
public:
	GENERATED_NODE_BODY(3, 3, 1)
};

GENERATE_3D_NOISE_COMPUTENODE(GetValue, 3DValueNoise)

UCLASS(meta = (DisplayName = "3D Value Noise Fractal"))
class VOXEL_API UVoxelNode_3DValueNoiseFractal : public UVoxelNode_NoiseNodeFractal
{
	GENERATED_BODY()
public:
	GENERATED_NODE_BODY(3, 3, 1)
};

GENERATE_3D_NOISE_COMPUTENODE_FRACTAL(GetValueFractal, 3DValueNoiseFractal)

//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "3D Perlin Noise"))
class VOXEL_API UVoxelNode_3DPerlinNoise : public UVoxelNode_NoiseNode
{
	GENERATED_BODY()
public:
	GENERATED_NODE_BODY(3, 3, 1)
};

GENERATE_3D_NOISE_COMPUTENODE(GetPerlin, 3DPerlinNoise)

UCLASS(meta = (DisplayName = "3D Perlin Noise Fractal"))
class VOXEL_API UVoxelNode_3DPerlinNoiseFractal : public UVoxelNode_NoiseNodeFractal
{
	GENERATED_BODY()
public:
	GENERATED_NODE_BODY(3, 3, 1)
};

GENERATE_3D_NOISE_COMPUTENODE_FRACTAL(GetPerlinFractal, 3DPerlinNoiseFractal)

//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "3D Simplex Noise"))
class VOXEL_API UVoxelNode_3DSimplexNoise : public UVoxelNode_NoiseNode
{
	GENERATED_BODY()
public:
	GENERATED_NODE_BODY(3, 3, 1)
};

GENERATE_3D_NOISE_COMPUTENODE(GetSimplex, 3DSimplexNoise)

UCLASS(meta = (DisplayName = "3D Simplex Noise Fractal"))
class VOXEL_API UVoxelNode_3DSimplexNoiseFractal : public UVoxelNode_NoiseNodeFractal
{
	GENERATED_BODY()
public:
	GENERATED_NODE_BODY(3, 3, 1)
};

GENERATE_3D_NOISE_COMPUTENODE_FRACTAL(GetSimplexFractal, 3DSimplexNoiseFractal)

//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "3D Cubic Noise"))
class VOXEL_API UVoxelNode_3DCubicNoise : public UVoxelNode_NoiseNode
{
	GENERATED_BODY()
public:
	GENERATED_NODE_BODY(3, 3, 1)
};

GENERATE_3D_NOISE_COMPUTENODE(GetCubic, 3DCubicNoise)

UCLASS(meta = (DisplayName = "3D Cubic Noise Fractal"))
class VOXEL_API UVoxelNode_3DCubicNoiseFractal : public UVoxelNode_NoiseNodeFractal
{
	GENERATED_BODY()
public:
	GENERATED_NODE_BODY(3, 3, 1)
};

GENERATE_3D_NOISE_COMPUTENODE_FRACTAL(GetCubicFractal, 3DCubicNoiseFractal)

//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "3D White Noise"))
class VOXEL_API UVoxelNode_3DWhiteNoise : public UVoxelNode_NoiseNode
{
	GENERATED_BODY()
public:
	GENERATED_NODE_BODY(3, 3, 1)
};

GENERATE_3D_NOISE_COMPUTENODE(GetWhiteNoise, 3DWhiteNoise)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "2D Cellular Noise"))
class VOXEL_API UVoxelNode_2DCellularNoise : public UVoxelNode_NoiseNode
{
	GENERATED_BODY()
public:
	GENERATED_NODE_BODY(2, 2, 1)
			
	UPROPERTY(EditAnywhere)
	ECellularDistanceFunction DistanceFunction;
	
	UPROPERTY(EditAnywhere)
	ECellularReturnType ReturnType;
	
	UPROPERTY(EditAnywhere)
	float Jitter = 0.45;
};

class FVoxelComputeNode_2DCellularNoise : public FVoxelComputeNode_NoiseNode
{
public:
	FVoxelComputeNode_2DCellularNoise(const UVoxelNode_2DCellularNoise* Node)
		: DistanceFunction(Node->DistanceFunction)
		, ReturnType(Node->ReturnType)
		, Jitter(Node->Jitter)
		, FVoxelComputeNode_NoiseNode(Node)
	{
	}

	void Init(const AVoxelWorld* VoxelWorld) override
	{
		FVoxelComputeNode_NoiseNode::Init(VoxelWorld);

		Noise.SetCellularDistanceFunction((FastNoise::CellularDistanceFunction)DistanceFunction);
		Noise.SetCellularReturnType((FastNoise::CellularReturnType)ReturnType);
		Noise.SetCellularJitter(Jitter);
	}
	void Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const override
	{
		Outputs[0].F = Noise.GetCellular(Inputs[0].F / Scale, Inputs[1].F / Scale);
	}
	void GetSetVoxelWorld(const FString& VoxelWorld, FString& OutCpp) const override
	{
		FVoxelComputeNode_NoiseNode::GetSetVoxelWorld(VoxelWorld, OutCpp);
		
		OutCpp.Append(NoiseName + TEXT(".SetCellularDistanceFunction((FastNoise::CellularDistanceFunction)" + FString::FromInt((int)DistanceFunction) + TEXT(");\n")));
		OutCpp.Append(NoiseName + TEXT(".SetCellularReturnType((FastNoise::CellularReturnType)" + FString::FromInt((int)ReturnType) + TEXT(");\n")));
		OutCpp.Append(NoiseName + TEXT(".SetCellularJitter(") + FString::SanitizeFloat(Jitter) + TEXT(");\n"));
	}
	void GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const override
	{
		FString ScaleName = FString::SanitizeFloat(Scale);
		FString DivideByScale = TEXT(" / ") + ScaleName;

		OutCpp.Append(Outputs[0] + TEXT(" = "));

		OutCpp.Append(NoiseName + TEXT(".GetCellular(") + Inputs[0] + DivideByScale + TEXT(", ") + Inputs[1] + DivideByScale + TEXT(");"));
	}

private:
	const ECellularDistanceFunction DistanceFunction;
	const ECellularReturnType ReturnType;
	const float Jitter;
};

//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "3D Cellular Noise"))
class VOXEL_API UVoxelNode_3DCellularNoise : public UVoxelNode_NoiseNode
{
	GENERATED_BODY()
public:
	GENERATED_NODE_BODY(3, 3, 1)
			
	UPROPERTY(EditAnywhere)
	ECellularDistanceFunction DistanceFunction;
	
	UPROPERTY(EditAnywhere)
	ECellularReturnType ReturnType;
	
	UPROPERTY(EditAnywhere)
	float Jitter = 0.45;
};

class FVoxelComputeNode_3DCellularNoise : public FVoxelComputeNode_NoiseNode
{
public:
	FVoxelComputeNode_3DCellularNoise (const UVoxelNode_3DCellularNoise* Node)
		: DistanceFunction(Node->DistanceFunction)
		, ReturnType(Node->ReturnType)
		, Jitter(Node->Jitter)
		, FVoxelComputeNode_NoiseNode(Node)
	{
	}

	void Init(const AVoxelWorld* VoxelWorld) override
	{
		FVoxelComputeNode_NoiseNode::Init(VoxelWorld);

		Noise.SetCellularDistanceFunction((FastNoise::CellularDistanceFunction)DistanceFunction);
		Noise.SetCellularReturnType((FastNoise::CellularReturnType)ReturnType);
		Noise.SetCellularJitter(Jitter);
	}
	void Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const override
	{
		Outputs[0].F = Noise.GetCellular(Inputs[0].F / Scale, Inputs[1].F / Scale, Inputs[2].F);
	}
	void GetSetVoxelWorld(const FString& VoxelWorld, FString& OutCpp) const override
	{
		FVoxelComputeNode_NoiseNode::GetSetVoxelWorld(VoxelWorld, OutCpp);
		
		OutCpp.Append(NoiseName + TEXT(".SetCellularDistanceFunction((FastNoise::CellularDistanceFunction)" + FString::FromInt((int)DistanceFunction) + TEXT(");\n")));
		OutCpp.Append(NoiseName + TEXT(".SetCellularReturnType((FastNoise::CellularReturnType)" + FString::FromInt((int)ReturnType) + TEXT(");\n")));
		OutCpp.Append(NoiseName + TEXT(".SetCellularJitter(") + FString::SanitizeFloat(Jitter) + TEXT(");\n"));
	}
	void GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const override
	{
		FString ScaleName = FString::SanitizeFloat(Scale);
		FString DivideByScale = TEXT(" / ") + ScaleName;

		OutCpp.Append(Outputs[0] + TEXT(" = "));

		OutCpp.Append(NoiseName + TEXT(".GetCellular(") + Inputs[0] + DivideByScale + TEXT(", ") + Inputs[1] + DivideByScale + TEXT(", ") + Inputs[2] + DivideByScale + TEXT(");"));
	}

private:
	const ECellularDistanceFunction DistanceFunction;
	const ECellularReturnType ReturnType;
	const float Jitter;
};

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "2D Gradient Perturb"))
class VOXEL_API UVoxelNode_2DGradientPerturb : public UVoxelNode_GradientPerturb
{
	GENERATED_BODY()
public:
	GENERATED_NODE_BODY(2, 2, 2)
};


class FVoxelComputeNode_2DGradientPerturb : public FVoxelComputeNode_GradientPerturb
{
public:
	FVoxelComputeNode_2DGradientPerturb(const UVoxelNode_2DGradientPerturb* Node)
		: FVoxelComputeNode_GradientPerturb(Node)
	{
	}

	void Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const override
	{
		Outputs[0].F = Inputs[0].F / Scale;
		Outputs[1].F = Inputs[1].F / Scale;
		Noise.GradientPerturb(Outputs[0].F, Outputs[1].F);
	}
	void GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const override
	{
		FString ScaleName = FString::SanitizeFloat(Scale);

		OutCpp.Append(Outputs[0] + TEXT(" = ") + Inputs[0] + TEXT("/") + ScaleName + TEXT(";\n"));
		OutCpp.Append(Outputs[1] + TEXT(" = ") + Inputs[1] + TEXT("/") + ScaleName + TEXT(";\n"));

		OutCpp.Append(NoiseName + TEXT(".GradientPerturb(") + Inputs[0] + TEXT(", ") + Inputs[1] + TEXT(");"));
	}
};

//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "2D Gradient Perturb Fractal"))
class VOXEL_API UVoxelNode_2DGradientPerturbFractal : public UVoxelNode_GradientPerturbFractal
{
	GENERATED_BODY()
public:
	GENERATED_NODE_BODY(2, 2, 2)
};


class FVoxelComputeNode_2DGradientPerturbFractal : public FVoxelComputeNode_GradientPerturbFractal
{
public:
	FVoxelComputeNode_2DGradientPerturbFractal(const UVoxelNode_2DGradientPerturbFractal* Node)
		: FVoxelComputeNode_GradientPerturbFractal(Node)
	{
	}

	void Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const override
	{
		Outputs[0].F = Inputs[0].F / Scale;
		Outputs[1].F = Inputs[1].F / Scale;
		Noise.GradientPerturbFractal(Outputs[0].F, Outputs[1].F);
	}
	void GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const override
	{
		FString ScaleName = FString::SanitizeFloat(Scale);

		OutCpp.Append(Outputs[0] + TEXT(" = ") + Inputs[0] + TEXT("/") + ScaleName + TEXT(";\n"));
		OutCpp.Append(Outputs[1] + TEXT(" = ") + Inputs[1] + TEXT("/") + ScaleName + TEXT(";\n"));

		OutCpp.Append(NoiseName + TEXT(".GradientPerturbFractal(") + Inputs[0] + TEXT(", ") + Inputs[1] + TEXT(");"));
	}
};

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "3D Gradient Perturb"))
class VOXEL_API UVoxelNode_3DGradientPerturb : public UVoxelNode_GradientPerturb
{
	GENERATED_BODY()
public:
	GENERATED_NODE_BODY(3, 3, 3)
};


class FVoxelComputeNode_3DGradientPerturb : public FVoxelComputeNode_GradientPerturb
{
public:
	FVoxelComputeNode_3DGradientPerturb(const UVoxelNode_3DGradientPerturb* Node)
		: FVoxelComputeNode_GradientPerturb(Node)
	{
	}

	void Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const override
	{
		Outputs[0].F = Inputs[0].F / Scale;
		Outputs[1].F = Inputs[1].F / Scale;
		Outputs[2].F = Inputs[2].F / Scale;
		Noise.GradientPerturb(Outputs[0].F, Outputs[1].F, Outputs[2].F);
	}
	void GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const override
	{
		FString ScaleName = FString::SanitizeFloat(Scale);

		OutCpp.Append(Outputs[0] + TEXT(" = ") + Inputs[0] + TEXT("/") + ScaleName + TEXT(";\n"));
		OutCpp.Append(Outputs[1] + TEXT(" = ") + Inputs[1] + TEXT("/") + ScaleName + TEXT(";\n"));
		OutCpp.Append(Outputs[2] + TEXT(" = ") + Inputs[2] + TEXT("/") + ScaleName + TEXT(";\n"));

		OutCpp.Append(NoiseName + TEXT(".GradientPerturb(") + Inputs[0] + TEXT(", ") + Inputs[1] + TEXT(", ") + Inputs[2] + TEXT(");"));
	}
};

//////////////////////////////////////////////////////////////////////////////////////

UCLASS(meta = (DisplayName = "3D Gradient Perturb Fractal"))
class VOXEL_API UVoxelNode_3DGradientPerturbFractal : public UVoxelNode_GradientPerturbFractal
{
	GENERATED_BODY()
public:
	GENERATED_NODE_BODY(3, 3, 3)
};


class FVoxelComputeNode_3DGradientPerturbFractal : public FVoxelComputeNode_GradientPerturbFractal
{
public:
	FVoxelComputeNode_3DGradientPerturbFractal(const UVoxelNode_3DGradientPerturbFractal* Node)
		: FVoxelComputeNode_GradientPerturbFractal(Node)
	{
	}

	void Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const override
	{
		Outputs[0].F = Inputs[0].F / Scale;
		Outputs[1].F = Inputs[1].F / Scale;
		Outputs[2].F = Inputs[2].F / Scale;
		Noise.GradientPerturb(Outputs[0].F, Outputs[1].F, Outputs[2].F);
	}
	void GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const override
	{
		FString ScaleName = FString::SanitizeFloat(Scale);

		OutCpp.Append(Outputs[0] + TEXT(" = ") + Inputs[0] + TEXT("/") + ScaleName + TEXT(";\n"));
		OutCpp.Append(Outputs[1] + TEXT(" = ") + Inputs[1] + TEXT("/") + ScaleName + TEXT(";\n"));
		OutCpp.Append(Outputs[2] + TEXT(" = ") + Inputs[2] + TEXT("/") + ScaleName + TEXT(";\n"));

		OutCpp.Append(NoiseName + TEXT(".GradientPerturbFractal(") + Inputs[0] + TEXT(", ") + Inputs[1] + TEXT(", ") + Inputs[2] + TEXT(");"));
	}
};
