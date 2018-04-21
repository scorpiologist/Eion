// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelThread.h"
#include "VoxelProceduralMeshComponent.h"

class AVoxelWorld;
class FVoxelPolygonizer;
class UVoxelChunkComponent;
class FVoxelData;
class FVoxelWorldGeneratorInstance;
class AVoxelActor;

/**
 * Thread to create mesh
 */
class FAsyncCubicPolygonizerWork : public FVoxelAsyncWork
{
public:
	FVoxelData* const Data;
	const FIntVector ChunkPosition;
	const FIntVector PositionOffset;

	const AVoxelWorld* const World;
	
	const bool bComputeGrass;
	const bool bComputeVoxelActors;

	// Mesh Output
	FVoxelProcMeshSection Section;
	// Grass Output
	TArray<TSharedPtr<FVoxelGrassBuffer>> GrassBuffers;
	TArray<TSet<FIntVector>> NewGrassPositionsArray;
	// Actors Output
	TArray<FVoxelActorSpawnInfo> ActorsSpawnInfo;

	FAsyncCubicPolygonizerWork(
		FVoxelData* Data,
		const FIntVector& ChunkPosition,
		const FIntVector& PositionOffset,
		AVoxelWorld* World
		,bool bComputeGrass = true,
		const TArray<TSet<FIntVector>>& OldPositionsArray = TArray<TSet<FIntVector>>(),
		bool bComputeVoxelActors = false
		);
	
	virtual void DoWork() override;
	virtual int GetPriority() const override;

private:
	FThreadSafeCounter IsDoneCounter;
	FEvent* DoneEvent;
	FCriticalSection DoneSection;

	TArray<TSet<FIntVector>> OldGrassPositionsArray;
};