// Copyright 2018 Phyronnaz

#include "VoxelAssetBuilderFactory.h"
#include "AssetTypeCategories.h"
#include "VoxelAssetBuilder.h"

UVoxelAssetBuilderFactory::UVoxelAssetBuilderFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UVoxelAssetBuilder::StaticClass();
}

UObject* UVoxelAssetBuilderFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	auto NewAsset = NewObject<UVoxelAssetBuilder>(InParent, Class, Name, Flags);

	return NewAsset;
}
