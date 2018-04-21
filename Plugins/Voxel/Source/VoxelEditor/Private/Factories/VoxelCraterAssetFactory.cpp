// Copyright 2018 Phyronnaz

#include "VoxelCraterAssetFactory.h"
#include "AssetTypeCategories.h"
#include "VoxelAssets/VoxelCraterAsset.h"

UVoxelCraterAssetFactory::UVoxelCraterAssetFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UVoxelCraterAsset::StaticClass();
}

UObject* UVoxelCraterAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	auto NewDataAsset = NewObject<UVoxelCraterAsset>(InParent, Class, Name, Flags);

	return NewDataAsset;
}
