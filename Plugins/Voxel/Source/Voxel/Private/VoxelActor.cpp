// Copyright 2018 Phyronnaz

#include "VoxelActor.h"
#include "VoxelPrivate.h"
#include "VoxelData.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "ProceduralMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"

DECLARE_CYCLE_STAT(TEXT("VoxelActor ~ IsOverlappingVoxelWorld"), STAT_VoxelActor_IsOverlappingVoxelWorld, STATGROUP_Voxel);

TMap<UClass*, TArray<FVector>> AVoxelActor::VerticesByClass;
TMap<UClass*, float> AVoxelActor::HeightByClass;
bool AVoxelActor::bMapsHaveBeenEmptied = true;

AVoxelActor::AVoxelActor()
	: bEnabled(true)
{

}

bool AVoxelActor::IsOverlappingVoxelWorld(AVoxelWorld* World)
{
	return AVoxelActor::IsOverlappingVoxelWorld(GetClass(), World, GetTransform());
}

bool AVoxelActor::IsOverlappingVoxelWorld(TSubclassOf<AVoxelActor> VoxelActorClass, AVoxelWorld* World, const FTransform& Transform)
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelActor_IsOverlappingVoxelWorld);

	if (!VerticesByClass.Contains(VoxelActorClass))
	{
		auto Actor = World->GetWorld()->SpawnActor<AVoxelActor>(VoxelActorClass, FVector::ZeroVector, FRotator::ZeroRotator);
		Actor->Destroy();
	}

	TSet<FIntVector> Positions;
	for (auto& V : VerticesByClass[VoxelActorClass])
	{
		Positions.Append(World->GetNeighboringPositions(Transform.TransformPosition(V)));
	}
	for (auto P : Positions)
	{
		if (World->GetValue(P) <= 0)
		{
			return true;
		}
	}
	return false;
}

float AVoxelActor::GetActorHeight(TSubclassOf<AVoxelActor> VoxelActorClass)
{
	if (!HeightByClass.Contains(VoxelActorClass))
	{
		float Height = VoxelActorClass->GetDefaultObject<AVoxelActor>()->GetHeight();
		HeightByClass.Add(VoxelActorClass, Height);
	}
	return HeightByClass[VoxelActorClass];
}

void AVoxelActor::Enable()
{
	check(!bEnabled);
	bEnabled = true;

	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);

	OnVoxelActorEnable.Broadcast();
}

void AVoxelActor::Disable()
{
	check(bEnabled);
	bEnabled = false;

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);

	OnVoxelActorDisable.Broadcast();
}

void AVoxelActor::StartSimulatingPhysics()
{
	if (!bEnabled)
	{
		Enable();
	}
	OnVoxelActorSimulatePhysics.Broadcast();
}

bool AVoxelActor::IsEnabled()
{
	return bEnabled;
}

float AVoxelActor::GetHeight()
{
	return Height;
}

void AVoxelActor::BeginPlay()
{
	Super::BeginPlay();

	if (!bMapsHaveBeenEmptied)
	{
		VerticesByClass.Empty();
		HeightByClass.Empty();
		bMapsHaveBeenEmptied = true;
	}

	if (!VerticesByClass.Contains(GetClass()))
	{
		TArray<FVector>& Vertices = VerticesByClass.Add(GetClass());
		for (UActorComponent* Component : GetComponents())
		{
			if (Component && Component->GetClass()->IsChildOf(UStaticMeshComponent::StaticClass()))
			{
				UStaticMeshComponent* StaticMeshComponent = CastChecked<UStaticMeshComponent>(Component);
				UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();

				for (int SectionIndex = 0; SectionIndex < StaticMesh->GetNumSections(0); SectionIndex++)
				{
					TArray<FVector> TmpVertices;
					TArray<int32> Triangles;
					TArray<FVector> Normals;
					TArray<FVector2D> UVs;
					TArray<FProcMeshTangent> Tangents;
					UKismetProceduralMeshLibrary::GetSectionFromStaticMesh(StaticMesh, 0, SectionIndex, TmpVertices, Triangles, Normals, UVs, Tangents);
					for (auto& V : TmpVertices)
					{
						Vertices.Add(GetTransform().InverseTransformPosition(StaticMeshComponent->GetComponentTransform().TransformPosition(V)));
					}
				}
			}
		}
	}
}

void AVoxelActor::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	bMapsHaveBeenEmptied = false;
}
