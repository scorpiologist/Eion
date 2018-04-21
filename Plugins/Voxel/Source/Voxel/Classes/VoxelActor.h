// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorld.h"
#include "SubclassOf.h"
#include "VoxelActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVoxelActorEnable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVoxelActorDisable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVoxelActorSimulatePhysics);

UCLASS()
class VOXEL_API AVoxelActor : public AActor
{
	GENERATED_BODY()

public:
	AVoxelActor();

	UFUNCTION(BlueprintCallable)
	bool IsOverlappingVoxelWorld(AVoxelWorld* World);

	static bool IsOverlappingVoxelWorld(TSubclassOf<AVoxelActor> VoxelActorClass, AVoxelWorld* World, const FTransform& Transform);
	
	static float GetActorHeight(TSubclassOf<AVoxelActor> VoxelActorClass);

	void Enable();
	void Disable();

	void StartSimulatingPhysics();

	FORCEINLINE bool IsEnabled();
	FORCEINLINE float GetHeight();

protected:
	// In cm
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Height;

	UPROPERTY(BlueprintAssignable)
	FOnVoxelActorEnable OnVoxelActorEnable;

	UPROPERTY(BlueprintAssignable)
	FOnVoxelActorDisable OnVoxelActorDisable;

	UPROPERTY(BlueprintAssignable)
	FOnVoxelActorSimulatePhysics OnVoxelActorSimulatePhysics;

	//~ AActor Interface Start
	void BeginPlay() override;
	void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	//~ AActor Interface End

private:
	static TMap<UClass*, TArray<FVector>> VerticesByClass;
	static TMap<UClass*, float> HeightByClass;
	static bool bMapsHaveBeenEmptied;

	bool bEnabled;
};
