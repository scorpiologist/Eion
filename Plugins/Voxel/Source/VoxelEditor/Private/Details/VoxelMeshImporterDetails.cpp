// Copyright 2018 Phyronnaz

#include "VoxelMeshImporterDetails.h"
#include "VoxelImporters/VoxelMeshImporter.h"

#include "VoxelDataAssetFactory.h"
#include "VoxelAssets/VoxelDataAsset.h"

#include "MessageDialog.h"

#include "VoxelEditorUtils.h"

#define LOCTEXT_NAMESPACE "VoxelMeshImporterDetails"

TSharedRef<IDetailCustomization> FVoxelMeshImporterDetails::MakeInstance()
{
	return MakeShareable(new FVoxelMeshImporterDetails());
}

void FVoxelMeshImporterDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	MeshImporter = FVoxelEditorDetailsUtils::GetCurrentObjectFromDetails<AVoxelMeshImporter>(DetailLayout);

	ADD_BUTTON_TO_CATEGORY(DetailLayout,
		"Create VoxelDataAsset from Mesh",
		LOCTEXT("Create", "Create"),
		LOCTEXT("CreateFromMesh", "Create From Mesh"),
		LOCTEXT("Create", "Create"),
		this,
		&FVoxelMeshImporterDetails::OnCreateFromMesh)
}

FReply FVoxelMeshImporterDetails::OnCreateFromMesh()
{
	if (MeshImporter.IsValid())
	{
		if (!MeshImporter->StaticMesh)
		{
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Invalid Mesh")));
		}
		else
		{
			FVoxelEditorDetailsUtils::CreateAsset<UVoxelDataAsset, UVoxelDataAssetFactory>(MeshImporter.Get(), [&](UVoxelDataAsset* DataAsset)
			{
				MeshImporter->ImportToAsset(*DataAsset);
				DataAsset->Save();
				return true;
			});
		}
	}
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
