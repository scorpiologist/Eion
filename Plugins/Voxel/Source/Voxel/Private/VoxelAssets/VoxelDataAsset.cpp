// Copyright 2018 Phyronnaz

#include "VoxelAssets/VoxelDataAsset.h"
#include "VoxelUtilities.h"
#include "BufferArchive.h"
#include "MemoryReader.h"


UVoxelDataAsset::UVoxelDataAsset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
};

TSharedRef<FVoxelAssetInstance> UVoxelDataAsset::GetAssetInternal(const FIntVector& Position) const
{
	return MakeShareable(new FVoxelDataAssetInstance(Size, Values, Materials, VoxelTypes, Position));
}

void UVoxelDataAsset::SetSize(const FIntVector& NewSize, bool bInitialize)
{
	Size.X = FMath::Max(0, NewSize.X);
	Size.Y = FMath::Max(0, NewSize.Y);
	Size.Z = FMath::Max(0, NewSize.Z);

	int Count = Size.X * Size.Y * Size.Z;

	Values.SetNumUninitialized(Count);
	Materials.SetNumUninitialized(Count);
	VoxelTypes.SetNumUninitialized(Count);

	if (bInitialize)
	{
		for(int i = 0; i < Count; i++) 
		{
			Values[i] = 1;
			Materials[i] = FVoxelMaterial();
			VoxelTypes[i] = FVoxelType::IgnoreAll().Value;
		}
	}
}

FIntVector UVoxelDataAsset::GetSize() const
{
	return Size;
}

void UVoxelDataAsset::SetValue(int X, int Y, int Z, float NewValue)
{
	check(0 <= X && X < Size.X);
	check(0 <= Y && Y < Size.Y);
	check(0 <= Z && Z < Size.Z);
	Values[X + Size.X * Y + Size.X * Size.Y * Z] = NewValue;
}

void UVoxelDataAsset::SetMaterial(int X, int Y, int Z, FVoxelMaterial NewMaterial)
{
	check(0 <= X && X < Size.X);
	check(0 <= Y && Y < Size.Y);
	check(0 <= Z && Z < Size.Z);
	Materials[X + Size.X * Y + Size.X * Size.Y * Z] = NewMaterial;
}

void UVoxelDataAsset::SetVoxelType(int X, int Y, int Z, FVoxelType VoxelType)
{
	check(0 <= X && X < Size.X);
	check(0 <= Y && Y < Size.Y);
	check(0 <= Z && Z < Size.Z);
	VoxelTypes[X + Size.X * Y + Size.X * Size.Y * Z] = VoxelType.Value;
}

float UVoxelDataAsset::GetValue(int X, int Y, int Z) const
{
	check(0 <= X && X < Size.X);
	check(0 <= Y && Y < Size.Y);
	check(0 <= Z && Z < Size.Z);
	return Values[X + Size.X * Y + Size.X * Size.Y * Z];
}

FVoxelMaterial UVoxelDataAsset::GetMaterial(int X, int Y, int Z) const
{
	check(0 <= X && X < Size.X);
	check(0 <= Y && Y < Size.Y);
	check(0 <= Z && Z < Size.Z);
	return Materials[X + Size.X * Y + Size.X * Size.Y * Z];
}

FVoxelType UVoxelDataAsset::GetVoxelType(int X, int Y, int Z) const
{
	check(0 <= X && X < Size.X);
	check(0 <= Y && Y < Size.Y);
	check(0 <= Z && Z < Size.Z);
	return FVoxelType(VoxelTypes[X + Size.X * Y + Size.X * Size.Y * Z]);
}

void UVoxelDataAsset::SetPrecomputedArrays(FIntVector InSize, TArray<float>& InValues, TArray<FVoxelMaterial>& InMaterials, TArray<uint8>& InVoxelTypes)
{
	int Count = InSize.X * InSize.Y * InSize.Z;
	check(InValues.Num() == Count);
	check(InMaterials.Num() == Count);
	check(InVoxelTypes.Num() == Count);

	Size = InSize;
	Values = InValues;
	Materials = InMaterials;
	VoxelTypes = InVoxelTypes;
}

void UVoxelDataAsset::Save()
{
	FBufferArchive Archive;

	Archive << Size;

	TArray<uint8> RLEValues;
	FVoxelUtilities::CompressRLE(Values, RLEValues);
	Archive << RLEValues;

	TArray<uint8> RLEMaterials;
	FVoxelUtilities::CompressRLE(Materials, RLEMaterials);
	Archive << RLEMaterials;

	TArray<uint8> RLETypes;
	FVoxelUtilities::CompressRLE(VoxelTypes, RLETypes);
	Archive << RLETypes;

	int32 UncompressedSize = Archive.Num();
	int32 CompressedSize = FCompression::CompressMemoryBound(CompressionFlags, UncompressedSize);

	CompressedData.SetNumUninitialized(CompressedSize + sizeof(UncompressedSize));

	FMemory::Memcpy(&CompressedData[0], &UncompressedSize, sizeof(UncompressedSize));
	verify(FCompression::CompressMemory(CompressionFlags, CompressedData.GetData() + sizeof(UncompressedSize), CompressedSize, Archive.GetData(), Archive.Num()));
	CompressedData.SetNum(CompressedSize + sizeof(UncompressedSize));
}

void UVoxelDataAsset::LoadInternal()
{
	TArray<uint8> Data;

	int32 UncompressedSize;
	FMemory::Memcpy(&UncompressedSize, &CompressedData[0], sizeof(UncompressedSize));
	Data.SetNum(UncompressedSize);
	verify(FCompression::UncompressMemory(CompressionFlags, Data.GetData(), UncompressedSize, CompressedData.GetData() + sizeof(UncompressedSize), CompressedData.Num() - sizeof(UncompressedSize)));

	FMemoryReader Reader(Data);

	Reader << Size;

	TArray<uint8> RLEValues;
	Reader << RLEValues;
	FVoxelUtilities::DecompressRLE(RLEValues, Values);

	TArray<uint8> RLEMaterials;
	Reader << RLEMaterials;
	FVoxelUtilities::DecompressRLE(RLEMaterials, Materials);

	TArray<uint8> RLETypes;
	Reader << RLETypes;
	FVoxelUtilities::DecompressRLE(RLETypes, VoxelTypes);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

FVoxelDataAssetInstance::FVoxelDataAssetInstance(const FIntVector& Size, const TArray<float>& Values, const TArray<FVoxelMaterial>& Materials, const TArray<uint8>& VoxelTypes, const FIntVector& Position)
	: Size(Size)
	, Values(Values)
	, Materials(Materials)
	, VoxelTypes(VoxelTypes)
	, FVoxelAssetInstance(Position)
{

}

void FVoxelDataAssetInstance::GetValuesAndMaterialsAndVoxelTypes(float InValues[], FVoxelMaterial InMaterials[], FVoxelType InVoxelTypes[], const FIntVector& Start, const FIntVector& StartIndex, const int Step, const FIntVector& InSize, const FIntVector& ArraySize) const
{
	for (int K = 0; K < InSize.Z; K++)
	{
		const int Z = Start.Z + K * Step - Position.Z;

		for (int J = 0; J < InSize.Y; J++)
		{
			const int Y = Start.Y + J * Step - Position.Y;

			for (int I = 0; I < InSize.X; I++)
			{
				const int X = Start.X + I * Step - Position.X;

				const int Index = (StartIndex.X + I) + ArraySize.X * (StartIndex.Y + J) + ArraySize.X * ArraySize.Y * (StartIndex.Z + K);

				const bool bValid = (0 <= X && X < Size.X) && (0 <= Y && Y < Size.Y) && (0 <= Z && Z < Size.Z);

				const int LocalArrayIndex = X + Size.X * Y + Size.X * Size.Y * Z;

				if (InValues)
				{
					InValues[Index] = bValid ? Values[LocalArrayIndex] : 1;
				}
				if (InMaterials)
				{
					InMaterials[Index] = bValid ? Materials[LocalArrayIndex] : FVoxelMaterial();
				}
				if (InVoxelTypes)
				{
					InVoxelTypes[Index] = bValid ? FVoxelType(VoxelTypes[LocalArrayIndex]) : FVoxelType::IgnoreAll();
				}
			}
		}
	}
}

FIntBox FVoxelDataAssetInstance::GetLocalBounds() const
{
	const FIntVector Bounds;

	FIntBox Box;
	Box.Min = FIntVector(0, 0, 0);
	Box.Max = FIntVector(Size.X - 1, Size.Y - 1, Size.Z - 1);
	return Box;
}
