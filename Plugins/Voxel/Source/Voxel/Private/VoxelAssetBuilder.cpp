// Copyright 2018 Phyronnaz

#include "VoxelAssetBuilder.h"

UVoxelAssetBuilder::UVoxelAssetBuilder(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
};

TSharedRef<FVoxelAssetInstance> UVoxelAssetBuilder::GetAssetInternal(const FIntVector& Position) const
{
	return MakeShareable(new FVoxelAssetBuilderInstance(WorldGenerator.GetWorldGenerator(), Position, Bounds, Offset));
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

FVoxelAssetBuilderInstance::FVoxelAssetBuilderInstance(TSharedRef<FVoxelWorldGeneratorInstance> WorldGenerator, const FIntVector& Position, const FIntBox& Bounds, const FIntVector& Offset)
	: WorldGenerator(WorldGenerator)
	, Bounds(Bounds)
	, Offset(Offset)
	, FVoxelAssetInstance(Position)
{

}

void FVoxelAssetBuilderInstance::GetValuesAndMaterialsAndVoxelTypes(float Values[], FVoxelMaterial Materials[], FVoxelType VoxelTypes[], const FIntVector& Start, const FIntVector& StartIndex, const int Step, const FIntVector& Size, const FIntVector& ArraySize) const
{
	if (GetWorldBounds().Contains(FIntBox(Start, Start + Size * Step)))
	{
		WorldGenerator->GetValuesAndMaterialsAndVoxelTypes(Values, Materials, VoxelTypes, Start - (Position + Offset), StartIndex, Step, Size, ArraySize);
	}
	else
	{
		for (int I = 0; I < Size.X; I++)
		{
			const int X = Start.X + I * Step - (Position.X + Offset.X);

			for (int J = 0; J < Size.Y; J++)
			{
				const int Y = Start.Y + J * Step - (Position.Y + Offset.Y);

				for (int K = 0; K < Size.Z; K++)
				{
					const int Z = Start.Z + K * Step - (Position.Z + Offset.Z);

					const int Index = (StartIndex.X + I) + ArraySize.X * (StartIndex.Y + J) + ArraySize.X * ArraySize.Y * (StartIndex.Z + K);

					bool bValid = Bounds.IsInside(X, Y, Z);

					if (bValid)
					{
						float Value;
						FVoxelMaterial Material;
						FVoxelType VoxelType;
						WorldGenerator->GetValueAndMaterialAndVoxelType(X, Y, Z, Value, Material, VoxelType);

						if (Values)
						{
							Values[Index] = Value;
						}
						if (Materials)
						{
							Materials[Index] = Material;
						}
						if (VoxelTypes)
						{
							VoxelTypes[Index] = VoxelType;
						}
					}
					else
					{
						if (Values)
						{
							Values[Index] = 1;
						}
						if (Materials)
						{
							Materials[Index] = FVoxelMaterial();
						}
						if (VoxelTypes)
						{
							VoxelTypes[Index] = FVoxelType::IgnoreAll();
						}
					}
				}
			}
		}
	}
}

bool FVoxelAssetBuilderInstance::IsAssetEmpty(const FIntVector& Start, const int Step, const FIntVector& Size) const
{
	return WorldGenerator->IsEmpty(Start - (Position + Offset), Step, Size);
}

FIntBox FVoxelAssetBuilderInstance::GetLocalBounds() const
{
	return Bounds.TranslateBy(Offset);
}

void FVoxelAssetBuilderInstance::SetVoxelWorld(const AVoxelWorld* VoxelWorld)
{
	WorldGenerator->SetVoxelWorld(VoxelWorld);
}

