// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"
#include "VoxelGrassSpawner.h"

class FAssetTypeActions_VoxelGrassSpawner : public FAssetTypeActions_Base
{
public:
	FAssetTypeActions_VoxelGrassSpawner(EAssetTypeCategories::Type InAssetCategory)
		: MyAssetCategory(InAssetCategory)
	{

	}

	// IAssetTypeActions Implementation
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_VoxelGrassSpawner", "Voxel Grass Spawner"); }
	virtual FColor GetTypeColor() const override { return FColor(128, 255, 128); }
	virtual UClass* GetSupportedClass() const override { return UVoxelGrassSpawner::StaticClass(); }
	virtual uint32 GetCategories() override { return MyAssetCategory; }

private:
	EAssetTypeCategories::Type MyAssetCategory;
};
