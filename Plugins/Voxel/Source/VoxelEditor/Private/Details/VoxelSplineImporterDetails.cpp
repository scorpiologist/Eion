// Copyright 2018 Phyronnaz

#include "VoxelSplineImporterDetails.h"
#include "VoxelImporters/VoxelSplineImporter.h"

#include "VoxelDataAssetFactory.h"
#include "VoxelAssets/VoxelDataAsset.h"

#include "VoxelEditorUtils.h"

#define LOCTEXT_NAMESPACE "VoxelSplineImporterDetails"

TSharedRef<IDetailCustomization> FVoxelSplineImporterDetails::MakeInstance()
{
	return MakeShareable(new FVoxelSplineImporterDetails());
}

void FVoxelSplineImporterDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	SplineImporter = FVoxelEditorDetailsUtils::GetCurrentObjectFromDetails<AVoxelSplineImporter>(DetailLayout);

	ADD_BUTTON_TO_CATEGORY(DetailLayout,
		"Create VoxelDataAsset from Splines",
		LOCTEXT("Create", "Create"),
		LOCTEXT("CreateFromSplines", "Create From Splines"),
		LOCTEXT("Create", "Create"),
		this,
		&FVoxelSplineImporterDetails::OnCreateFromSplines)
}

FReply FVoxelSplineImporterDetails::OnCreateFromSplines()
{
	if (SplineImporter.IsValid())
	{
		if (SplineImporter->Splines.Num() == 0)
		{
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("No splines")));
		}
		else
		{
			FVoxelEditorDetailsUtils::CreateAsset<UVoxelDataAsset, UVoxelDataAssetFactory>(SplineImporter.Get(), [&](UVoxelDataAsset* DataAsset)
			{
				SplineImporter->ImportToAsset(*DataAsset);
				DataAsset->Save();
				return true;
			});
		}
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
