// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelImporter.h"
#include "VoxelMagicaVoxelImporter.generated.h"

/**
 * Actor that create a UVoxelDataAsset from a MagicalVoxel file
 */
UCLASS(BlueprintType, HideCategories = ("Tick", "Replication", "Input", "Actor", "Rendering", "HLOD"))
class VOXEL_API AVoxelMagicaVoxelImporter : public AVoxelImporter
{
	GENERATED_BODY()

public:
	// The MagicaVoxel file
	UPROPERTY(EditAnywhere, Category = "Import configuration")
	FFilePath File;
};
