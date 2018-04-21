// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.h"
#include "IntBox.h"
#include "VoxelAsset.h"
#include "VoxelLandscapeAsset.generated.h"

/**
 * Asset that holds 2D information
 */
UCLASS(MinimalAPI)
class UVoxelLandscapeAsset : public UVoxelAsset 
// TODO: rename to HeightmapAsset and create float, uin16 and uint8 versions
{
	GENERATED_BODY()

public:
	UVoxelLandscapeAsset(const FObjectInitializer& ObjectInitializer);

	/**
	 * Set the size of this asset. This reset all the arrays.
	 * @param	Width			The new Width to set
	 * @param	Height			The new Height to set
	 * @param	bInitialize		Should the arrays be initialized?
	 */
	VOXEL_API void SetSize(int Width, int Height, bool bInitialize);

	// Getters
	FORCEINLINE int GetTerrainWidth();
	FORCEINLINE int GetTerrainHeight();

	// Setters
	VOXEL_API FORCEINLINE void SetHeight(int X, int Y, float Height);
	VOXEL_API FORCEINLINE void SetMaterial(int X, int Y, FVoxelMaterial Material);

	// Getters
	VOXEL_API FORCEINLINE float GetHeight(int X, int Y) const;
	VOXEL_API FORCEINLINE FVoxelMaterial GetMaterial(int X, int Y) const;

	/**
	 * Directly set the arrays
	 */
	VOXEL_API void SetPrecomputedValues(const TArray<float>& Heights, const TArray<FVoxelMaterial>& Materials, int Width, int Height, float MaxHeight, float MinHeight);

	/**
	 * Save this asset. This MUST be called after SetValue/SetMaterial to save the modifications
	 */
	VOXEL_API void Save();

public:
	// Higher precision can improve render quality, but voxel values are lower (hardness not constant)
	UPROPERTY(EditAnywhere, meta = (ClampMin = "1", UIMin = "1"))
	int Precision;

	UPROPERTY(EditAnywhere)
	float HeightMultiplier;

	// In world size
	UPROPERTY(EditAnywhere)
	float HeightOffset;

	// In world size. Fill under the world
	UPROPERTY(EditAnywhere)
	float AdditionalThickness;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "1", UIMin = "1"))
	int ScaleMultiplier;

	UPROPERTY(EditAnywhere)
	bool bShrink;

protected:
	//~ Begin UVoxelAsset Interface
	TSharedRef<FVoxelAssetInstance> GetAssetInternal(const FIntVector& Position) const override;
	VOXEL_API void LoadInternal() override;
	//~ End UVoxelAsset Interface

private:
	UPROPERTY()
	TArray<uint8> CompressedData;

	TArray<float> Heights;
	TArray<FVoxelMaterial> Materials;
	int Width;
	int Height;
	float MaxHeight;
	float MinHeight;

	static const ECompressionFlags CompressionFlags = (ECompressionFlags)(COMPRESS_ZLIB | COMPRESS_BiasSpeed);
};


class VOXEL_API FVoxelLandscapeAssetInstance : public FVoxelAssetInstance
{
public:
	FVoxelLandscapeAssetInstance(
		const TArray<float>& Heights,
		const TArray<FVoxelMaterial>& Materials,
		int Width,
		int Height,
		float MaxHeight,
		float MinHeight,
		int Precision,
		float HeightMultiplier,
		float HeightOffset,
		float AdditionalThickness,
		int ScaleMultiplier,
		bool bShrink,
		const FIntVector& Position);

	//~ Begin FVoxelAssetInstance Interface
	void SetVoxelWorld(const AVoxelWorld* VoxelWorld) override;
	void GetValuesAndMaterialsAndVoxelTypes(float Values[], FVoxelMaterial Materials[], FVoxelType VoxelTypes[], const FIntVector& Start, const FIntVector& StartIndex, int Step, const FIntVector& Size, const FIntVector& ArraySize) const override;
	bool IsAssetEmpty(const FIntVector& Start, const int Step, const FIntVector& Size) const override;
	FIntBox GetLocalBounds() const override;
	//~ End FVoxelAssetInstance Interface

	// Getters
	FORCEINLINE float GetHeight(int X, int Y) const;
	FORCEINLINE FVoxelMaterial GetMaterial(int X, int Y) const;

private:
	const TArray<float> Heights;
	const TArray<FVoxelMaterial> Materials;
	const int Width;
	const int Height;
	const float MaxHeight;
	const float MinHeight;

	const int Precision;
	const float HeightMultiplier;
	const float HeightOffset;
	const float AdditionalThickness;
	const int ScaleMultiplier;
	const bool bShrink;

private:
	float VoxelSize;
};
