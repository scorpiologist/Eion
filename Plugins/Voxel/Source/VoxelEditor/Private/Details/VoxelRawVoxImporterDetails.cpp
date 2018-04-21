// Copyright 2018 Phyronnaz

#include "VoxelRawVoxImporterDetails.h"
#include "VoxelImporters/VoxelRawVoxImporter.h"

#include "VoxelDataAssetFactory.h"
#include "VoxelAssets/VoxelDataAsset.h"

#include "MessageDialog.h"
#include "FileHelper.h"

#include "VoxelEditorUtils.h"

#define LOCTEXT_NAMESPACE "VoxelRawVoxImporterDetails"

TSharedRef<IDetailCustomization> FVoxelRawVoxImporterDetails::MakeInstance()
{
	return MakeShareable(new FVoxelRawVoxImporterDetails());
}

void FVoxelRawVoxImporterDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	Importer = FVoxelEditorDetailsUtils::GetCurrentObjectFromDetails<AVoxelRawVoxImporter>(DetailLayout);

	ADD_BUTTON_TO_CATEGORY(DetailLayout,
		"Create VoxelDataAsset from RawVox (3D Coat)",
		LOCTEXT("Create", "Create"),
		LOCTEXT("CreateFromRawVox", "Create From RawVox"),
		LOCTEXT("Create", "Create"),
		this,
		&FVoxelRawVoxImporterDetails::OnCreate)
}

bool ImportToAsset(const FString& File, UVoxelDataAsset& Asset)
{
	TArray<uint8> Result;
	bool bSuccess = FFileHelper::LoadFileToArray(Result, *File);
	if (!bSuccess)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Error when opening the file")));
		return false;
	}

	int Position = 0;
	TCHAR Start[4];
	for (int i = 0; i < 4; i++)
	{
		Start[i] = Result[Position];
		Position++;
	}
	bSuccess = Start[0] == 'X' && Start[1] == 'O' && Start[2] == 'V' && Start[3] == 'R';
	if (!bSuccess)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("File is corrupted")));
		return false;
	}

	const int SizeX = Result[Position] + 256 * Result[Position + 1] + 256 * 256 * Result[Position + 2] + 256 * 256 * 256 * Result[Position + 3];
	Position += 4;
	const int SizeY = Result[Position] + 256 * Result[Position + 1] + 256 * 256 * Result[Position + 2] + 256 * 256 * 256 * Result[Position + 3];
	Position += 4;
	const int SizeZ = Result[Position] + 256 * Result[Position + 1] + 256 * 256 * Result[Position + 2] + 256 * 256 * 256 * Result[Position + 3];
	Position += 4;

	const int BitsPerVoxel = Result[Position];
	Position += 4;

	union
	{
		float f;
		uint8 b[4];
	} U;

	Asset.SetSize(FIntVector(SizeX, SizeZ, SizeY), false);

	for (int Z = 0; Z < SizeZ; Z++)
	{
		for (int Y = 0; Y < SizeY; Y++)
		{
			for (int X = 0; X < SizeX; X++)
			{
				float Val;
				if (BitsPerVoxel == 8)
				{
					Val = (Result[Position] - 128) / 128.f;
					Position++;
				}
				else if (BitsPerVoxel == 16)
				{
					Val = (Result[Position] + 256 * Result[Position + 1] - 32768) / 32768.f;
					Position += 2;
				}
				else if (BitsPerVoxel == 32)
				{
					U.b[0] = Result[Position];
					Position++;
					U.b[1] = Result[Position];
					Position++;
					U.b[2] = Result[Position];
					Position++;
					U.b[3] = Result[Position];
					Position++;

					Val = -(U.f - 0.5) * 2;
				}
				else
				{
					FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("File is corrupted")));
					return false;
				}
				Asset.SetValue(X, Z, Y, Val);
				Asset.SetMaterial(X, Z, Y, FVoxelMaterial(0, 0, 0, 0));
				Asset.SetVoxelType(X, Z, Y, FVoxelType::UseAll());
			}
		}
	}

	return true;
}

FReply FVoxelRawVoxImporterDetails::OnCreate()
{
	if (Importer.IsValid())
	{
		FVoxelEditorDetailsUtils::CreateAsset<UVoxelDataAsset, UVoxelDataAssetFactory>(Importer.Get(), [&](UVoxelDataAsset* DataAsset)
		{
			bool bSuccess = ImportToAsset(Importer->File.FilePath, *DataAsset);
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
