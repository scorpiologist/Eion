// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IntBox.h"
#include "VoxelWorldGenerator.h"
#include "VoxelAsset.generated.h"

/**
 * A Voxel Asset is a World Generator with bounds and a position
 */
UCLASS()
class VOXEL_API UVoxelAsset : public UVoxelWorldGenerator
{
	GENERATED_BODY()

public:
	UVoxelAsset(const FObjectInitializer& ObjectInitializer);

	//~ Begin UVoxelWorldGenerator Interface
	TSharedRef<FVoxelWorldGeneratorInstance> GetWorldGenerator() final;
	//~ End UVoxelWorldGenerator Interface

	/**
	 * Get the FVoxelAssetInstance corresponding to this asset. Load if needed
	 */
	TSharedRef<class FVoxelAssetInstance> GetAsset(const FIntVector& Position);
	
	/**
	 * Load the asset (for instance decompress it if needed)
	 */
	void Load();

protected:
	//~ Begin UVoxelAsset Interface
	/**
	 * Get the FVoxelAssetInstance corresponding to this asset
	 */
	virtual TSharedRef<class FVoxelAssetInstance> GetAssetInternal(const FIntVector& Position) const;
	/**
	 * If this asset has compressed data, decompress it in this function
	 */
	virtual void LoadInternal();
	//~ End UVoxelAsset Interface

private:
	bool bIsLoaded;
};

class VOXEL_API FVoxelAssetInstance : public FVoxelWorldGeneratorInstance
{
public:
	const FIntVector Position;

	FVoxelAssetInstance(const FIntVector& Position) : Position(Position) {}
	
	//~Begin FVoxelWorldGeneratorInstance Interface
	bool IsEmpty(const FIntVector& Start, const int Step, const FIntVector& Size) const final;
	//~End FVoxelWorldGeneratorInstance Interface

	/** Get the bounds translated by Position */
	FIntBox GetWorldBounds() const;

	//~ Begin FVoxelAssetInstance Interface
	/** Local bounds (without Position) */
	virtual FIntBox GetLocalBounds() const;;
	/** Is the asset empty? No need to check if it intersects */
	virtual bool IsAssetEmpty(const FIntVector& Start, const int Step, const FIntVector& Size) const;
	//~ End FVoxelAssetInstance Interface
};
