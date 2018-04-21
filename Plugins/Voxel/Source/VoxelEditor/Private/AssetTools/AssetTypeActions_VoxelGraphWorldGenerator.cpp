// Copyright 2018 Phyronnaz

#include "AssetTypeActions_VoxelGraphWorldGenerator.h"
#include "Sound/SoundAttenuation.h"
#include "Misc/PackageName.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Factories/SoundAttenuationFactory.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "AudioEditorModule.h"
#include "VoxelGraphWorldGenerator.h"
#include "VoxelEditorModule.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

UClass* FAssetTypeActions_VoxelGraphWorldGenerator::GetSupportedClass() const
{
	return UVoxelGraphGenerator::StaticClass();
}

void FAssetTypeActions_VoxelGraphWorldGenerator::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto VoxelGraphWorldGenerator = Cast<UVoxelGraphGenerator>(*ObjIt);
		if (VoxelGraphWorldGenerator != NULL)
		{
			IVoxelEditorModule* VoxelEditorModule = &FModuleManager::LoadModuleChecked<IVoxelEditorModule>("VoxelEditor");
			VoxelEditorModule->CreateVoxelEditor(Mode, EditWithinLevelEditor, VoxelGraphWorldGenerator);
		}
	}
}
uint32 FAssetTypeActions_VoxelGraphWorldGenerator::GetCategories()
{
	return MyAssetCategory;
}
#undef LOCTEXT_NAMESPACE
