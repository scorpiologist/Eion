// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelAsset.h"
#include "VoxelDataAsset.generated.h"

/**
 * A Data Asset stores the values of every voxel inside it
 */
UCLASS(MinimalAPI)
class UVoxelDataAsset : public UVoxelAsset
{
	GENERATED_BODY()

public:
	UVoxelDataAsset(const FObjectInitializer& ObjectInitializer);
	
	/**
	 * Set the size of this asset. This reset all the arrays.
	 * @param	NewSize			The new size to set
	 * @param	bInitialize		Should the arrays be initialized?
	 */
	VOXEL_API void SetSize(const FIntVector& NewSize, bool bInitialize);
	/**
	 * Get the size of this asset 
	 */
	VOXEL_API FIntVector GetSize() const;


	/**
	 * Set the value at (X, Y, Z)
	 * @see	Save
	 */
	VOXEL_API FORCEINLINE void SetValue(int X, int Y, int Z, float NewValue);
	/**
	 * Set the material at (X, Y, Z)
	 * @see	Save
	 */
	VOXEL_API FORCEINLINE void SetMaterial(int X, int Y, int Z, FVoxelMaterial NewMaterial);
	/**
	 * Set the voxel type at (X, Y, Z)
	 * @see	Save
	 */
	VOXEL_API FORCEINLINE void SetVoxelType(int X, int Y, int Z, FVoxelType VoxelType);


	/**
	 * Get the value at (X, Y, Z)
	 */
	VOXEL_API FORCEINLINE float GetValue(int X, int Y, int Z) const;
	/**
	 * Get the material at (X, Y, Z)
	 */
	VOXEL_API FORCEINLINE FVoxelMaterial GetMaterial(int X, int Y, int Z) const;
	/**
	 * Get the voxel type at (X, Y, Z)
	 */
	VOXEL_API FORCEINLINE FVoxelType GetVoxelType(int X, int Y, int Z) const;

	/**
	 * Directly set the arrays and the size
	 */
	VOXEL_API void SetPrecomputedArrays(FIntVector Size, TArray<float>& Values, TArray<FVoxelMaterial>& Materials, TArray<uint8>& VoxelTypes);

	/**
	 * Save this asset. This MUST be called after SetValue/SetMaterial/SetVoxelType to save the modifications
	 */
	VOXEL_API void Save();

protected:
	//~ Begin UVoxelAsset Interface
	TSharedRef<FVoxelAssetInstance> GetAssetInternal(const FIntVector& Position) const override;
	VOXEL_API void LoadInternal() override;
	//~ End UVoxelAsset Interface

private:
	UPROPERTY()
	TArray<uint8> CompressedData;

	FIntVector Size;

	TArray<float> Values;
	TArray<FVoxelMaterial> Materials;
	TArray<uint8> VoxelTypes;

	static const ECompressionFlags CompressionFlags = (ECompressionFlags)(COMPRESS_ZLIB | COMPRESS_BiasSpeed);
};

class FVoxelDataAssetInstance : public FVoxelAssetInstance
{
public:
	FVoxelDataAssetInstance(const FIntVector& Size, const TArray<float>& Values, const TArray<FVoxelMaterial>& Materials, const TArray<uint8>& VoxelTypes, const FIntVector& Position);

	//~ Begin FVoxelAssetInstance Interface
	void GetValuesAndMaterialsAndVoxelTypes(float Values[], FVoxelMaterial Materials[], FVoxelType VoxelTypes[], const FIntVector& Start, const FIntVector& StartIndex, int Step, const FIntVector& Size, const FIntVector& ArraySize) const override;
	FIntBox GetLocalBounds() const override;
	//~ End FVoxelAssetInstance Interface

private:
	const FIntVector Size;
	const TArray<float> Values;
	const TArray<FVoxelMaterial> Materials;
	const TArray<uint8> VoxelTypes;
};
