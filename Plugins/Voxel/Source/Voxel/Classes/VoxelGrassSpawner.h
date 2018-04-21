// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "LandscapeGrassType.h"
#include "VoxelGrassSpawner.generated.h"

USTRUCT()
struct FVoxelGrassVariety
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Grass)
	UStaticMesh* GrassMesh;

	/** Instances per 10 square meters. */
	UPROPERTY(EditAnywhere, Category = Grass)
	float GrassDensity;

	/** The distance where instances will begin to fade out if using a PerInstanceFadeAmount material node. 0 disables. */
	UPROPERTY(EditAnywhere, Category = Grass)
	int32 StartCullDistance;

	/**
	 * The distance where instances will have completely faded out when using a PerInstanceFadeAmount material node. 0 disables.
	 * When the entire cluster is beyond this distance, the cluster is completely culled and not rendered at all.
	 */
	UPROPERTY(EditAnywhere, Category = Grass)
	int32 EndCullDistance;

	/**
	 * Specifies the smallest LOD that will be used for this component.
	 * If -1 (default), the MinLOD of the static mesh asset will be used instead.
	 */
	UPROPERTY(EditAnywhere, Category = Grass)
	int32 MinLOD;

	/** Specifies grass instance scaling type */
	UPROPERTY(EditAnywhere, Category = Grass)
	EGrassScaling Scaling;

	/** Specifies the range of scale, from minimum to maximum, to apply to a grass instance's X Scale property */
	UPROPERTY(EditAnywhere, Category = Grass)
	FFloatInterval ScaleX;

	/** Specifies the range of scale, from minimum to maximum, to apply to a grass instance's Y Scale property */
	UPROPERTY(EditAnywhere, Category = Grass)
	FFloatInterval ScaleY;

	/** Specifies the range of scale, from minimum to maximum, to apply to a grass instance's Z Scale property */
	UPROPERTY(EditAnywhere, Category = Grass)
	FFloatInterval ScaleZ;

	/** Whether the grass instances should be placed at random rotation (true) or all at the same rotation (false) */
	UPROPERTY(EditAnywhere, Category = Grass)
	bool RandomRotation;

	/** Whether the grass instances should be tilted to the normal of the landscape (true), or always vertical (false) */
	UPROPERTY(EditAnywhere, Category = Grass)
	bool AlignToSurface;
	
	/** Min angle between grass up vector and world generator up vector in degrees */
	UPROPERTY(EditAnywhere, Category = Grass, meta = (UIMin = "0", UIMax = "180"))
	float MinAngleWithWorldUp;
	/** Max angle between grass up vector and world generator up vector in degrees */
	UPROPERTY(EditAnywhere, Category = Grass, meta = (UIMin = "0", UIMax = "180"))
	float MaxAngleWithWorldUp;

	/**
	 * Lighting channels that the grass will be assigned. Lights with matching channels will affect the grass.
	 * These channels only apply to opaque materials, direct lighting, and dynamic lighting and shadowing.
	 */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = Grass)
	FLightingChannels LightingChannels;

	/** Whether the grass instances should receive decals. */
	UPROPERTY(EditAnywhere, Category = Grass)
	bool bReceivesDecals;

	FVoxelGrassVariety()
		: GrassMesh(nullptr)
		, GrassDensity(20)
		, StartCullDistance(10000.0f)
		, EndCullDistance(10000.0f)
		, MinLOD(-1)
		, Scaling(EGrassScaling::Uniform)
		, ScaleX(1.0f, 1.0f)
		, ScaleY(1.0f, 1.0f)
		, ScaleZ(1.0f, 1.0f)
		, RandomRotation(true)
		, AlignToSurface(true)
		, MinAngleWithWorldUp(0)
		, MaxAngleWithWorldUp(90)
		, bReceivesDecals(true)
	{
	}
};

UCLASS(MinimalAPI)
class UVoxelGrassGroup : public UObject
{
	GENERATED_BODY()

public:
	UVoxelGrassGroup(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{

	}

	UPROPERTY(EditAnywhere, Category = Grass)
	TArray<FVoxelGrassVariety> GrassVarieties;
};

USTRUCT()
struct FVoxelGrassGroupID
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	UVoxelGrassGroup* GrassType;

	UPROPERTY(EditAnywhere)
	uint8 Material;
};


UCLASS(MinimalAPI)
class UVoxelGrassSpawner : public UObject
{
	GENERATED_BODY()

public:
	UVoxelGrassSpawner(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{

	}

	UPROPERTY(EditAnywhere)
	TArray<FVoxelGrassGroupID> GrassTypes;
};

//////////////////////////
// Thread safe versions //
//////////////////////////

struct FVoxelGrassGroup_ThreadSafe
{
	TArray<FVoxelGrassVariety> GrassVarieties;
};

struct FVoxelGrassGroupID_ThreadSafe
{
	FVoxelGrassGroup_ThreadSafe GrassType;

	uint8 Material;
};

struct FVoxelGrassSpawner_ThreadSafe
{
	FVoxelGrassSpawner_ThreadSafe() = default;
	FVoxelGrassSpawner_ThreadSafe(UVoxelGrassSpawner* Spawner)
	{
		for (const auto& GrassID : Spawner->GrassTypes)
		{
			FVoxelGrassGroupID_ThreadSafe NewID;
			NewID.Material = GrassID.Material;
			for (const auto& GrassVariety : GrassID.GrassType->GrassVarieties)
			{
				if (GrassVariety.GrassMesh)
				{
					NewID.GrassType.GrassVarieties.Add(GrassVariety);
				}
			}
			GrassTypes.Add(NewID);
		}
	}

	TArray<FVoxelGrassGroupID_ThreadSafe> GrassTypes;
};

