// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "SubclassOf.h"
#include "LandscapeGrassType.h"
#include "VoxelActorSpawner.generated.h"

class AVoxelActor;

USTRUCT()
struct FVoxelActorConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FString Name;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AVoxelActor> Actor;

	/** Instances per 1000 square meters. */
	UPROPERTY(EditAnywhere)
	float Density;
	
	/** Specifies instance scaling type */
	UPROPERTY(EditAnywhere)
	EGrassScaling Scaling;

	/** Specifies the range of scale, from minimum to maximum, to apply to an actor instance's X Scale property */
	UPROPERTY(EditAnywhere)
	FFloatInterval ScaleX;

	/** Specifies the range of scale, from minimum to maximum, to apply to an actor instance's Y Scale property */
	UPROPERTY(EditAnywhere)
	FFloatInterval ScaleY;

	/** Specifies the range of scale, from minimum to maximum, to apply to an actor instance's Z Scale property */
	UPROPERTY(EditAnywhere)
	FFloatInterval ScaleZ;

	/** Whether the actor instances should be placed at random rotation (true) or all at the same rotation (false) */
	UPROPERTY(EditAnywhere)
	bool RandomRotation;

	/** Whether the actor instances should be tilted to the normal of the landscape (true), or always vertical (false) */
	UPROPERTY(EditAnywhere)
	bool AlignToSurface;
	
	/** Min angle between object up vector and world generator up vector in degrees */
	UPROPERTY(EditAnywhere, meta = (UIMin = "0", UIMax = "180"))
	float MinAngleWithWorldUp;
	/** Max angle between object up vector and world generator up vector in degrees */
	UPROPERTY(EditAnywhere, meta = (UIMin = "0", UIMax = "180"))
	float MaxAngleWithWorldUp;

	
	FVoxelActorConfig()
		: Density(1)
		, Scaling(EGrassScaling::Uniform)
		, ScaleX(1.0f, 1.0f)
		, ScaleY(1.0f, 1.0f)
		, ScaleZ(1.0f, 1.0f)
		, RandomRotation(true)
		, AlignToSurface(true)
		, MinAngleWithWorldUp(0)
		, MaxAngleWithWorldUp(90)
	{
	}
};

UCLASS(MinimalAPI)
class UVoxelActorGroup : public UObject
{
	GENERATED_BODY()

public:
	UVoxelActorGroup(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{

	}

	UPROPERTY(EditAnywhere)
	TArray<FVoxelActorConfig> ActorConfigs;
};

USTRUCT()
struct FVoxelActorGroupID
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	UVoxelActorGroup* Group;

	UPROPERTY(EditAnywhere)
	uint8 ID;
};


UCLASS(MinimalAPI)
class UVoxelActorSpawner : public UObject
{
	GENERATED_BODY()

public:
	UVoxelActorSpawner(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{

	}

	UPROPERTY(EditAnywhere)
	TArray<FVoxelActorGroupID> ActorConfigs;
};

//////////////////////////
// Thread safe versions //
//////////////////////////

struct FVoxelActorGroup_ThreadSafe
{
	TArray<FVoxelActorConfig> ActorConfigs;
};

struct FVoxelActorGroupID_ThreadSafe
{
	FVoxelActorGroup_ThreadSafe Group;

	uint8 ID;
};

struct FVoxelActorSpawner_ThreadSafe
{
	FVoxelActorSpawner_ThreadSafe() = default;
	FVoxelActorSpawner_ThreadSafe(UVoxelActorSpawner* Spawner)
	{
		for (const auto& ActorID : Spawner->ActorConfigs)
		{
			FVoxelActorGroupID_ThreadSafe NewID;
			NewID.ID = ActorID.ID;
			for (const auto& Actor : ActorID.Group->ActorConfigs)
			{
				NewID.Group.ActorConfigs.Add(Actor);
			}
			ActorConfigs.Add(NewID);
		}
	}

	TArray<FVoxelActorGroupID_ThreadSafe> ActorConfigs;
};

