// Copyright 2018 Phyronnaz

#include "VoxelWorldSectionImporterDetails.h"
#include "VoxelImporters/VoxelWorldSectionImporter.h"

#include "DetailLayoutBuilder.h"

#include "VoxelEditorUtils.h"
#include "VoxelDataAssetFactory.h"

#define LOCTEXT_NAMESPACE "VoxelWorldSectionImporterDetails"

TSharedRef<IDetailCustomization> FVoxelWorldSectionImporterDetails::MakeInstance()
{
	return MakeShareable(new FVoxelWorldSectionImporterDetails());
}

void FVoxelWorldSectionImporterDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	Importer = FVoxelEditorDetailsUtils::GetCurrentObjectFromDetails<AVoxelWorldSectionImporter>(DetailLayout);

	ADD_BUTTON_TO_CATEGORY(DetailLayout,
		"Create VoxelDataAsset from World section",
		LOCTEXT("Import", "Import"),
		LOCTEXT("ImportFromWorld", "Import From World"),
		LOCTEXT("Create", "Create"),
		this,
		&FVoxelWorldSectionImporterDetails::OnImport)


	ADD_BUTTON_TO_CATEGORY(DetailLayout,
		"Import configuration",
		LOCTEXT("Import", "Import"),
		LOCTEXT("CopyActorsPositionsToCorners", "Copy Actors Positions To Corners"),
		LOCTEXT("Copy", "Copy"),
		this,
		&FVoxelWorldSectionImporterDetails::OnImportFromActors)
}

FReply FVoxelWorldSectionImporterDetails::OnImport()
{
	if (Importer.IsValid())
	{
		if (!Importer->World)
		{
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Invalid World")));
		}
		else
		{
			FVoxelEditorDetailsUtils::CreateAsset<UVoxelDataAsset, UVoxelDataAssetFactory>(Importer.Get(), [&](UVoxelDataAsset* Asset)
			{
				Importer->ImportToAsset(*Asset);
				Asset->Save();
				return true;
			});
		}
	}
	return FReply::Handled();
}

FReply FVoxelWorldSectionImporterDetails::OnImportFromActors()
{
	if (Importer.IsValid())
	{
		if (!Importer->World)
		{
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Invalid World")));
		}
		else
		{
			Importer->SetCornersFromActors();
		}
	}
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
