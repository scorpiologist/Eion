// Copyright 2018 Phyronnaz

#include "VoxelHeightmapImporterDetails.h"
#include "VoxelImporters/VoxelHeightmapImporter.h"

#include "VoxelAssets/VoxelLandscapeAsset.h"
#include "VoxelLandscapeAssetFactory.h"

#include "LandscapeFileFormatInterface.h"
#include "LandscapeEditorModule.h"

#include "VoxelEditorUtils.h"

#define LOCTEXT_NAMESPACE "VoxelHeightmapImporterDetails"

TSharedRef<IDetailCustomization> FVoxelHeightmapImporterDetails::MakeInstance()
{
	return MakeShareable(new FVoxelHeightmapImporterDetails());
}

void FVoxelHeightmapImporterDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	Importer = FVoxelEditorDetailsUtils::GetCurrentObjectFromDetails<AVoxelHeightmapImporter>(DetailLayout);

	ADD_BUTTON_TO_CATEGORY(DetailLayout,
		"Create VoxelLandscapeAsset from Heightmap",
		LOCTEXT("Create", "Create"),
		LOCTEXT("CreateFromHeightmap", "Create From Heightmap"),
		LOCTEXT("Create", "Create"),
		this,
		&FVoxelHeightmapImporterDetails::OnCreateFromHeightmap)
}

bool GetHeightmap(const FString& HeightmapFilename, int& OutWidth, int& OutHeight, FLandscapeHeightmapImportData& OutHeightmapImportData)
{
	ILandscapeEditorModule& LandscapeEditorModule = FModuleManager::GetModuleChecked<ILandscapeEditorModule>("LandscapeEditor");
	const ILandscapeHeightmapFileFormat* HeightmapFormat = LandscapeEditorModule.GetHeightmapFormatByExtension(*FPaths::GetExtension(HeightmapFilename, true));

	if (HeightmapFormat)
	{
		FLandscapeHeightmapInfo Info = HeightmapFormat->Validate(*HeightmapFilename);
		bool bContinue;
		if (Info.ResultCode == ELandscapeImportResult::Success)
		{
			bContinue = true;
		}
		else if (Info.ResultCode == ELandscapeImportResult::Warning)
		{
			auto DialogReturn = FMessageDialog::Open(EAppMsgType::YesNo, FText::Format(FText::FromString(TEXT("Warning: {0} \nContinue?")), Info.ErrorMessage));
			switch (DialogReturn)
			{
			case EAppReturnType::No:
				bContinue = false;
				break;
			case EAppReturnType::Yes:
				bContinue = true;
				break;
			default:
				check(false);
				bContinue = false;
				break;
			}
		}
		else if (Info.ResultCode == ELandscapeImportResult::Error)
		{
			bContinue = false;
			FMessageDialog::Open(EAppMsgType::Ok, FText::Format(FText::FromString(TEXT("Error: {0}")), Info.ErrorMessage));
		}
		else
		{
			bContinue = false;
			check(false);
		}
		if (!bContinue)
		{
			return false;
		}
		else
		{
			OutHeightmapImportData = HeightmapFormat->Import(*HeightmapFilename, Info.PossibleResolutions[0]);

			if (OutHeightmapImportData.ResultCode == ELandscapeImportResult::Success)
			{
				bContinue = true;
			}
			else if (OutHeightmapImportData.ResultCode == ELandscapeImportResult::Warning)
			{
				auto DialogReturn = FMessageDialog::Open(EAppMsgType::YesNo, FText::Format(FText::FromString(TEXT("Warning: {0} \nContinue?")), Info.ErrorMessage));
				switch (DialogReturn)
				{
				case EAppReturnType::No:
					bContinue = false;
					break;
				case EAppReturnType::Yes:
					bContinue = true;
					break;
				default:
					check(false);
					bContinue = false;
					break;
				}
			}
			else if (OutHeightmapImportData.ResultCode == ELandscapeImportResult::Error)
			{
				bContinue = false;
				FMessageDialog::Open(EAppMsgType::Ok, FText::Format(FText::FromString(TEXT("Error: {0}")), Info.ErrorMessage));
			}
			else
			{
				bContinue = false;
				check(false);
			}
			if (!bContinue)
			{
				return false;
			}
			else
			{
				OutWidth = Info.PossibleResolutions[0].Width;
				OutHeight = Info.PossibleResolutions[0].Height;
			}
		}
	}
	return true;
}

FReply FVoxelHeightmapImporterDetails::OnCreateFromHeightmap()
{
	if (Importer.IsValid())
	{
		FVoxelEditorDetailsUtils::CreateAsset<UVoxelLandscapeAsset, UVoxelLandscapeAssetFactory>(Importer.Get(), [&](UVoxelLandscapeAsset* LandscapeAsset)
		{
			int Width, Height;
			FLandscapeHeightmapImportData HeightmapImportData;
			bool bContinue = GetHeightmap(Importer->Heightmap.FilePath, Width, Height, HeightmapImportData);

			if (bContinue)
			{
				TArray<FLandscapeHeightmapImportData> WeightmapsData;
				for (auto Weightmap : Importer->Weightmaps)
				{
					int TmpWidth = -1;
					int TmpHeight = -1;
					WeightmapsData.SetNum(WeightmapsData.Num() + 1);
					bContinue = bContinue && GetHeightmap(Weightmap.File.FilePath, TmpWidth, TmpHeight, WeightmapsData[WeightmapsData.Num() - 1]);

					if (TmpWidth != Width || TmpHeight != Height)
					{
						FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Weightmaps resolutions is not the same as Heightmap")));
						bContinue = false;
					}

					if (!bContinue)
					{
						break;
					}
				}

				if (bContinue)
				{
					TArray<float> Heights;
					TArray<FVoxelMaterial> Materials;
					Heights.SetNumUninitialized(Width * Height);
					Materials.SetNumUninitialized(Width * Height);
					float MaxHeight = -1e10;
					float MinHeight = 1e10;

					for (int i = 0; i < Width * Height; i++)
					{
						Heights[i] = HeightmapImportData.Data[i];
						MaxHeight = FMath::Max(Heights[i], MaxHeight);
						MinHeight = FMath::Min(Heights[i], MinHeight);

						uint8 FirstMaxValue = 0;
						uint8 FirstMaxIndex = 0;
						uint8 SecondMaxValue = 0;
						uint8 SecondMaxIndex = 0;

						for (int k = 0; k < WeightmapsData.Num(); k++)
						{
							float WeightFloat = WeightmapsData[k].Data[i];

							float Min = Importer->Weightmaps[k].MinValue;
							float Max = Importer->Weightmaps[k].MaxValue;

							uint8 Weight = FMath::Clamp<int>(255.f * (WeightFloat - Min) / (Max - Min), 0, 255);
							if (Weight >= FirstMaxValue)
							{
								SecondMaxValue = FirstMaxValue;
								SecondMaxIndex = FirstMaxIndex;

								FirstMaxValue = Weight;
								FirstMaxIndex = Importer->Weightmaps[k].Material;
							}
							else if (Weight >= SecondMaxValue)
							{
								SecondMaxValue = Weight;
								SecondMaxIndex = Importer->Weightmaps[k].Material;
							}
						}
						check(FirstMaxValue >= SecondMaxValue);
						Materials[i] = FVoxelMaterial(FirstMaxIndex, SecondMaxIndex, FMath::Clamp<int>(((255 - FirstMaxValue) + SecondMaxValue) / 2, 0, 255), 0);

					}
					LandscapeAsset->SetPrecomputedValues(Heights, Materials, Width, Height, MaxHeight, MinHeight);
					LandscapeAsset->Save();
				}
			}

			return bContinue;
		});
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
