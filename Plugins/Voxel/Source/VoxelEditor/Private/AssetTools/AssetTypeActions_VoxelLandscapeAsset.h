// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"
#include "VoxelAssets/VoxelLandscapeAsset.h"

class FAssetTypeActions_VoxelLandscapeAsset : public FAssetTypeActions_Base
{
public:
	FAssetTypeActions_VoxelLandscapeAsset(EAssetTypeCategories::Type InAssetCategory)
		: MyAssetCategory(InAssetCategory)
	{

	}

	// IAssetTypeActions Implementation
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "FAssetTypeActions_VoxelLandscapeAsset", "Voxel Landscape Asset"); }
	virtual FColor GetTypeColor() const override { return FColor(200, 80, 80); }
	virtual UClass* GetSupportedClass() const override { return UVoxelLandscapeAsset::StaticClass(); }
	virtual uint32 GetCategories() override { return MyAssetCategory; }

private:
	EAssetTypeCategories::Type MyAssetCategory;
};
