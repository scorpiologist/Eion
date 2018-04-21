// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelImporter.generated.h"

/**
 * Base class of voxel importers
 */
UCLASS(abstract, HideCategories = ("Tick", "Replication", "Input", "Actor", "Rendering", "HLOD"))
class VOXEL_API AVoxelImporter : public AActor
{
	GENERATED_BODY()

public:
	// The path of the asset to create
	UPROPERTY(EditAnywhere, meta = (DisplayName = "Save Path: /Game/", RelativeToGameContentDir), Category = "Save configuration")
	FDirectoryPath SavePath;

	// The name of the asset to create
	UPROPERTY(EditAnywhere, Category = "Save configuration")
	FString FileName;
};
