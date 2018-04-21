// Copyright 2018 Phyronnaz

#include "VoxelImporters/VoxelWorldSectionImporter.h"
#include "VoxelWorld.h"
#include "VoxelData.h"
#include "DrawDebugHelpers.h"
#include "VoxelUtilities.h"


AVoxelWorldSectionImporter::AVoxelWorldSectionImporter()
	: World(nullptr)
	, TopCorner(1, 1, 1)
	, BottomCorner(-1, -1, -1)
{
	PrimaryActorTick.bCanEverTick = true;
};

void AVoxelWorldSectionImporter::ImportToAsset(UVoxelDataAsset& Asset)
{
	check(World);
	FIntVector Size = TopCorner - BottomCorner;
	Asset.SetSize(Size, false);

	FVoxelData* Data = World->GetData();

	{
		auto Octrees = Data->BeginGet(FIntBox(BottomCorner, BottomCorner + Size));
		for (int X = 0; X < Size.X; X++)
		{
			for (int Y = 0; Y < Size.Y; Y++)
			{
				for (int Z = 0; Z < Size.Z; Z++)
				{
					FIntVector P = BottomCorner + FIntVector(X, Y, Z);
					if (LIKELY(Data->IsInWorld(P.X, P.Y, P.Z)))
					{
						float Value;
						FVoxelMaterial Material;
						Data->GetValueAndMaterial(P.X, P.Y, P.Z, Value, Material);

						Asset.SetValue(X, Y, Z, Value);
						Asset.SetMaterial(X, Y, Z, Material);

						Asset.SetVoxelType(X, Y, Z, FVoxelUtilities::GetVoxelTypeFromValue(Value));
					}
					else
					{
						Asset.SetValue(X, Y, Z, 0);
						Asset.SetMaterial(X, Y, Z, FVoxelMaterial(0, 0, 0, 0));
						Asset.SetVoxelType(X, Y, Z, FVoxelType::IgnoreAll());
					}
				}
			}
		}
		Data->EndGet(Octrees);
	}
}

void AVoxelWorldSectionImporter::SetCornersFromActors()
{
	check(World);
	if (BottomActor)
	{
		BottomCorner = World->GlobalToLocal(BottomActor->GetActorLocation());
	}
	if (TopActor)
	{
		TopCorner = World->GlobalToLocal(TopActor->GetActorLocation());
	}
	if (BottomCorner.X > TopCorner.X)
	{
		BottomCorner.X = TopCorner.X;
	}
	if (BottomCorner.Y > TopCorner.Y)
	{
		BottomCorner.Y = TopCorner.Y;
	}
	if (BottomCorner.Z > TopCorner.Z)
	{
		BottomCorner.Z = TopCorner.Z;
	}
}

#if WITH_EDITOR
void AVoxelWorldSectionImporter::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (BottomCorner.X > TopCorner.X)
	{
		BottomCorner.X = TopCorner.X;
	}
	if (BottomCorner.Y > TopCorner.Y)
	{
		BottomCorner.Y = TopCorner.Y;
	}
	if (BottomCorner.Z > TopCorner.Z)
	{
		BottomCorner.Z = TopCorner.Z;
	}
}

void AVoxelWorldSectionImporter::Tick(float Deltatime)
{
	if (World)
	{
		FVector Bottom = World->LocalToGlobal(BottomCorner);
		FVector Top = World->LocalToGlobal(TopCorner);
		DrawDebugBox(GetWorld(), (Bottom + Top) / 2, (Top - Bottom) / 2, FColor::Red, false, 2 * Deltatime, 0, 10);
	}
}
#endif
