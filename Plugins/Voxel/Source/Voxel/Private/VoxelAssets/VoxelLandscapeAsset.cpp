// Copyright 2018 Phyronnaz

#include "VoxelAssets/VoxelLandscapeAsset.h"
#include "VoxelWorld.h"
#include "ArchiveSaveCompressedProxy.h"
#include "ArchiveLoadCompressedProxy.h"
#include "VoxelUtilities.h"
#include "BufferArchive.h"
#include "MemoryReader.h"

UVoxelLandscapeAsset::UVoxelLandscapeAsset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, Precision(4)
	, HeightMultiplier(1)
	, HeightOffset(0)
	, ScaleMultiplier(1)
	, bShrink(false)
	, MaxHeight(-1e10)
	, MinHeight(1e10)
	, AdditionalThickness(1000000)
{
};

TSharedRef<FVoxelAssetInstance> UVoxelLandscapeAsset::GetAssetInternal(const FIntVector& Position) const
{
	return MakeShareable(new FVoxelLandscapeAssetInstance(
		Heights,
		Materials,
		Width,
		Height,
		MaxHeight,
		MinHeight,
		Precision,
		HeightMultiplier,
		HeightOffset,
		AdditionalThickness,
		ScaleMultiplier,
		bShrink,
		Position));
}

void UVoxelLandscapeAsset::SetSize(int InWidth, int InHeight, bool bInitialize)
{
	Width = InWidth;
	Height = InHeight;

	if (bInitialize)
	{
		Heights.SetNum(Width * Height);
		Materials.SetNum(Width * Height);
	}
	else
	{
		Heights.SetNumUninitialized(Width * Height);
		Materials.SetNumUninitialized(Width * Height);
	}
}

int UVoxelLandscapeAsset::GetTerrainWidth()
{
	return Width;
}

int UVoxelLandscapeAsset::GetTerrainHeight()
{
	return Height;
}

void UVoxelLandscapeAsset::SetHeight(int X, int Y, float NewHeight)
{
	check(0 <= X && X < Width);
	check(0 <= Y && Y < Height);
	MaxHeight = FMath::Max(MaxHeight, NewHeight);
	MinHeight = FMath::Min(MinHeight, NewHeight);
	Heights[X + Width * Y] = NewHeight;
}

void UVoxelLandscapeAsset::SetMaterial(int X, int Y, FVoxelMaterial Material)
{
	check(0 <= X && X < Width);
	check(0 <= Y && Y < Height);
	Materials[X + Width * Y] = Material;
}

float UVoxelLandscapeAsset::GetHeight(int X, int Y) const
{
	check(0 <= X && X < Width);
	check(0 <= Y && Y < Height);
	return Heights[X + Width * Y];
}

FVoxelMaterial UVoxelLandscapeAsset::GetMaterial(int X, int Y) const
{
	check(0 <= X && X < Width);
	check(0 <= Y && Y < Height);
	return Materials[X + Width * Y];
}

void UVoxelLandscapeAsset::SetPrecomputedValues(const TArray<float>& InHeights, const TArray<FVoxelMaterial>& InMaterials, int InWidth, int InHeight, float InMaxHeight, float InMinHeight)
{
	Heights = InHeights;
	Materials = InMaterials;

	Width = InWidth;
	Height = InHeight;

	MaxHeight = InMaxHeight;
	MinHeight = InMinHeight;
}

void UVoxelLandscapeAsset::Save()
{
	FBufferArchive Archive;

	// Heights are too random to benefit from RLE
	Archive << Heights;
	TArray<uint8> TmpData;
	FVoxelUtilities::CompressRLE(Materials, TmpData);
	Archive << TmpData;
	Archive << Width;
	Archive << Height;
	Archive << MaxHeight;
	Archive << MinHeight;

	int32 UncompressedSize = Archive.Num();
	int32 CompressedSize = FCompression::CompressMemoryBound(CompressionFlags, UncompressedSize);

	CompressedData.SetNumUninitialized(CompressedSize + sizeof(UncompressedSize));

	FMemory::Memcpy(&CompressedData[0], &UncompressedSize, sizeof(UncompressedSize));
	verify(FCompression::CompressMemory(CompressionFlags, CompressedData.GetData() + sizeof(UncompressedSize), CompressedSize, Archive.GetData(), Archive.Num()));
	CompressedData.SetNum(CompressedSize + sizeof(UncompressedSize));
}

void UVoxelLandscapeAsset::LoadInternal()
{
	TArray<uint8> Data;

	int32 UncompressedSize;
	FMemory::Memcpy(&UncompressedSize, &CompressedData[0], sizeof(UncompressedSize));
	Data.SetNum(UncompressedSize);
	verify(FCompression::UncompressMemory(CompressionFlags, Data.GetData(), UncompressedSize, CompressedData.GetData() + sizeof(UncompressedSize), CompressedData.Num() - sizeof(UncompressedSize)));
	
	FMemoryReader Reader(Data);
	Reader << Heights;
	TArray<uint8> TmpData;
	Reader << TmpData;
	FVoxelUtilities::DecompressRLE(TmpData, Materials);
	Reader << Width;
	Reader << Height;
	Reader << MaxHeight;
	Reader << MinHeight;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

FVoxelLandscapeAssetInstance::FVoxelLandscapeAssetInstance(
	const TArray<float>& Heights,
	const TArray<FVoxelMaterial>& Materials,
	int Width,
	int Height,
	float MaxHeight,
	float MinHeight,
	int Precision,
	float HeightMultiplier,
	float HeightOffset,
	float AdditionalThickness,
	int ScaleMultiplier,
	bool bShrink,
	const FIntVector& Position)
	: Heights(Heights)
	, Materials(Materials)
	, Width(Width)
	, Height(Height)
	, MaxHeight(MaxHeight)
	, MinHeight(MinHeight)
	, Precision(Precision)
	, HeightMultiplier(HeightMultiplier)
	, HeightOffset(HeightOffset)
	, AdditionalThickness(AdditionalThickness)
	, ScaleMultiplier(ScaleMultiplier)
	, bShrink(bShrink)
	, FVoxelAssetInstance(Position)
{

}

void FVoxelLandscapeAssetInstance::GetValuesAndMaterialsAndVoxelTypes(float InValues[], FVoxelMaterial InMaterials[], FVoxelType InVoxelTypes[], const FIntVector& Start, const FIntVector& StartIndex, const int Step, const FIntVector& InSize, const FIntVector& ArraySize) const
{
	const FIntBox Bounds = GetLocalBounds();
	for (int I = 0; I < InSize.X; I++)
	{
		const int X = Start.X + I * Step - Position.X;

		for (int J = 0; J < InSize.Y; J++)
		{
			const int Y = Start.Y + J * Step - Position.Y;

			const int IndexX = bShrink ? (X * ScaleMultiplier) : (X / ScaleMultiplier);
			const int IndexY = bShrink ? (Y * ScaleMultiplier) : (Y / ScaleMultiplier);

			if (0 <= IndexX && IndexX < Width && 0 <= IndexY && IndexY < Height)
			{
				const float CurrentHeight = GetHeight(IndexX, IndexY) * HeightMultiplier + HeightOffset;

				for (int K = 0; K < InSize.Z; K++)
				{
					const int Z = Start.Z + K * Step - Position.Z;

					const int Index = (StartIndex.X + I) + ArraySize.X * (StartIndex.Y + J) + ArraySize.X * ArraySize.Y * (StartIndex.Z + K);

					if (InValues)
					{
						float Value;
						if (CurrentHeight > (Z + Precision) * VoxelSize)
						{
							// If voxel over us is in, we're entirely in unless we're under the map
							if (Bounds.Min.Z <= Z && Z <= Bounds.Max.Z)
							{
								Value = -1;
							}
							else
							{
								Value = 1;
							}
						}
						else if ((Z - Precision) * VoxelSize > CurrentHeight)
						{
							// If voxel under us is out, we're entirely out
							Value = 1;
						}
						else
						{
							float Alpha = (Z * VoxelSize - CurrentHeight) / VoxelSize / Precision;
							Value = Alpha;
						}
						InValues[Index] = Value;
					}
					if (InMaterials)
					{
						InMaterials[Index] = GetMaterial(IndexX, IndexY);
					}
					if (InVoxelTypes)
					{
						FVoxelType VoxelType;
						if ((Z - Precision) * VoxelSize <= CurrentHeight || CurrentHeight <= (Z + Precision) * VoxelSize)
						{
							bool bUseMaterial = (Z - Precision + 1) * VoxelSize <= CurrentHeight;
							if (Z * VoxelSize - CurrentHeight <= 0)
							{
								VoxelType = FVoxelType::UseAll();
							}
							else
							{
								VoxelType = FVoxelType(EVoxelValueType::UseValueIfSameSign, bUseMaterial ? EVoxelMaterialType::UseMaterial : EVoxelMaterialType::IgnoreMaterial);
							}
						}
						else
						{
							VoxelType = FVoxelType::IgnoreAll();
						}
						InVoxelTypes[Index] = VoxelType;
					}
				}
			}
			else
			{
				for (int K = 0; K < InSize.Z; K++)
				{
					const int Z = Start.Z + K * Step - Position.Z;

					const int Index = (StartIndex.X + I) + ArraySize.X * (StartIndex.Y + J) + ArraySize.X * ArraySize.Y * (StartIndex.Z + K);

					if (InValues)
					{
						InValues[Index] = 1;
					}
					if (InMaterials)
					{
						InMaterials[Index] = FVoxelMaterial();
					}
					if (InVoxelTypes)
					{
						InVoxelTypes[Index] = FVoxelType::IgnoreAll();
					}
				}
			}
		}
	}
}

bool FVoxelLandscapeAssetInstance::IsAssetEmpty(const FIntVector& Start, const int Step, const FIntVector& Size) const
{
	FIntBox Box = FIntBox(Start - Position , Start - Position + Size * Step);
	FIntBox InfiniteZBox = GetLocalBounds();
	InfiniteZBox.Min.Z = MIN_int32;
	InfiniteZBox.Max.Z = MAX_int32;

	if (Box.Intersect(InfiniteZBox) && !InfiniteZBox.Contains(Box))
	{
		return false;
	}
	else
	{
		if (MaxHeight * HeightMultiplier + HeightOffset < (Box.Min.Z - Precision) * VoxelSize)
		{
			// Box above the terrain
			return true;
		}
		else
		{
			if ((Box.Max.Z + Precision) * VoxelSize < MinHeight * HeightMultiplier + HeightOffset)
			{
				// Box under the terrain

				float LowerBorder = HeightOffset - AdditionalThickness + MinHeight * HeightMultiplier;

				return (Box.Max.Z + Precision) * VoxelSize < LowerBorder // Below lower border
					|| LowerBorder < (Box.Min.Z - Precision) * VoxelSize; // Between lower border and terrain
			}
			else
			{
				return false;
			}
		}
	}
}

void FVoxelLandscapeAssetInstance::SetVoxelWorld(const AVoxelWorld* VoxelWorld)
{
	VoxelSize = VoxelWorld->GetVoxelSize();
}

FIntBox FVoxelLandscapeAssetInstance::GetLocalBounds() const
{
	const int FinalWidth = bShrink ? FMath::FloorToInt((double)Width / (double)ScaleMultiplier) : (Width * ScaleMultiplier);
	const int FinalHeight = bShrink ? FMath::FloorToInt((double)Height / (double)ScaleMultiplier) : (Height * ScaleMultiplier);

	FIntBox Box;
	Box.Min = FIntVector(0, 0, FMath::FloorToInt(-Precision + (HeightOffset - AdditionalThickness + MinHeight * HeightMultiplier) / VoxelSize));
	Box.Max = FIntVector(FinalWidth, FinalHeight, FMath::CeilToInt(Precision + (HeightOffset + MaxHeight * HeightMultiplier) / VoxelSize));

	return Box;
}

float FVoxelLandscapeAssetInstance::GetHeight(int X, int Y) const
{
	check(0 <= X && X < Width);
	check(0 <= Y && Y < Height);
	return Heights[X + Width * Y];
}

FVoxelMaterial FVoxelLandscapeAssetInstance::GetMaterial(int X, int Y) const
{
	check(0 <= X && X < Width);
	check(0 <= Y && Y < Height);
	return Materials[X + Width * Y];
}
