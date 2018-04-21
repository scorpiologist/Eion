// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelAsset.h"
#include "VoxelMaterial.h"
#include "EdGraph/EdGraphNode.h"
#include "VoxelNode.generated.h"

#define MAX_PINS 64

class UVoxelNode;
class FVoxelComputeNode;

struct FVoxelContext
{
	int32 X;
	int32 Y;
	int32 Z;
};

// Can be used with union
struct FVoxelMaterial_internal
{
	uint8 Index1;
	uint8 Index2;
	uint8 Alpha;
	uint8 VoxelActor;

	FVoxelMaterial_internal() = default;
	FVoxelMaterial_internal(const FVoxelMaterial& Material) : Index1(Material.Index1), Index2(Material.Index2), Alpha(Material.Alpha), VoxelActor(Material.VoxelActor) {}
	FVoxelMaterial_internal(uint8 Index1, uint8 Index2, uint8 Alpha, uint8 VoxelActor) : Index1(Index1), Index2(Index2), Alpha(Alpha), VoxelActor(VoxelActor) {}
	operator FVoxelMaterial() const { return FVoxelMaterial(Index1, Index2, Alpha, VoxelActor); }
};

// Can be used with union
struct FVoxelType_internal
{
	uint8 Value;

	FVoxelType_internal() = default;
	FVoxelType_internal(const FVoxelType& Type) : Value(Type.Value) {}
	FVoxelType_internal(EVoxelValueType ValueType, EVoxelMaterialType MaterialType)
		: Value((uint8)ValueType | ((uint8)MaterialType << 4))
	{
	}

	FORCEINLINE EVoxelValueType GetValueType() const
	{
		return (EVoxelValueType)(0x0F & Value);
	}

	FORCEINLINE EVoxelMaterialType GetMaterialType() const
	{
		return (EVoxelMaterialType)(Value >> 4);
	}

	operator FVoxelType() const { return FVoxelType(Value); }
};

union FVoxelNodeType
{
	float F;
	int32 I;
	bool B;
	FVoxelMaterial_internal M;
	EVoxelValueType VVT;
	EVoxelMaterialType VMT;
	FVoxelType_internal VT;
};

UENUM()
enum class EVoxelPinCategory : uint8
{
	Exec,
	Boolean,
	Int,
	Float,
	Material,
	ValueType,
	MaterialType,
	VoxelType
};

class VOXEL_API FVoxelPinCategory
{
public:
	static const FString PC_Exec;
	static const FString PC_Boolean;
	static const FString PC_Int;
	static const FString PC_Float;
	static const FString PC_Material;
	static const FString PC_ValueType;
	static const FString PC_MaterialType;
	static const FString PC_VoxelType;

	static const EVoxelPinCategory FromString(const FString& String);
	static const FString ToString(EVoxelPinCategory Category);
};

USTRUCT()
struct FVoxelPin
{
	GENERATED_BODY()

	UPROPERTY()
	FGuid PinId;

	UPROPERTY()
	EVoxelPinCategory PinCategory;

	UPROPERTY()
	FString DefaultValue;

	UPROPERTY()
	UVoxelNode* OtherNode;

	UPROPERTY()
	FGuid OtherPinId;

	FVoxelPin()
		: OtherNode(nullptr)
		, PinCategory(EVoxelPinCategory::Exec)
		, DefaultValue(TEXT(""))
	{
	}

	FVoxelPin(const FGuid& PinId, const FString& PinCategory, UVoxelNode* const& OtherNode = nullptr, const FGuid& OtherPinId = FGuid(), const FString& DefaultValue = FString())
		: PinId(PinId)
		, PinCategory(FVoxelPinCategory::FromString(PinCategory))
		, OtherNode(OtherNode)
		, OtherPinId(OtherPinId)
		, DefaultValue(DefaultValue)
	{
	}
};

/**
 * Base class for VoxelNodes
 */
UCLASS(abstract, hidecategories = Object, editinlinenew)
class VOXEL_API UVoxelNode : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<FVoxelPin> InputPins;
	
	UPROPERTY()
	TArray<FVoxelPin> OutputPins;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	UEdGraphNode* GraphNode;
#endif

	// Temporary variables used for compilation
	TArray<int32> InputIds;
	TArray<int32> OutputIds;

	/** Get the index of output pin PinId. Return -1 if there's no matching pin */
	int32 GetOutputPinIndex(const FGuid& PinId);

	/** Create initial input pins */
	void CreateStartingConnectors();

	/** Get the current number of input pins */
	int32 GetInputPinsCount() const { return InputPinCount; }
	/** Remove an input pin */
	void RemoveInputPin();
	/** Add an input pin */
	void AddInputPin();

	/** Get the number of input pins of this node, exec pin excluded */
	int32 GetInputPinsWithoutExecCount() const;
	/** Get the number of outnput pins of this node, exec pin excluded */
	int32 GetOutputPinsWithoutExecCount() const;

	/** Check if this node can input Category */
	bool HasInputPinWithCategory(const FString& Category) const;
	/** Check if this node can output Category */
	bool HasOutputPinWithCategory(const FString& Category) const;

	//~ Begin UVoxelNode Interface
	/** Get the max number of input pins this node can have. Must be < MAX_PINS */
	virtual int32 GetMaxInputPins() const { return 1; }
	/** Get the min number of input pins this node must have. Must be < MAX_PINS */
	virtual int32 GetMinInputPins() const { return 0; }
	/** Get the number of output pins this node have. Must be < MAX_PINS */
	virtual int32 GetOutputPinsCount() const { return 1; }

	/** The color of this node. Black by default */
	virtual FLinearColor GetColor() const { return FLinearColor::Black; }
	/** The title of this node. By default the DisplayName */
	virtual FText GetTitle() const;
	/** SHould this node be displayed in the compact form? Default to false */
	virtual bool IsCompact() const { return false; }

	/** Get the input pin PinIndex name */
	virtual FText GetInputPinName(int32 PinIndex) const { return FText::GetEmpty(); }
	/** Get the output pin PinIndex name */
	virtual FText GetOutputPinName(int32 PinIndex) const { return FText::GetEmpty(); }

	/** Get the input pin PinIndex category */
	virtual FString GetInputPinCategory(int32 PinIndex) const { return FVoxelPinCategory::PC_Float; }
	/** Get the output pin PinIndex category */
	virtual FString GetOutputPinCategory(int32 PinIndex) const { return FVoxelPinCategory::PC_Float; }

	/** Get the ComputeNode corresponding to this VoxelNode */
	virtual TSharedPtr<FVoxelComputeNode> GetComputeNode() const;
	//~ End UVoxelNode Interface

#if WITH_EDITOR
	//~ Begin UObject Interface
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	void PostLoad() override;
	//~ End UObject Interface
#endif //WITH_EDITOR

private:
	UPROPERTY()
	int32 InputPinCount;
};

struct FVoxelVariable
{
	// The variable type. Must not have any space/tab
	FString VariableType;
	// The variable name. Must not have any space/tab
	FString VariableName;

	FVoxelVariable() = default;
	FVoxelVariable(const FString& Type, const FString& Name) 
		: VariableType(Type)
		, VariableName(Name)
	{
		check(Type.Find(" ") == INDEX_NONE);
		check(Type.Find("	") == INDEX_NONE);

		check(Name.Find(" ") == INDEX_NONE);
		check(Name.Find("	") == INDEX_NONE);
	}
};

struct FVoxelExposedVariable : public FVoxelVariable
{
	// Optional
	FString VariableDefaultValue;
	// Optional. For instance if variable name is WorldGenerator, "WorldGenerator.GetWorldGenerator()"
	FString VariableCustomAccessor;
	// Optional. For instance, VariableType = FVoxelWorldGeneratorPicker and VariableInstanceType = FVoxelWorldGeneratorInstance*
	FString VariableInstanceType;

	FString GetAccessor() const
	{
		return VariableCustomAccessor.IsEmpty() ? VariableName : VariableCustomAccessor;
	}

	FString GetInstanceType() const
	{
		return VariableInstanceType.IsEmpty() ? VariableType : VariableInstanceType;
	}
};

class FVoxelComputeNode
{
public:
	const int InputCount;
	const int OutputCount;
	const UVoxelNode* const Node;

	FVoxelComputeNode(const UVoxelNode* InNode);
	virtual ~FVoxelComputeNode() = default;

	virtual void Init(const AVoxelWorld* VoxelWorld) {};
	virtual void Compute(FVoxelNodeType Inputs[], FVoxelNodeType Outputs[], const FVoxelContext& Context) const { check(false); }
	virtual int32 GetBranchResult(FVoxelNodeType Inputs[]) const { check(false); return -1; }

	virtual bool IsSetValueNode() const { return false; }
	virtual bool IsSetMaterialNode() const { return false; }
	virtual bool IsSetVoxelTypeNode() const { return false; }

	FORCEINLINE FVoxelNodeType GetDefaultValue(int32 Index) { return DefaultValues[Index]; }
	FORCEINLINE FString GetDefaultValueString(int32 Index) { return DefaultValueStrings[Index]; }
	FORCEINLINE int32 GetInputId(int32 Index) { return InputIds[Index]; }
	FORCEINLINE int32 GetOutputId(int32 Index) { return OutputIds[Index]; }
	FORCEINLINE FString GetOutputType(int32 Index) { return OutputType[Index]; }

	// Add headers to OutAdditionalHeaders: for instance, "\"CoreMinimal.h\""
	virtual void GetAdditionalHeaders(TArray<FString>& OutAdditionalHeaders) const {}
	virtual void GetVariables(TArray<FVoxelVariable>& Variables) const {}
	virtual void GetExposedVariables(TArray<FVoxelExposedVariable>& Variables) const {}
	virtual void GetSetVoxelWorld(const FString& VoxelWorld, FString& OutCpp) const {}
	virtual void GetMain(const TArray<FString>& Inputs, const TArray<FString>& Outputs, const FString& Context, FString& OutCpp) const { check(false); }
	virtual FString GetBranchResultCpp(const TArray<FString>& Inputs) const { check(false); return FString(); }

private:
	FVoxelNodeType DefaultValues[MAX_PINS];
	FString DefaultValueStrings[MAX_PINS];
	FString OutputType[MAX_PINS];
	int32 InputIds[MAX_PINS];
	int32 OutputIds[MAX_PINS];
};
