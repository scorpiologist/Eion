// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "VoxelWorldGenerator.h"
#include "VoxelNode.h"
#include "VoxelGraphWorldGenerator.generated.h"

class UVoxelGraphGenerator;

#if WITH_EDITOR
/** 
 * Interface for voxel graph interaction with the VoxelEditor module.
 */
class IVoxelGraphEditor
{
public:
	virtual ~IVoxelGraphEditor() = default;

	/** Called when creating a new voxel graph. */
	virtual UEdGraph* CreateNewVoxelGraph(UVoxelGraphGenerator* InGenerator) = 0;

	/** Sets up a voxel node. */
	virtual void SetupVoxelNode(UEdGraph* VoxelGraph, UVoxelNode* VoxelNode, bool bSelectNewNode) = 0;

	/** Compiles voxel nodes from graph nodes. */
	virtual void CompileVoxelNodesFromGraphNodes(UVoxelGraphGenerator* WorldGenerator) = 0;

	/** Creates an input pin on the given voxel graph node. */
	virtual void CreateInputPin(UEdGraphNode* VoxelNode) = 0;
	/** Creates an output pin on the given voxel graph node. */
	virtual void CreateOutputPin(UEdGraphNode* VoxelNode) = 0;
};
#endif

///////////////////////////////////////////////////////////////////////////////

/**
 * Tree of the compute nodes. Used to interpret the graph at runtime and to compile it
 */
struct FVoxelComputeNodeTree
{
	FVoxelComputeNodeTree();
	~FVoxelComputeNodeTree();

	/**
	 * Add a child to the root of this tree and return it
	 */
	FVoxelComputeNodeTree* AddChild();

	/**
	 * Set the compute nodes of the root of this tree
	 */
	void SetNodes(const TArray<TSharedPtr<FVoxelComputeNode>>& InNodes);

	// Runtime
	void Init(const AVoxelWorld* VoxelWorld) const;
	void Compute(FVoxelNodeType Variables[], const FVoxelContext& Context, float& Value, FVoxelMaterial& Material, FVoxelType& VoxelType) const;

	// Compilation
	void GetAdditionalHeaders(TArray<FString>& OutAdditionalHeaders) const;
	void GetExposedVariables(TArray<FVoxelExposedVariable>& Variables) const;
	void GetVariables(TArray<FVoxelVariable>& Variables) const;
	void GetSetVoxelWorld(const FString& VoxelWorld, FString& OutCpp) const;
	void GetMain(const TArray<FString>& Variables, const FString& Context, const FString& Value, const FString& Material, const FString& VoxelType, FString& OutCpp);

private:
	TSharedPtr<FVoxelComputeNode>* Nodes;
	int32 NodesCount;
	FVoxelComputeNodeTree* Childs[MAX_PINS];
	int32 ChildsCount;
};

//////////////////////////////////////////////////////////////////////////////////////

/**
 * Tree used to create the above compute tree
 */
struct FVoxelNodeTree
{
	FVoxelNodeTree() = default;
	~FVoxelNodeTree();

	/**
	 * Add a child tree and return it
	 */
	FVoxelNodeTree* AddChild();

	/**
	 * Init this tree with the node AllNodes[NodeIndex], which must be a branch node
	 */
	void Init(const TArray<UVoxelNode*>& AllNodes, const TArray<TArray<int32>>& Dependencies, int32 NodeIndex);
	
	/**
	 * Remove the nodes of this tree that are in parents trees 
	 */
	void CleanupDependencies(TArray<FVoxelNodeTree*> Parents = TArray<FVoxelNodeTree*>());
	
	/**
	 * Set the temp output ids of the VoxelNodes
	 */
	void SetOutputIds(int32 StartId, int32& OutEndId, const TArray<UVoxelNode*>& AllNodes);
	/**
	 * Set the temp input ids of the VoxelNodes
	 */
	void SetInputIds(const TArray<UVoxelNode*>& AllNodes);

	/**
	 * Create the ComputeNodeTree
	 */
	void ConvertToComputeNodeTree(FVoxelComputeNodeTree& OutTree, const TArray<UVoxelNode*>& AllNodes);

private:
	TArray<int32> Nodes;
	TArray<FVoxelNodeTree*> Childs;
};

///////////////////////////////////////////////////////////////////////////////

/**
 * The graph world generator
 */
UCLASS(BlueprintType, hidecategories = (Object))
class VOXEL_API UVoxelGraphGenerator : public UVoxelWorldGenerator
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<UVoxelNode*> AllNodes;

	UPROPERTY()
	UVoxelNode* FirstNode;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	class UEdGraph* VoxelGraph;
#endif

	/**
	 * Create a new node of NewNodeClass
	 */
	UVoxelNode* ConstructNewNode(UClass* NewNodeClass, bool bSelectNewNode = true);

	/**
	 * Create the cpp file of this graph
	 */
	bool CompileToCpp(FString& OutCppText, const FString& Filename, FString& OutErrorMessage);
	/**
	 * Create the ComputeNodeTree of this graph
	 */
	bool CreateComputeTree(FVoxelComputeNodeTree& OutTree, int32& OutMaxId);

	//~ Begin UVoxelWorldGenerator Interface
	TSharedRef<FVoxelWorldGeneratorInstance> GetWorldGenerator() override;
	//~ End UVoxelWorldGenerator Interface

#if WITH_EDITOR
	//~ Begin UObject Interface 
	void PostInitProperties() override;
	//~ End UObject Interface
	
	/** Set up EdGraph parts of a VoxelNode */
	void SetupVoxelNode(UVoxelNode* InVoxelNode, bool bSelectNewNode = true);
	/** Create the basic voxel graph */
	void CreateGraph();
	/** Get the EdGraph of VoxelNodes */
	class UEdGraph* GetGraph();
	/** Use the EdGraph representation to compile the VoxelNodes */
	void CompileVoxelNodesFromGraphNodes();


	/** Sets the voxel graph editor implementation.* */
	static void SetVoxelGraphEditor(TSharedPtr<IVoxelGraphEditor> InVoxelGraphEditor);
	/** Gets the voxel graph editor implementation. */
	static TSharedPtr<IVoxelGraphEditor> GetVoxelGraphEditor();

private:
	/** Ptr to interface to voxel editor operations. */
	static TSharedPtr<IVoxelGraphEditor> VoxelGraphEditor;
#endif
};

class VOXEL_API FVoxelGraphWorldGeneratorInstance : public FVoxelWorldGeneratorInstance
{
public:
	FVoxelGraphWorldGeneratorInstance(int32 MaxId, TSharedRef<FVoxelComputeNodeTree> ComputeTree);

	//~ Begin FVoxelWorldGeneratorInstance Interface
	void GetValuesAndMaterialsAndVoxelTypes(float Values[], FVoxelMaterial Materials[], FVoxelType VoxelTypes[], const FIntVector& Start, const FIntVector& StartIndex, int Step, const FIntVector& Size, const FIntVector& ArraySize) const override;
	void SetVoxelWorld(const AVoxelWorld* VoxelWorld) override;
	bool IsEmpty(const FIntVector& Start, const int Step, const FIntVector& Size) const override;
	//~ End FVoxelWorldGeneratorInstance Interface

private:
	const int32 MaxId; // Exclusive
	const TSharedRef<FVoxelComputeNodeTree> ComputeTree;
};
