// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.h"
#include "VoxelAssets/VoxelDataAsset.h"
#include "VoxelImporter.h"
#include "Components/SphereComponent.h"
#include "VoxelSplineImporter.generated.h"

class USplineComponent;

/**
 * Actor that convert splines to voxels
 */
UCLASS(Blueprintable, HideCategories = ("Tick", "Replication", "Input", "Actor", "Rendering", "HLOD"))
class VOXEL_API AVoxelSplineImporter : public AVoxelImporter
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Splines configuration")
	TArray<USplineComponent*> Splines;

	UPROPERTY(EditAnywhere, Category = "Splines configuration")
	bool bSetMaterial;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bSetMaterial"), Category = "Splines configuration")
	FVoxelMaterial Material;

	UPROPERTY(EditAnywhere, Category = "Import configuration")
	float VoxelSize;


	AVoxelSplineImporter();

	void ImportToAsset(UVoxelDataAsset& Asset);

protected:
#if WITH_EDITOR
	//~ Begin AActor Interface
	void Tick(float DeltaTime) override;
	bool ShouldTickIfViewportsOnly() const override;
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	//~ End AActor Interface
#endif

private:
	UPROPERTY()
	TArray<USphereComponent*> Spheres;
};
