// Copyright 2018 Phyronnaz

#include "VoxelGrassGroupFactory.h"
#include "AssetTypeCategories.h"
#include "VoxelGrassSpawner.h"

UVoxelGrassGroupFactory::UVoxelGrassGroupFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UVoxelGrassGroup::StaticClass();
}

UObject* UVoxelGrassGroupFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	auto NewGrassType = NewObject<UVoxelGrassGroup>(InParent, Class, Name, Flags | RF_Transactional);

	return NewGrassType;
}
