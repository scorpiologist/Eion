// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"
#include "VoxelAssets/VoxelCraterAsset.h"

class FAssetTypeActions_VoxelCraterAsset : public FAssetTypeActions_Base
{
public:
	FAssetTypeActions_VoxelCraterAsset(EAssetTypeCategories::Type InAssetCategory)
		: MyAssetCategory(InAssetCategory)
	{

	}

	// IAssetTypeActions Implementation
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_VoxelCraterAsset", "Voxel Crater Asset"); }
	virtual FColor GetTypeColor() const override { return FColor::Orange; }
	virtual UClass* GetSupportedClass() const override { return UVoxelCraterAsset::StaticClass(); }
	virtual uint32 GetCategories() override { return MyAssetCategory; }

private:
	EAssetTypeCategories::Type MyAssetCategory;
};
