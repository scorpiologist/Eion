// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"
#include "VoxelAssetBuilder.h"

class FAssetTypeActions_VoxelAssetBuilder : public FAssetTypeActions_Base
{
public:
	FAssetTypeActions_VoxelAssetBuilder(EAssetTypeCategories::Type InAssetCategory)
		: MyAssetCategory(InAssetCategory)
	{

	}

	// IAssetTypeActions Implementation
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "FAssetTypeActions_VoxelAssetBuilder", "Voxel Asset Builder"); }
	virtual FColor GetTypeColor() const override { return FColor::Orange; }
	virtual UClass* GetSupportedClass() const override { return UVoxelAssetBuilder::StaticClass(); }
	virtual uint32 GetCategories() override { return MyAssetCategory; }

private:
	EAssetTypeCategories::Type MyAssetCategory;
};
