// Copyright 2018 Phyronnaz

#include "VoxelLandscapeImporterDetails.h"
#include "VoxelImporters/VoxelLandscapeImporter.h"

#include "VoxelAssets/VoxelLandscapeAsset.h"
#include "VoxelLandscapeAssetFactory.h"

#include "MessageDialog.h"

#include "LandscapeDataAccess.h"
#include "Landscape.h"
#include "LandscapeComponent.h"

#include "VoxelEditorUtils.h"

#define LOCTEXT_NAMESPACE "VoxelLandscapeImporterDetails"

TSharedRef<IDetailCustomization> FVoxelLandscapeImporterDetails::MakeInstance()
{
	return MakeShareable(new FVoxelLandscapeImporterDetails());
}

void FVoxelLandscapeImporterDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	LandscapeImporter = FVoxelEditorDetailsUtils::GetCurrentObjectFromDetails<AVoxelLandscapeImporter>(DetailLayout);

	ADD_BUTTON_TO_CATEGORY(DetailLayout,
		"Create VoxelLandscapeAsset from Landscape",
		LOCTEXT("Create", "Create"),
		LOCTEXT("CreateFromLandscape", "Create From Landscape"),
		LOCTEXT("Create", "Create"),
		this,
		&FVoxelLandscapeImporterDetails::OnCreateFromLandscape)
}

FReply FVoxelLandscapeImporterDetails::OnCreateFromLandscape()
{
	if (LandscapeImporter.IsValid())
	{
		if (!LandscapeImporter->Landscape)
		{
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Invalid Landscape")));
		}
		else
		{
			FVoxelEditorDetailsUtils::CreateAsset<UVoxelLandscapeAsset, UVoxelLandscapeAssetFactory>(LandscapeImporter.Get(), [&](UVoxelLandscapeAsset* LandscapeAsset)
			{
				int MipLevel = 0;
				int ComponentSize = 0;
				int Count = 0;

				// TODO: Non square landscapes?

				for (auto Component : LandscapeImporter->Landscape->GetLandscapeActor()->LandscapeComponents)
				{
					int Size = (Component->ComponentSizeQuads + 1) >> MipLevel;
					Count++;
					if (ComponentSize == 0)
					{
						ComponentSize = Size;
					}
					else
					{
						check(ComponentSize == Size);
					}

				}

				check(FMath::RoundToInt(FMath::Sqrt(Count)) * FMath::RoundToInt(FMath::Sqrt(Count)) == Count);
				const int TotalSize = FMath::RoundToInt(FMath::Sqrt(Count)) * ComponentSize;

				check(LandscapeAsset);
				LandscapeAsset->SetSize(TotalSize, TotalSize, true);

				for (auto Component : LandscapeImporter->Landscape->GetLandscapeActor()->LandscapeComponents)
				{
					FLandscapeComponentDataInterface DataInterface(Component, MipLevel);
					int Size = (Component->ComponentSizeQuads + 1) >> MipLevel;

					TArray<TArray<uint8>> Weightmaps;
					Weightmaps.SetNum(LandscapeImporter->LayerInfos.Num());

					for (int i = 0; i < Weightmaps.Num(); i++)
					{
						DataInterface.GetWeightmapTextureData(LandscapeImporter->LayerInfos[i].Layer, Weightmaps[i]);
					}

					int32 WeightmapSize = ((Component->SubsectionSizeQuads + 1) * Component->NumSubsections) >> MipLevel;

					for (int X = 0; X < Size; X++)
					{
						for (int Y = 0; Y < Size; Y++)
						{
							FVector Vertex = DataInterface.GetWorldVertex(X, Y);
							FVector LocalVertex = (Vertex - LandscapeImporter->Landscape->GetActorLocation()) / Component->GetComponentTransform().GetScale3D();
							LandscapeAsset->SetHeight(LocalVertex.X, LocalVertex.Y, Vertex.Z);

							uint8 MaxIndex = 0;
							uint8 MaxValue = 0;
							uint8 SecondMaxIndex = 0;
							uint8 SecondMaxValue = 0;

							for (int i = 0; i < Weightmaps.Num(); i++)
							{
								if (Weightmaps[i].Num())
								{
									uint8 Weight = Weightmaps[i][X + WeightmapSize * Y];
									if (Weight > MaxValue)
									{
										SecondMaxValue = MaxValue;
										SecondMaxIndex = MaxIndex;
										MaxValue = Weight;
										MaxIndex = LandscapeImporter->LayerInfos[i].Material;
									}
									else if (Weight > SecondMaxValue)
									{
										SecondMaxValue = Weight;
										SecondMaxIndex = LandscapeImporter->LayerInfos[i].Material;
									}
								}
							}

							LandscapeAsset->SetMaterial(LocalVertex.X, LocalVertex.Y, FVoxelMaterial(MaxIndex, SecondMaxIndex, FMath::Clamp<int>(((255 - MaxValue) + SecondMaxValue) / 2, 0, 255), 0));
						}
					}
				}

				LandscapeAsset->Save();

				return true;
			});
		}
	}
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
