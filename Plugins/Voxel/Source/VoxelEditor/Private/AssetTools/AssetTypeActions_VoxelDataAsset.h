// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"
#include "VoxelAssets/VoxelDataAsset.h"

class FAssetTypeActions_VoxelDataAsset : public FAssetTypeActions_Base
{
public:
	FAssetTypeActions_VoxelDataAsset(EAssetTypeCategories::Type InAssetCategory)
		: MyAssetCategory(InAssetCategory)
	{

	}

	// IAssetTypeActions Implementation
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_VoxelDataAsset", "Voxel Data Asset"); }
	virtual FColor GetTypeColor() const override { return FColor(128, 0, 64); }
	virtual UClass* GetSupportedClass() const override { return UVoxelDataAsset::StaticClass(); }
	virtual uint32 GetCategories() override { return MyAssetCategory; }
	
private:
	EAssetTypeCategories::Type MyAssetCategory;
};
