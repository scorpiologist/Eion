// Copyright 2018 Phyronnaz

#include "VoxelActorOctree.h"
#include "VoxelPrivate.h"
#include "VoxelGlobals.h"
#include "VoxelActor.h"

FVoxelActorOctree::FVoxelActorOctree(uint8 LOD, float MaxRenderDistance)
	: TVoxelOctree(LOD)
	, MaxRenderDistanceSquared(FMath::CeilToInt(FMath::Square(MaxRenderDistance)))
	, bOctreeIsDisabled(true)
{

}

FVoxelActorOctree::FVoxelActorOctree(FVoxelActorOctree* Parent, uint8 ChildIndex)
	: TVoxelOctree(Parent, ChildIndex)
	, MaxRenderDistanceSquared(Parent->MaxRenderDistanceSquared)
	, bOctreeIsDisabled(true)
{

}

void FVoxelActorOctree::AddActor(AVoxelActor* Actor, const FIntVector& InPosition)
{
	if (LOD == 0)
	{
		check(!Actors.ContainsByPredicate([&](FVoxelActorWithPosition P) { return P.Actor == Actor; }));
		FVoxelActorWithPosition ActorWithPosition;
		ActorWithPosition.Actor = Actor;
		ActorWithPosition.Position = InPosition;
		Actors.Add(ActorWithPosition);
		if (bOctreeIsDisabled)
		{
			if (Actor->IsEnabled())
			{
				Actor->Disable();
			}
		}
		else
		{
			if (!Actor->IsEnabled())
			{
				Actor->Enable();
			}
		}
	}
	else
	{
		if (IsLeaf())
		{
			CreateChilds();
		}
		GetLeaf(InPosition)->AddActor(Actor, InPosition);
	}
}

bool FVoxelActorOctree::RemoveActor(AVoxelActor* Actor)
{
	if (IsLeaf())
	{
		int32 RemovedActors = Actors.RemoveAll([&](FVoxelActorWithPosition P) { return P.Actor == Actor; });
		check(RemovedActors <= 1);

		return RemovedActors > 0;
	}
	else
	{
		for (auto Child : GetChilds())
		{
			if (Child->RemoveActor(Actor))
			{
				return true;
			}
		}
		return false;
	}
}

void FVoxelActorOctree::UpdateVisibility(const TArray<FIntVector>& CameraPositions)
{
	bool bDisable = true;
	for (auto Camera : CameraPositions)
	{
		if (GetBounds().ComputeSquaredDistanceFromBoxToPoint(Camera) < MaxRenderDistanceSquared)
		{
			bDisable = false;
			break;
		}
	}

	if (bDisable)
	{
		if (!bOctreeIsDisabled)
		{
			SetIsEnabled(false);
		}
	}
	else
	{
		if (bOctreeIsDisabled)
		{
			SetIsEnabled(true);
		}
		for (auto Child : GetChilds())
		{
			Child->UpdateVisibility(CameraPositions);
		}
	}
}

void FVoxelActorOctree::GetActorsInBox(const FIntBox& Box, TArray<AVoxelActor*>& OutActors)
{
	TArray<FVoxelActorOctree*> Octrees;
	GetLeavesOverlappingBox(Box, Octrees);

	for (auto Octree : Octrees)
	{
		for (auto P : Octree->Actors)
		{
			if (Box.IsInside(P.Position))
			{
				OutActors.Add(P.Actor);
			}
		}
	}
}

void FVoxelActorOctree::SetIsEnabled(bool bIsEnabled)
{
	if (bOctreeIsDisabled != !bIsEnabled)
	{
		bOctreeIsDisabled = !bIsEnabled;

		for (auto P : Actors)
		{
			if (bOctreeIsDisabled)
			{
				check(P.Actor->IsEnabled());
				{
					P.Actor->Disable();
				}
			}
			else
			{
				check(!P.Actor->IsEnabled());
				{
					P.Actor->Enable();
				}
			}
		}

		if (bOctreeIsDisabled)
		{
			for (auto Child : GetChilds())
			{
				Child->SetIsEnabled(false);
			}
		}
	}
}
