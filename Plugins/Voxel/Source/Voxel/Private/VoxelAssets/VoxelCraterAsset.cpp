// Copyright 2018 Phyronnaz

#include "VoxelAssets/VoxelCraterAsset.h"

UVoxelCraterAsset::UVoxelCraterAsset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, Radius(5)
{
};

TSharedRef<FVoxelAssetInstance> UVoxelCraterAsset::GetAssetInternal(const FIntVector& Position) const
{
	return MakeShareable(new FVoxelCraterAssetInstance(Radius, Material, Position));
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////


FVoxelCraterAssetInstance::FVoxelCraterAssetInstance(float Radius, const FVoxelMaterial& Material, const FIntVector& Position)
	: Radius(Radius)
	, Material(Material)
	, FVoxelAssetInstance(Position)
{

}

void FVoxelCraterAssetInstance::GetValuesAndMaterialsAndVoxelTypes(float Values[], FVoxelMaterial Materials[], FVoxelType VoxelTypes[], const FIntVector& Start, const FIntVector& StartIndex, const int Step, const FIntVector& InSize, const FIntVector& ArraySize) const
{
	FIntBox Bounds = GetLocalBounds();
	for (int I = 0; I < InSize.X; I++)
	{
		const int X = Start.X + I * Step - Position.X;

		for (int J = 0; J < InSize.Y; J++)
		{
			const int Y = Start.Y + J * Step - Position.Y;

			for (int K = 0; K < InSize.Z; K++)
			{
				const int Z = Start.Z + K * Step - Position.Z;

				const int Index = (StartIndex.X + I) + ArraySize.X * (StartIndex.Y + J) + ArraySize.X * ArraySize.Y * (StartIndex.Z + K);

				bool bValid = Bounds.IsInside(X, Y, Z);

				if (bValid)
				{
					const FIntVector CurrentPosition = FIntVector(X, Y, Z);

					float CurrentRadius = FVector(X, Y, Z).Size();
					float Distance = CurrentRadius;

					if (Distance <= Radius + 3)
					{
						float CurrentNoise = Noise.GetWhiteNoise((X + Position.X) / CurrentRadius, (Y + Position.Y) / CurrentRadius, (Z + Position.Z) / CurrentRadius);
						Distance -= CurrentNoise;
					}

					if (Distance <= Radius + 2)
					{
						// We want (Radius - Distance) != 0
						const float NoiseValue = (Radius - Distance == 0) ? 0.0001f : 0;
						float Value = FMath::Clamp(Radius - Distance + NoiseValue, -2.f, 2.f) / 2;

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
							VoxelTypes[Index] = FVoxelType(Value > 0 ? EVoxelValueType::UseValue : EVoxelValueType::UseValueIfSameSign, EVoxelMaterialType::UseMaterial);
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

FIntBox FVoxelCraterAssetInstance::GetLocalBounds() const
{
	FIntVector Bound = FIntVector(1, 1, 1) * FMath::CeilToInt(Radius + 4);

	FIntBox Box;
	Box.Min = Bound * -1;
	Box.Max = Bound;

	return Box;
}
