// Copyright 2018 Phyronnaz

#include "VoxelActorGroupFactory.h"
#include "VoxelActorSpawner.h"
#include "AssetTypeCategories.h"

UVoxelActorGroupFactory::UVoxelActorGroupFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UVoxelActorGroup::StaticClass();
}

UObject* UVoxelActorGroupFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	auto NewGrassType = NewObject<UVoxelActorGroup>(InParent, Class, Name, Flags | RF_Transactional);

	return NewGrassType;
}
