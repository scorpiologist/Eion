// Copyright 2018 Phyronnaz

#include "VoxelGraphWorldGeneratorAssetFactory.h"
#include "AssetTypeCategories.h"
#include "VoxelGraphWorldGenerator.h"

UVoxelGraphWorldGeneratorAssetFactory::UVoxelGraphWorldGeneratorAssetFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UVoxelGraphGenerator::StaticClass();
}

UObject* UVoxelGraphWorldGeneratorAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	auto NewAsset = NewObject<UVoxelGraphGenerator>(InParent, Class, Name, Flags);

	return NewAsset;
}
