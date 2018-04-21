// Copyright 2018 Phyronnaz

#pragma once

#include "VoxelGrassSpawner.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Runtime/Engine/Private/InstancedStaticMesh.h"
#include "Runtime/Launch/Resources/Version.h"

struct FVoxelGrassBuffer
{
	FVoxelGrassVariety GrassVariety;
	FStaticMeshInstanceData InstanceBuffer;
	TArray<FClusterNode> ClusterTree;
	int OutOcclusionLayerNum;

	FVoxelGrassBuffer()
		: InstanceBuffer(true, true)
	{

	}
};

class FVoxelGrassUtilities
{
public:
	static void InitGrass(UHierarchicalInstancedStaticMeshComponent* NewGrass, const TSharedPtr<FVoxelGrassBuffer>& Buffer)
	{
		const FVoxelGrassVariety& GrassVariety = Buffer->GrassVariety;

		NewGrass->SetStaticMesh(GrassVariety.GrassMesh);
		NewGrass->MinLOD = GrassVariety.MinLOD;
		NewGrass->bSelectable = false;
		NewGrass->bHasPerInstanceHitProxies = false;
		NewGrass->bReceivesDecals = GrassVariety.bReceivesDecals;
		static FName NoCollision(TEXT("BlockAll"));
		NewGrass->SetCollisionProfileName(NoCollision);
		NewGrass->bDisableCollision = true;
		NewGrass->SetCanEverAffectNavigation(false);

		int32 FolSeed = FCrc::StrCrc32((GrassVariety.GrassMesh->GetName()).GetCharArray().GetData());
		if (FolSeed == 0)
		{
			FolSeed++;
		}
		NewGrass->InstancingRandomSeed = FolSeed;
		NewGrass->LightingChannels = GrassVariety.LightingChannels;

		NewGrass->InstanceStartCullDistance = GrassVariety.StartCullDistance;
		NewGrass->InstanceEndCullDistance = GrassVariety.EndCullDistance;

		NewGrass->bAffectDistanceFieldLighting = false;
	}

	static void SetNewPositions(UHierarchicalInstancedStaticMeshComponent* NewGrass, const TSharedPtr<FVoxelGrassBuffer>& Buffer)
	{
#if ENGINE_MINOR_VERSION < 19
		if (Buffer->InstanceBuffer.NumInstances())
#else
		if (Buffer->InstanceBuffer.GetNumInstances())
#endif
		{
#if ENGINE_MINOR_VERSION == 17
			if (!NewGrass->PerInstanceRenderData.IsValid())
			{
				NewGrass->InitPerInstanceRenderData(&Buffer->InstanceBuffer);
			}
			else
			{
				NewGrass->PerInstanceRenderData->UpdateFromPreallocatedData(NewGrass, Buffer->InstanceBuffer);
			}
#else
			if (!NewGrass->PerInstanceRenderData.IsValid())
			{
				NewGrass->InitPerInstanceRenderData(true, &Buffer->InstanceBuffer);
			}
			else
			{
				NewGrass->PerInstanceRenderData->UpdateFromPreallocatedData(NewGrass, Buffer->InstanceBuffer, false);
			}
#endif
			NewGrass->AcceptPrebuiltTree(Buffer->ClusterTree, Buffer->OutOcclusionLayerNum);

			//HierarchicalInstancedStaticMeshComponent->RecreateRenderState_Concurrent();
			NewGrass->MarkRenderStateDirty();
		}
		else
		{
			NewGrass->ClearInstances();
		}
	}
};
