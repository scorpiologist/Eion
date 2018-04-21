// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"
#include "VoxelActorSpawner.h"

class FAssetTypeActions_VoxelActorGroup : public FAssetTypeActions_Base
{
public:
	FAssetTypeActions_VoxelActorGroup(EAssetTypeCategories::Type InAssetCategory)
		: MyAssetCategory(InAssetCategory)
	{

	}

	// IAssetTypeActions Implementation
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_VoxelActorGroup", "Voxel Actor Group"); }
	virtual FColor GetTypeColor() const override { return FColor(255, 25, 128); }
	virtual UClass* GetSupportedClass() const override { return UVoxelActorGroup::StaticClass(); }
	virtual uint32 GetCategories() override { return MyAssetCategory; }

private:
	EAssetTypeCategories::Type MyAssetCategory;
};
