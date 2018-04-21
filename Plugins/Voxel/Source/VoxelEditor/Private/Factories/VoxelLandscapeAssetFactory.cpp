// Copyright 2018 Phyronnaz

#include "VoxelLandscapeAssetFactory.h"
#include "AssetTypeCategories.h"
#include "VoxelAssets/VoxelLandscapeAsset.h"

UVoxelLandscapeAssetFactory::UVoxelLandscapeAssetFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = false;
	bEditAfterNew = true;
	SupportedClass = UVoxelLandscapeAsset::StaticClass();
}

UObject* UVoxelLandscapeAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	auto NewLandscapeAsset = NewObject<UVoxelLandscapeAsset>(InParent, Class, Name, Flags);

	return NewLandscapeAsset;
}
