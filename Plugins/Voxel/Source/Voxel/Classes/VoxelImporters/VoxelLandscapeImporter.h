// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Landscape.h"
#include "LandscapeLayerInfoObject.h"
#include "VoxelImporter.h"
#include "VoxelLandscapeImporter.generated.h"

/**
 * Hold informations for landscape layers import
 */
USTRUCT()
struct VOXEL_API FLayerInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	ULandscapeLayerInfoObject* Layer;

	UPROPERTY(EditAnywhere)
	uint8 Material;
};

/**
 * Actor that create a UVoxelLandscapeAsset from an ALandscape
 */
UCLASS(BlueprintType, HideCategories = ("Tick", "Replication", "Input", "Actor", "Rendering", "HLOD"))
class VOXEL_API AVoxelLandscapeImporter : public AVoxelImporter
{
	GENERATED_BODY()

public:
	// The landscape to import
	UPROPERTY(EditAnywhere, Category = "Import configuration")
	ALandscape* Landscape;

	// The layers to import
	UPROPERTY(EditAnywhere, Category = "Import configuration")
	TArray<FLayerInfo> LayerInfos;
};
