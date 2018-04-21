// Copyright 2018 Phyronnaz

#include "VoxelActorSpawnerFactory.h"
#include "VoxelActorSpawner.h"
#include "AssetTypeCategories.h"

UVoxelActorSpawnerFactory::UVoxelActorSpawnerFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UVoxelActorSpawner::StaticClass();
}

UObject* UVoxelActorSpawnerFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	auto NewGrassType = NewObject<UVoxelActorSpawner>(InParent, Class, Name, Flags | RF_Transactional);

	return NewGrassType;
}
