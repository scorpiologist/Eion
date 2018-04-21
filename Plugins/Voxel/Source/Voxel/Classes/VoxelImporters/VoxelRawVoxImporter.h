// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelImporter.h"
#include "VoxelRawVoxImporter.generated.h"

/**
 * Actor that create a VoxelDataAsset from an .rawvox
 */
UCLASS(BlueprintType, HideCategories = ("Tick", "Replication", "Input", "Actor", "Rendering", "HLOD"))
class VOXEL_API AVoxelRawVoxImporter : public AVoxelImporter
{
	GENERATED_BODY()

public:
	// The .rawvox
	UPROPERTY(EditAnywhere, Category = "Import configuration")
	FFilePath File;
};
