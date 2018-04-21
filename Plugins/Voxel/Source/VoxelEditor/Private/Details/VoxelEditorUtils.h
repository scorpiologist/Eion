// Copyright 2018 Phyronnaz

#pragma once

#include "Runtime/Launch/Resources/Version.h"

#include "PropertyHandle.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"
#include "IDetailsView.h"
#include "PropertyHandle.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"
#include "IDetailsView.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "MessageDialog.h"
#include "PackageTools.h"

#include "VoxelImporter.h"
#include "AssetRegistryModule.h"
#include "SNotificationList.h"
#include "EditorViewportClient.h"
#include "NotificationManager.h"
#include "Editor.h"

class FVoxelEditorDetailsUtils
{
public:
	template<class T>
	static T* GetCurrentObjectFromDetails(IDetailLayoutBuilder& DetailLayout)
	{
#if ENGINE_MINOR_VERSION == 17
		const TArray< TWeakObjectPtr<UObject>>& SelectedObjects = DetailLayout.GetDetailsView().GetSelectedObjects();
#else
		const TArray<TWeakObjectPtr<AActor>>& SelectedObjects = DetailLayout.GetDetailsView()->GetSelectedActors();
#endif
		for (int32 ObjectIndex = 0; ObjectIndex < SelectedObjects.Num(); ++ObjectIndex)
		{
#if ENGINE_MINOR_VERSION == 17
			const TWeakObjectPtr<UObject>& CurrentObject = SelectedObjects[ObjectIndex];
#else
			const TWeakObjectPtr<AActor>& CurrentObject = SelectedObjects[ObjectIndex];
#endif
			if (CurrentObject.IsValid())
			{
				T* CurrentCaptureActor = Cast<T>(CurrentObject.Get());
				if (CurrentCaptureActor != NULL)
				{
					// Enable realtime
					((FEditorViewportClient*)(GEditor->GetActiveViewport()->GetClient()))->SetRealtime(true);
					return CurrentCaptureActor;
				}
			}
		}
		return nullptr;
	}

	template<class TAsset, class TAssetFactory, typename F>
	static void CreateAsset(AVoxelImporter* Importer, const F& CreateAsset)
	{
		if (Importer->FileName.IsEmpty())
		{
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Please enter a file name")));
		}
		else
		{
			FString NewPackageName = PackageTools::SanitizePackageName(TEXT("/Game/") + Importer->SavePath.Path + TEXT("/") + Importer->FileName);
			UPackage* Package = CreatePackage(NULL, *NewPackageName);

			FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

			// See if the asset already exists with the expected name, if it does, ask what to do
			TArray<FAssetData> OutAssetData;
			if (AssetRegistryModule.Get().GetAssetsByPackageName(*NewPackageName, OutAssetData) && OutAssetData.Num() > 0)
			{
				auto DialogReturn = FMessageDialog::Open(EAppMsgType::YesNoCancel, FText::Format(FText::FromString(TEXT("{0} already exists. Replace it?")), FText::FromString(NewPackageName)));
				switch (DialogReturn)
				{
				case EAppReturnType::Yes:
					break;
				default:
					return;
				}
			}

			// Create the asset
			TAssetFactory* AssetFactory = NewObject<TAssetFactory>();

			TAsset* Asset = (TAsset*)AssetFactory->FactoryCreateNew(TAsset::StaticClass(), Package, FName(*(Importer->FileName)), RF_Standalone | RF_Public, NULL, GWarn);
			check(Asset);

			bool bContinue = CreateAsset(Asset);
			if (bContinue)
			{
				// Notify the asset registry
				FAssetRegistryModule::AssetCreated(Asset);

				// Set the dirty flag so this package will get saved later
				Package->SetDirtyFlag(true);


				FString Text = NewPackageName + TEXT(" was successfully created");
				FNotificationInfo Info(FText::FromString(Text));
				Info.ExpireDuration = 10.f;
				FSlateNotificationManager::Get().AddNotification(Info);
			}
		}
	}
};

#define ADD_BUTTON_TO_CATEGORY(DetailLayout, CategoryName, NewRowFilterString, TextLeftToButton, ButtonText, ObjectPtr, FunctionPtr) \
	{ \
		DetailLayout.EditCategory((CategoryName)) \
			.AddCustomRow((NewRowFilterString)) \
			.NameContent() \
			[ \
				SNew(STextBlock) \
				.Font(IDetailLayoutBuilder::GetDetailFont()) \
				.Text((TextLeftToButton)) \
			] \
			.ValueContent() \
			.MaxDesiredWidth(125.f) \
			.MinDesiredWidth(125.f) \
			[ \
				SNew(SButton) \
				.ContentPadding(2) \
				.VAlign(VAlign_Center) \
				.HAlign(HAlign_Center) \
				.OnClicked((ObjectPtr), (FunctionPtr)) \
				[ \
					SNew(STextBlock) \
					.Font(IDetailLayoutBuilder::GetDetailFont()) \
					.Text((ButtonText)) \
				] \
			]; \
	}
