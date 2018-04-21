// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelAssets/VoxelDataAsset.h"
#include "VoxelImporter.h"
#include "VoxelMeshImporter.generated.h"

class UStaticMesh;

/**
 * Actor that creates a VoxelDataAsset from a static mesh
 */
UCLASS(BlueprintType, HideCategories = ("Tick", "Replication", "Input", "Actor", "Rendering", "HLOD"))
class VOXEL_API AVoxelMeshImporter : public AVoxelImporter
{
	GENERATED_BODY()

public:
	// The static mesh to import from
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "Import configuration")
	UStaticMesh* StaticMesh;

	// The material to set
	UPROPERTY(EditAnywhere, Category = "Import configuration")
	FVoxelMaterial Material;

	// Size of the voxels used to voxelize the mesh
	UPROPERTY(EditAnywhere, Category = "Import configuration", meta = (ClampMin = "0", UIMin = "0"))
	float MeshVoxelSize;

	// UpscalingFactor^3 = Number of voxels of MeshVoxelSize size per real voxel
	UPROPERTY(EditAnywhere, Category = "Import configuration", meta = (ClampMin = "2", UIMin = "2"))
	int UpscalingFactor;

	// One actor per part of the mesh. Used to apply a paint bucket algorithm starting from those
	UPROPERTY(EditAnywhere, Category = "Import configuration")
	TArray<AActor*> ActorsInsideTheMesh;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDrawPoints;

	AVoxelMeshImporter();

	void ImportToAsset(UVoxelDataAsset& Asset);

protected:
#if WITH_EDITOR
	//~ Begin UObject Interface
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	//~ End UObject Interface
#endif

private:
	UPROPERTY()
	UStaticMeshComponent* MeshComponent;
};
