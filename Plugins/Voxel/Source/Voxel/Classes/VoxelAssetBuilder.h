// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelAsset.h"
#include "VoxelAssetBuilder.generated.h"

/**
 * A Sphere Asset is defined with a 3 coordinates size
 */
UCLASS(MinimalAPI)
class UVoxelAssetBuilder : public UVoxelAsset
{
	GENERATED_BODY()

public:
	UVoxelAssetBuilder(const FObjectInitializer& ObjectInitializer);

protected:
	//~ Begin UVoxelAsset Interface
	TSharedRef<FVoxelAssetInstance> GetAssetInternal(const FIntVector& Position) const override;
	//~ End UVoxelAsset Interface

private:
	UPROPERTY(EditAnywhere)
	FVoxelWorldGeneratorPicker WorldGenerator;

	// In voxels
	UPROPERTY(EditAnywhere)
	FIntBox Bounds;

	// In voxels
	UPROPERTY(EditAnywhere)
	FIntVector Offset;
};
	
class FVoxelAssetBuilderInstance : public FVoxelAssetInstance
{
public:
	FVoxelAssetBuilderInstance(TSharedRef<FVoxelWorldGeneratorInstance> WorldGenerator, const FIntVector& Position, const FIntBox& Bounds, const FIntVector& Offset);

	//~ Begin FVoxelAssetInstance Interface
	void GetValuesAndMaterialsAndVoxelTypes(float Values[], FVoxelMaterial Materials[], FVoxelType VoxelTypes[], const FIntVector& Start, const FIntVector& StartIndex, int Step, const FIntVector& Size, const FIntVector& ArraySize) const override;
	bool IsAssetEmpty(const FIntVector& Start, const int Step, const FIntVector& Size) const override;
	FIntBox GetLocalBounds() const override;
	void SetVoxelWorld(const AVoxelWorld* VoxelWorld);
	//~ End FVoxelAssetInstance Interface

private:
	const TSharedRef<FVoxelWorldGeneratorInstance> WorldGenerator;
	const FIntBox Bounds;
	const FIntVector Offset;
};
