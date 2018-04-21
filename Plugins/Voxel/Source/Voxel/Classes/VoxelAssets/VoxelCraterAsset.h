// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelAsset.h"
#include "FastNoise.h"
#include "VoxelCraterAsset.generated.h"

/**
 * A Crater Asset 
 */
UCLASS(MinimalAPI)
class UVoxelCraterAsset : public UVoxelAsset
{
	GENERATED_BODY()

public:
	UVoxelCraterAsset(const FObjectInitializer& ObjectInitializer);

	// Radius of the crater in voxel space
	UPROPERTY(EditAnywhere)
	float Radius;

	// Material to set inside the crater
	UPROPERTY(EditAnywhere)
	FVoxelMaterial Material;

protected:
	//~ Begin UVoxelAsset Interface
	TSharedRef<FVoxelAssetInstance> GetAssetInternal(const FIntVector& Position) const override;
	//~ End UVoxelAsset Interface
};
	
class FVoxelCraterAssetInstance : public FVoxelAssetInstance
{
public:
	FVoxelCraterAssetInstance(float Radius, const FVoxelMaterial& Material, const FIntVector& Position);

	//~ Begin FVoxelAssetInstance Interface
	void GetValuesAndMaterialsAndVoxelTypes(float Values[], FVoxelMaterial Materials[], FVoxelType VoxelTypes[], const FIntVector& Start, const FIntVector& StartIndex, int Step, const FIntVector& Size, const FIntVector& ArraySize) const override;
	FIntBox GetLocalBounds() const override;
	//~ End FVoxelAssetInstance Interface

private:
	const float Radius;
	const FVoxelMaterial Material;

	FastNoise Noise;
};
