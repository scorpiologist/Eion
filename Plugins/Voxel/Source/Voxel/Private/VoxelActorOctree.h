// Copyright 2018 Phyronnaz

#pragma once


#include "CoreMinimal.h"
#include "Octree.h"

class AVoxelActor;

struct FVoxelActorWithPosition
{
	AVoxelActor* Actor;
	FIntVector Position;
};

class FVoxelActorOctree : public TVoxelOctree<FVoxelActorOctree, 32>
{
public:
	FVoxelActorOctree(uint8 LOD, float MaxRenderDistance);
	FVoxelActorOctree(FVoxelActorOctree* Parent, uint8 ChildIndex);

	void AddActor(AVoxelActor* Actor, const FIntVector& Position);
	bool RemoveActor(AVoxelActor* Actor);

	void UpdateVisibility(const TArray<FIntVector>& CameraVoxelPositions);
	void GetActorsInBox(const FIntBox& Box, TArray<AVoxelActor*>& OutActors);

private:
	TArray<FVoxelActorWithPosition> Actors;
	
	bool bOctreeIsDisabled;
	const int MaxRenderDistanceSquared;	
	
	void SetIsEnabled(bool bIsEnabled);
};
