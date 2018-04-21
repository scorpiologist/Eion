// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorld.h"
#include "VoxelImporter.h"
#include "VoxelAssets/VoxelDataAsset.h"
#include "VoxelWorldSectionImporter.generated.h"

/**
 * Actor that create a VoxelDataAsset from a part of the voxel world
 */
UCLASS(BlueprintType, HideCategories = ("Tick", "Replication", "Input", "Actor", "Rendering", "HLOD"))
class VOXEL_API AVoxelWorldSectionImporter : public AVoxelImporter
{
	GENERATED_BODY()

public:
	// The world to import from
	UPROPERTY(EditAnywhere, Category = "Import configuration")
	AVoxelWorld* World;

	// The top corner of the section
	UPROPERTY(EditAnywhere, Category = "Import configuration")
	FIntVector TopCorner;

	// The lower corner of the section
	UPROPERTY(EditAnywhere, Category = "Import configuration")
		FIntVector BottomCorner;

	// Optional. Can be easier to select top corner with an actor
	UPROPERTY(EditAnywhere, Category = "Import configuration")
	AActor* TopActor;

	// Optional. Can be easier to select bottom corner with an actor
	UPROPERTY(EditAnywhere, Category = "Import configuration")
	AActor* BottomActor;

	AVoxelWorldSectionImporter();

	void ImportToAsset(UVoxelDataAsset& Asset);
	void SetCornersFromActors();

protected:
#if WITH_EDITOR
	//~ Begin AActor Interface
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	void Tick(float Deltatime) override;
	//~ End AActor Interface
#endif
};
