// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelImporter.h"
#include "VoxelHeightmapImporter.generated.h"

/**
 * Hold informations for a weightmap import
 */
USTRUCT()
struct VOXEL_API FVoxelWeightmapImporter
{
	GENERATED_BODY()

	// The weightmap
	UPROPERTY(EditAnywhere)
	FFilePath File;

	// The material to set for this weightmap
	UPROPERTY(EditAnywhere)
	uint8 Material;

	// The min value of this weightmap
	UPROPERTY(EditAnywhere, meta = (AdvancedDisplay))
	uint16 MinValue = 0;

	// The max value of this weightmap
	UPROPERTY(EditAnywhere, meta = (AdvancedDisplay))
	uint16 MaxValue = 65535;
};

/**
 * Actor that create a UVoxelLandscapeAsset from an heightmap and its weightmaps
 */
UCLASS(BlueprintType, HideCategories = ("Tick", "Replication", "Input", "Actor", "Rendering", "HLOD"))
class VOXEL_API AVoxelHeightmapImporter : public AVoxelImporter
{
	GENERATED_BODY()

public:
	// The path of the heightmap
	UPROPERTY(EditAnywhere, Category = "Import configuration")
	FFilePath Heightmap;

	// The weightmaps
	UPROPERTY(EditAnywhere, Category = "Import configuration")
	TArray<FVoxelWeightmapImporter> Weightmaps;
};
