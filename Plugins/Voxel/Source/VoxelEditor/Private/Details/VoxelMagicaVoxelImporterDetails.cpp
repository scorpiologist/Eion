// Copyright 2018 Phyronnaz

#include "VoxelMagicaVoxelImporterDetails.h"
#include "VoxelImporters/VoxelMagicaVoxelImporter.h"

#include "VoxelDataAssetFactory.h"
#include "VoxelAssets/VoxelDataAsset.h"

#include "MessageDialog.h"
#include "FileHelper.h"

#include "VoxelEditorUtils.h"

#define LOCTEXT_NAMESPACE "VoxelMagicaVoxelImporterDetails"

TSharedRef<IDetailCustomization> FVoxelMagicaVoxelImporterDetails::MakeInstance()
{
	return MakeShareable(new FVoxelMagicaVoxelImporterDetails());
}

void FVoxelMagicaVoxelImporterDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	Importer = FVoxelEditorDetailsUtils::GetCurrentObjectFromDetails<AVoxelMagicaVoxelImporter>(DetailLayout);

	ADD_BUTTON_TO_CATEGORY(DetailLayout,
		"Create VoxelDataAsset from MagicalVoxel asset",
		LOCTEXT("Create", "Create"),
		LOCTEXT("CreateFromMagicalVoxel", "Create From MagicaVoxel"),
		LOCTEXT("Create", "Create"),
		this,
		&FVoxelMagicaVoxelImporterDetails::OnCreate)
}

FORCEINLINE int ReadInt(TArray<uint8>& Bytes, int& Position)
{
	int Result = Bytes[Position] + 256 * Bytes[Position + 1] + 256 * 256 * Bytes[Position + 2] + 256 * 256 * 256 * Bytes[Position + 3];
	Position += 4;
	return Result;
}

FORCEINLINE bool ReadString(TArray<uint8>& Bytes, int& Position, TCHAR* Chars)
{
	TCHAR Start[4];
	for (int i = 0; i < 4; i++)
	{
		Start[i] = Bytes[Position];
		Position++;
	}
	return Start[0] == Chars[0] && Start[1] == Chars[1] && Start[2] == Chars[2] && Start[3] == Chars[3];
}

FORCEINLINE uint8 ReadByte(TArray<uint8>& Bytes, int& Position)
{
	uint8 Result = Bytes[Position];
	Position++;
	return Result;
}

bool MagicaImportToAsset(const FString& File, UVoxelDataAsset& Asset)
{
	TArray<uint8> Bytes;
	bool bSuccess = FFileHelper::LoadFileToArray(Bytes, *File);
	if (!bSuccess)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Error when opening the file")));
		return false;
	}

	int Position = 0;

	bSuccess = ReadString(Bytes, Position, TEXT("VOX "));
	if (!bSuccess)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("File is corrupted")));
		return false;
	}

	int Version = ReadInt(Bytes, Position);

	bSuccess = ReadString(Bytes, Position, TEXT("MAIN"));
	if (!bSuccess)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("File is corrupted")));
		return false;
	}

	Position += 8; // Unknown

	int PackCount;
	bSuccess = ReadString(Bytes, Position, TEXT("PACK"));
	Position += 8; // Unknown
	if (bSuccess)
	{
		PackCount = ReadInt(Bytes, Position);
	}
	else
	{
		Position -= 4 + 8;
		PackCount = 1;
	}

	for (int i = 0; i < 1 /*PackCount*/; i++) // TODO: PackCount != 1
	{
		bSuccess = ReadString(Bytes, Position, TEXT("SIZE"));
		if (!bSuccess)
		{
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("File is corrupted")));
			return false;
		}
		Position += 8; // Unknown

		const int SizeX = ReadInt(Bytes, Position);
		const int SizeY = ReadInt(Bytes, Position);
		const int SizeZ = ReadInt(Bytes, Position);

		Asset.SetSize(FIntVector(SizeY, SizeX, SizeZ), false);

		TArray<bool> Blocks;
		TArray<uint8> Colors;
		Blocks.SetNum(SizeX * SizeY * SizeZ);
		Colors.SetNum(SizeX * SizeY * SizeZ);


		bSuccess = ReadString(Bytes, Position, TEXT("XYZI"));
		if (!bSuccess)
		{
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("File is corrupted")));
			return false;
		}
		Position += 8; // Unknown

		const int N = ReadInt(Bytes, Position);

		for (int k = 0; k < N; k++)
		{
			int X = ReadByte(Bytes, Position);
			int Y = ReadByte(Bytes, Position);
			int Z = ReadByte(Bytes, Position);
			int Color = ReadByte(Bytes, Position);

			Blocks[X + SizeX * Y + SizeX * SizeY * Z] = true;
			Colors[X + SizeX * Y + SizeX * SizeY * Z] = Color;
		}

		for (int X = 0; X < SizeX; X++)
		{
			for (int Y = 0; Y < SizeY; Y++)
			{
				for (int Z = 0; Z < SizeZ; Z++)
				{
					uint8 Color = Colors[X + SizeX * Y + SizeX * SizeY * Z];
					int Neighbors = 0;
					Neighbors += Blocks[X + SizeX * Y + SizeX * SizeY * Z];
					if (X != SizeX - 1)	Neighbors += Blocks[X + 1 + SizeX * Y + SizeX * SizeY * Z];
					if (Y != SizeY - 1) Neighbors += Blocks[X + SizeX * (Y + 1) + SizeX * SizeY * Z];
					if (Y != SizeY - 1) if (X != SizeX - 1)	Neighbors += Blocks[X + 1 + SizeX * (Y + 1) + SizeX * SizeY * Z];
					if (Z != SizeZ - 1)	Neighbors += Blocks[X + SizeX * Y + SizeX * SizeY * (Z + 1)];
					if (Z != SizeZ - 1)	if (X != SizeX - 1)	Neighbors += Blocks[X + 1 + SizeX * Y + SizeX * SizeY * (Z + 1)];
					if (Z != SizeZ - 1)	if (Y != SizeY - 1) Neighbors += Blocks[X + SizeX * (Y + 1) + SizeX * SizeY * (Z + 1)];
					if (Z != SizeZ - 1) if (Y != SizeY - 1) if (X != SizeX - 1)	Neighbors += Blocks[X + 1 + SizeX * (Y + 1) + SizeX * SizeY * (Z + 1)];

					if (Neighbors == 8)
					{
						Asset.SetValue(Y, X, Z, -1);
					}
					else if (Neighbors > 0)
					{
						Asset.SetValue(Y, X, Z, 0);
					}
					else
					{
						Asset.SetValue(Y, X, Z, 1);
					}
					Asset.SetMaterial(Y, X, Z, FVoxelMaterial(Color, Color, 0, 0));
					Asset.SetVoxelType(Y, X, Z, FVoxelType::UseAll());
				}
			}
		}
	}

	return true;
}

FReply FVoxelMagicaVoxelImporterDetails::OnCreate()
{
	if (Importer.IsValid())
	{
		FVoxelEditorDetailsUtils::CreateAsset<UVoxelDataAsset, UVoxelDataAssetFactory>(Importer.Get(), [&](UVoxelDataAsset* DataAsset)
		{
			bool bSuccess = MagicaImportToAsset(Importer->File.FilePath, *DataAsset);
			if (bSuccess)
			{
				DataAsset->Save();
			}
			return bSuccess;
		});
	}
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
