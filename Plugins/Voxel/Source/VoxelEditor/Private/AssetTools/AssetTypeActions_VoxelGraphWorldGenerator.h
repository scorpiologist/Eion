// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/IToolkitHost.h"
#include "AssetTypeActions_Base.h"

class FMenuBuilder;
class UVoxelGraphGenerator;

class FAssetTypeActions_VoxelGraphWorldGenerator : public FAssetTypeActions_Base
{
public:
	FAssetTypeActions_VoxelGraphWorldGenerator(EAssetTypeCategories::Type InAssetCategory)
		: MyAssetCategory(InAssetCategory)
	{

	}

	// IAssetTypeActions Implementation
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_VoxelGraphWorldGenerator", "Voxel Graph"); }
	virtual FColor GetTypeColor() const override { return FColor(0, 175, 255); }
	virtual UClass* GetSupportedClass() const override;
	virtual void OpenAssetEditor( const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>() ) override;
	virtual bool CanFilter() override { return true; }
	virtual uint32 GetCategories() override;
	
private:
	EAssetTypeCategories::Type MyAssetCategory;
};
