// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"
#include "VoxelActorSpawner.h"

class FAssetTypeActions_VoxelActorSpawner : public FAssetTypeActions_Base
{
public:
	FAssetTypeActions_VoxelActorSpawner(EAssetTypeCategories::Type InAssetCategory)
		: MyAssetCategory(InAssetCategory)
	{

	}

	// IAssetTypeActions Implementation
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_VoxelActorSpawner", "Voxel Actor Spawner"); }
	virtual FColor GetTypeColor() const override { return FColor(255, 0, 128); }
	virtual UClass* GetSupportedClass() const override { return UVoxelActorSpawner::StaticClass(); }
	virtual uint32 GetCategories() override { return MyAssetCategory; }

private:
	EAssetTypeCategories::Type MyAssetCategory;
};
