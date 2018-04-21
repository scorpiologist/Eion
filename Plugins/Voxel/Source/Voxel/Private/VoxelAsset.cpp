// Copyright 2018 Phyronnaz

#include "VoxelAsset.h"

UVoxelAsset::UVoxelAsset(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
, bIsLoaded(false)
{

}

TSharedRef<FVoxelWorldGeneratorInstance> UVoxelAsset::GetWorldGenerator()
{
	return GetAsset(FIntVector::ZeroValue);
}

TSharedRef<class FVoxelAssetInstance> UVoxelAsset::GetAsset(const FIntVector& Position)
{
	Load();
	return GetAssetInternal(Position);
}

void UVoxelAsset::Load()
{
	if (!bIsLoaded)
	{
		LoadInternal();
		bIsLoaded = true;
	}
}

TSharedRef<FVoxelAssetInstance> UVoxelAsset::GetAssetInternal(const FIntVector& Position) const
{
	unimplemented(); return MakeShareable(new FVoxelAssetInstance(Position));
}

void UVoxelAsset::LoadInternal()
{
	bIsLoaded = true;
}

///////////////////////////////////////////////////////////////////////////////

bool FVoxelAssetInstance::IsEmpty(const FIntVector& Start, const int Step, const FIntVector& Size) const
{
	return !FIntBox(Start, Start + Size * Step).Intersect(GetWorldBounds()) || IsAssetEmpty(Start, Step, Size);
}

FIntBox FVoxelAssetInstance::GetWorldBounds() const
{
	return GetLocalBounds().TranslateBy(Position);
}

FIntBox FVoxelAssetInstance::GetLocalBounds() const
{
	unimplemented(); return FIntBox();
}

bool FVoxelAssetInstance::IsAssetEmpty(const FIntVector& Start, const int Step, const FIntVector& Size) const
{
	return false;
}

