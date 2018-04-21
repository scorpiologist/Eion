// Copyright 2018 Phyronnaz

#include "VoxelDataAssetFactory.h"
#include "AssetTypeCategories.h"
#include "VoxelAssets/VoxelDataAsset.h"

UVoxelDataAssetFactory::UVoxelDataAssetFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = false;
	bEditAfterNew = true;
	SupportedClass = UVoxelDataAsset::StaticClass();
}

UObject* UVoxelDataAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	auto NewDataAsset = NewObject<UVoxelDataAsset>(InParent, Class, Name, Flags);

	return NewDataAsset;
}
