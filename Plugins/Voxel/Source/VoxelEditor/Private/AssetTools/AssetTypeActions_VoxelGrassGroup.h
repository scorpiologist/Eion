// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"
#include "VoxelGrassSpawner.h"

class FAssetTypeActions_VoxelGrassGroup : public FAssetTypeActions_Base
{
public:
	FAssetTypeActions_VoxelGrassGroup(EAssetTypeCategories::Type InAssetCategory)
		: MyAssetCategory(InAssetCategory)
	{

	}

	// IAssetTypeActions Implementation
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_VoxelGrassGroup", "Voxel Grass Group"); }
	virtual FColor GetTypeColor() const override { return FColor(128, 255, 128); }
	virtual UClass* GetSupportedClass() const override { return UVoxelGrassGroup::StaticClass(); }
	virtual uint32 GetCategories() override { return MyAssetCategory; }

private:
	EAssetTypeCategories::Type MyAssetCategory;
};
