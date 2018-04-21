// Copyright 2018 Phyronnaz

#include "VoxelGrassSpawnerFactory.h"
#include "VoxelGrassSpawner.h"
#include "AssetTypeCategories.h"

UVoxelGrassSpawnerFactory::UVoxelGrassSpawnerFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UVoxelGrassSpawner::StaticClass();
}

UObject* UVoxelGrassSpawnerFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	auto NewGrassType = NewObject<UVoxelGrassSpawner>(InParent, Class, Name, Flags | RF_Transactional);

	return NewGrassType;
}
