// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "IVoxelEditor.h"

/** Sound class editor module interface */
class IVoxelEditorModule : public IModuleInterface
{
public:
	/** Creates a new material editor, either for a material or a material function. */
	virtual TSharedRef<IVoxelEditor> CreateVoxelEditor(const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, UVoxelGraphGenerator* SoundCue) = 0;
};
