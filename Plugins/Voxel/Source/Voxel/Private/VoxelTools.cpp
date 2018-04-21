// Copyright 2018 Phyronnaz

#include "VoxelTools.h"
#include "VoxelPrivate.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/HUD.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "VoxelWorldGenerators/EmptyWorldGenerator.h"
#include "VoxelData.h"
#include "VoxelAssets/VoxelDataAsset.h"
#include "FastNoise.h"
#include "VoxelWorld.h"
#include "VoxelUtilities.h"
#include "VoxelActor.h"
#include "Engine/World.h"
#include "VoxelThread.h"

DECLARE_CYCLE_STAT(TEXT("UVoxelTools::SimulatePhysicsOnFloatingVoxelActors"), STAT_UVoxelTools_SimulatePhysicsOnFloatingVoxelActors, STATGROUP_Voxel);

DECLARE_CYCLE_STAT(TEXT("UVoxelTools::*.BeginSet"), STAT_UVoxelTools_BeginSet, STATGROUP_Voxel);

DECLARE_CYCLE_STAT(TEXT("UVoxelTools::SetValueSphere"), STAT_UVoxelTools_SetValueSphere, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("UVoxelTools::SetValueBox"), STAT_UVoxelTools_SetValueBox, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("UVoxelTools::SetMaterialBox"), STAT_UVoxelTools_SetMaterialBox, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("UVoxelTools::SetMaterialSphere"), STAT_UVoxelTools_SetMaterialSphere, STATGROUP_Voxel);

DECLARE_CYCLE_STAT(TEXT("UVoxelTools::FindModifiedPositionsForRaycasts"), STAT_UVoxelTools_FindModifiedPositionsForRaycasts, STATGROUP_Voxel);

DECLARE_CYCLE_STAT(TEXT("UVoxelTools::SetValueProjection"), STAT_UVoxelTools_SetValueProjection, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("UVoxelTools::SetMaterialProjection"), STAT_UVoxelTools_SetMaterialProjection, STATGROUP_Voxel);

DECLARE_CYCLE_STAT(TEXT("UVoxelTools::Flatten"), STAT_UVoxelTools_Flatten, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("UVoxelTools::ImportAsset"), STAT_UVoxelTools_ImportAsset, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("UVoxelTools::RemoveFloatingBlocks"), STAT_UVoxelTools_RemoveFloatingBlocks, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("UVoxelTools::TransformVoxelAsset"), STAT_UVoxelTools_TransformVoxelAsset, STATGROUP_Voxel);

void UVoxelTools::SimulatePhysicsOnFloatingVoxelActors(AVoxelWorld* World, const FVector& Position, int BoxRadius)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_SimulatePhysicsOnFloatingVoxelActors);

	if (!World)
	{
		UE_LOG(LogVoxel, Error, TEXT("SimulatePhysicsOnFloatingVoxelActors: World is NULL"));
		return;
	}
	const FIntVector LocalPosition = World->GlobalToLocal(Position);

	TArray<AVoxelActor*> Actors;

	const FIntVector R = FIntVector(BoxRadius, BoxRadius, BoxRadius);
	World->GetActorsInBox(FIntBox(LocalPosition - R, LocalPosition + R), Actors);

	for (AVoxelActor* Actor : Actors)
	{
		if (!Actor->IsOverlappingVoxelWorld(World))
		{
			Actor->StartSimulatingPhysics();
			World->RemoveActorFromOctree(Actor);
		}
	}
}

void UVoxelTools::SetValueSphere(AVoxelWorld* World, const FVector Position, const float WorldRadius, const bool bAdd)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_SetValueSphere);

	if (!World)
	{
		UE_LOG(LogVoxel, Error, TEXT("SetValueSphere: World is NULL"));
		return;
	}
	const float Radius = WorldRadius / World->GetVoxelSize();

	// Position in voxel space
	FIntVector LocalPosition = World->GlobalToLocal(Position);
	int IntRadius = FMath::CeilToInt(Radius) + 2;
	
	FIntVector R(IntRadius + 1, IntRadius + 1, IntRadius + 1);
	const FIntBox Bounds(LocalPosition - R, LocalPosition + R);

	FValueOctree* LastOctree = nullptr;
	FVoxelData* Data = World->GetData();

	{
		TArray<uint64> Octrees;
		{
			SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_BeginSet);
			Octrees = Data->BeginSet(Bounds);
		}
		for (int X = -IntRadius; X <= IntRadius; X++)
		{
			for (int Y = -IntRadius; Y <= IntRadius; Y++)
			{
				for (int Z = -IntRadius; Z <= IntRadius; Z++)
				{
					const FIntVector CurrentPosition = LocalPosition + FIntVector(X, Y, Z);
					const float Distance = FVector(X, Y, Z).Size();

					if (Distance <= Radius + 2)
					{
						// We want (Radius - Distance) != 0
						const float Noise = (Radius - Distance == 0) ? 0.0001f : 0;
						float Value = FMath::Clamp(Radius - Distance + Noise, -2.f, 2.f) / 2;

						Value *= (bAdd ? -1 : 1);

						float OldValue = Data->GetValue(CurrentPosition);

						bool bValid;
						if ((Value <= 0 && bAdd) || (Value > 0 && !bAdd))
						{
							bValid = true;
						}
						else
						{
							bValid = FVoxelUtilities::HaveSameSign(OldValue, Value);
						}
						if (bValid)
						{
							if (LIKELY(Data->IsInWorld(CurrentPosition)))
							{
								Data->SetValue(CurrentPosition, Value, LastOctree);
							}
						}
					}
				}
			}
		}
		Data->EndSet(Octrees);
	}
	World->UpdateChunksOverlappingBox(Bounds);
}

void UVoxelTools::SetValueBox(AVoxelWorld* World, const FVector Position, const FIntVector Size, const bool bAdd)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_SetValueBox);

	if (World == nullptr)
	{
		UE_LOG(LogVoxel, Error, TEXT("SetValueBox: World is NULL"));
		return;
	}
	check(World);

	FIntVector LocalPosition = World->GlobalToLocal(Position);
	
	const FIntBox Bounds(LocalPosition, LocalPosition + Size);

	FValueOctree* LastOctree = nullptr;
	FVoxelData* Data = World->GetData();

	{
		TArray<uint64> Octrees;
		{
			SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_BeginSet);
			Octrees = Data->BeginSet(Bounds);
		}

		for (int X = 0; X < Size.X; X++)
		{
			for (int Y = 0; Y < Size.Y; Y++)
			{
				for (int Z = 0; Z < Size.Z; Z++)
				{
					FIntVector P = LocalPosition + FIntVector(X, Y, Z);

					if (LIKELY(Data->IsInWorld(P)))
					{
						float Value;
						if (X == 0 || X == Size.X - 1 || Y == 0 || Y == Size.Y - 1 || Z == 0 || Z == Size.Z - 1)
						{
							Value = 0;;
						}
						else
						{
							Value = (bAdd ? -1 : 1);
						}

						if ((Value <= 0 && bAdd) || (Value > 0 && !bAdd) || FVoxelUtilities::HaveSameSign(Data->GetValue(P), Value))
						{
							Data->SetValue(P, Value, LastOctree);
						}
					}
				}
			}
		}

		Data->EndSet(Octrees);
	}
	World->UpdateChunksOverlappingBox(Bounds);
}

void UVoxelTools::SetMaterialBox(AVoxelWorld* World, FVector Position, FIntVector Size, uint8 MaterialIndex, EVoxelLayer Layer)
{
	
	SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_SetMaterialBox);

	if (World == nullptr)
	{
		UE_LOG(LogVoxel, Error, TEXT("SetValueBox: World is NULL"));
		return;
	}
	check(World);

	FIntVector LocalPosition = World->GlobalToLocal(Position);
	
	const FIntBox Bounds(LocalPosition, LocalPosition + Size);

	FValueOctree* LastOctree = nullptr;
	FVoxelData* Data = World->GetData();

	{
		TArray<uint64> Octrees;
		{
			SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_BeginSet);
			Octrees = Data->BeginSet(Bounds);
		}

		for (int X = 0; X < Size.X; X++)
		{
			for (int Y = 0; Y < Size.Y; Y++)
			{
				for (int Z = 0; Z < Size.Z; Z++)
				{
					FIntVector P = LocalPosition + FIntVector(X, Y, Z);

					if (LIKELY(Data->IsInWorld(P)))
					{
						FVoxelMaterial Material = Data->GetMaterial(P);

						Material.Alpha = Layer == EVoxelLayer::Layer1 ? 0 : 255;

						// Set index
						if (Layer == EVoxelLayer::Layer1)
						{
							Material.Index1 = MaterialIndex;
						}
						else
						{
							Material.Index2 = MaterialIndex;
						}

						if (Data->IsInWorld(P))
						{
							Data->SetMaterial(P, Material, LastOctree);
						}
					}
				}
			}
		}

		Data->EndSet(Octrees);
	}
	World->UpdateChunksOverlappingBox(Bounds);
}

void UVoxelTools::SetMaterialSphere(AVoxelWorld* World, const FVector Position, const float WorldRadius, const uint8 MaterialIndex, const EVoxelLayer Layer, const float FadeDistance, const float Exponent)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_SetMaterialSphere);

	if (World == nullptr)
	{
		UE_LOG(LogVoxel, Error, TEXT("SetMaterialSphere: World is NULL"));
		return;
	}

	const float Radius = WorldRadius / World->GetVoxelSize();
	const FIntVector LocalPosition = World->GlobalToLocal(Position);
	const int Size = FMath::CeilToInt(Radius + FadeDistance);
	const float VoxelDiagonalLength = 1.73205080757f;

	FIntVector R(Size + 1, Size + 1, Size + 1);
	const FIntBox Bounds = FIntBox(LocalPosition - R, LocalPosition + R);

	FValueOctree* LastOctree = nullptr;
	FVoxelData* Data = World->GetData();

	{
		TArray<uint64> Octrees;
		{
			SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_BeginSet);
			Octrees = Data->BeginSet(Bounds);
		}
		for (int X = -Size; X <= Size; X++)
		{
			for (int Y = -Size; Y <= Size; Y++)
			{
				for (int Z = -Size; Z <= Size; Z++)
				{
					const FIntVector CurrentPosition = LocalPosition + FIntVector(X, Y, Z);
					const float Distance = FVector(X, Y, Z).Size();

					if (Distance <= Radius + FadeDistance + VoxelDiagonalLength)
					{
						FVoxelMaterial Material = Data->GetMaterial(CurrentPosition);

						// Set alpha
						float Blend = FMath::Clamp((Radius + FadeDistance - Distance) / FMath::Max(1.f, FadeDistance), 0.f, 1.f);
						if (Layer == EVoxelLayer::Layer1)
						{
							Blend = 1 - Blend;
						}

						int8 Alpha = FMath::Clamp<int>(FMath::Pow(Blend, Exponent) * 255, 0, 255);
						 
						if ((Layer == EVoxelLayer::Layer1 ? Material.Index1 : Material.Index2) == MaterialIndex)
						{
							// Same index, don't override alpha if smaller
							Alpha = Layer == EVoxelLayer::Layer1 ? FMath::Min<uint8>(Alpha, Material.Alpha) : FMath::Max<uint8>(Alpha, Material.Alpha);
						}
						Material.Alpha = Alpha;

						// Set index
						if (Layer == EVoxelLayer::Layer1)
						{
							Material.Index1 = MaterialIndex;
						}
						else
						{
							Material.Index2 = MaterialIndex;
						}

						if (LIKELY(Data->IsInWorld(CurrentPosition)))
						{
							// Apply changes
							Data->SetMaterial(CurrentPosition, Material, LastOctree);
						}
					}
				}
			}
		}
		Data->EndSet(Octrees);
	}
	World->UpdateChunksOverlappingBox(Bounds);
}


void FindModifiedPositionsForRaycasts(AVoxelWorld* World, const FVector StartPosition, const FVector Direction, const float Radius, const float ToolHeight, const float Precision,
	const bool bShowRaycasts, const bool bShowHitPoints, const bool bShowModifiedVoxels, TArray<TTuple<FIntVector, float>>& OutModifiedPositionsAndDistances)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_FindModifiedPositionsForRaycasts);

	const FVector ToolPosition = StartPosition - Direction * ToolHeight;

	/**
	* Create a 2D basis from (Tangent, Bitangent)
	*/
	// Compute tangent
	FVector Tangent;
	// N dot T = 0
	// <=> N.X * T.X + N.Y * T.Y + N.Z * T.Z = 0
	// <=> T.Z = -1 / N.Z * (N.X * T.X + N.Y * T.Y) if N.Z != 0
	if (Direction.Z != 0)
	{
		Tangent.X = 1;
		Tangent.Y = 1;
		Tangent.Z = -1 / Direction.Z * (Direction.X * Tangent.X + Direction.Y * Tangent.Y);
	}
	else
	{
		Tangent = FVector(1, 0, 0);
	}
	Tangent.Normalize();

	// Compute bitangent
	const FVector Bitangent = FVector::CrossProduct(Tangent, Direction).GetSafeNormal();

	TSet<FIntVector> AddedPoints;

	// Scale to make sure we don't miss any point when rounding
	const float Scale = Precision * World->GetVoxelSize();

	for (int X = -Radius; X <= Radius; X += Scale)
	{
		for (int Y = -Radius; Y <= Radius; Y += Scale)
		{
			const float Distance = FVector2D(X, Y).Size();
			if (Distance < Radius)
			{
				FHitResult Hit;
				// Use 2D basis
				FVector Start = ToolPosition + (Tangent * X + Bitangent * Y);
				FVector End = Start + Direction * ToolHeight * 2;
				if (bShowRaycasts)
				{
					DrawDebugLine(World->GetWorld(), Start, End, FColor::Magenta, false, 1);
				}
				if (World->GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_WorldStatic) && Hit.Actor->IsA(AVoxelWorld::StaticClass()))
				{
					if (bShowHitPoints)
					{
						DrawDebugPoint(World->GetWorld(), Hit.ImpactPoint, 2, FColor::Red, false, 1);
					}
					for (auto Point : World->GetNeighboringPositions(Hit.ImpactPoint))
					{
						if (bShowModifiedVoxels)
						{
							DrawDebugPoint(World->GetWorld(), World->LocalToGlobal(Point), 3, FColor::White, false, 1);
						}
						if (!AddedPoints.Contains(Point) && LIKELY(World->IsInWorld(Point)))
						{
							AddedPoints.Add(Point);
							OutModifiedPositionsAndDistances.Add(TTuple<FIntVector, float>(Point, Distance));
						}
					}
				}
			}
		}
	}
}

void UVoxelTools::SetValueProjection(AVoxelWorld* World, const FVector StartPosition, const FVector Direction, const float Radius, const float Strength, const bool bAdd,
	const float ToolHeight, const float Precision, const bool bShowRaycasts, const bool bShowHitPoints, const bool bShowModifiedVoxels)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_SetValueProjection);

	if (!World)
	{
		UE_LOG(LogVoxel, Error, TEXT("SetValueProjection: World is NULL"));
		return;
	}

	TArray<TTuple<FIntVector, float>> ModifiedPositionsAndDistances;
	FindModifiedPositionsForRaycasts(World, StartPosition, Direction, Radius, ToolHeight, Precision, bShowRaycasts, bShowHitPoints, bShowModifiedVoxels, ModifiedPositionsAndDistances);
	
	if (ModifiedPositionsAndDistances.Num() > 0)
	{
		FIntVector Min(MAX_int32, MAX_int32, MAX_int32);
		FIntVector Max(MIN_int32, MIN_int32, MIN_int32);
		for (auto& Tuple : ModifiedPositionsAndDistances)
		{
			const FIntVector Point = Tuple.Get<0>();
			Min.X = FMath::Min(Min.X, Point.X);
			Min.Y = FMath::Min(Min.Y, Point.Y);
			Min.Z = FMath::Min(Min.Z, Point.Z);

			Max.X = FMath::Max(Max.X, Point.X);
			Max.Y = FMath::Max(Max.Y, Point.Y);
			Max.Z = FMath::Max(Max.Z, Point.Z);
		}

		FVoxelData* Data = World->GetData();
		TArray<uint64> Octrees;
		{
			SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_BeginSet);
			Octrees = Data->BeginSet(FIntBox(Min, FIntVector(Max.X + 1, Max.Y + 1, Max.Z + 1)));
		}

		for (auto& Tuple : ModifiedPositionsAndDistances)
		{
			const FIntVector Point = Tuple.Get<0>();
			const float Distance = Tuple.Get<1>();
			if (World->IsInWorld(Point))
			{
				if (bAdd)
				{
					Data->SetValue(Point, FMath::Clamp<float>(Data->GetValue(Point) - Strength, -1, 1));
				}
				else
				{
					Data->SetValue(Point, FMath::Clamp<float>(Data->GetValue(Point) + Strength, -1, 1));
				}
				World->UpdateChunksAtPosition(Point);
			}
		}

		Data->EndSet(Octrees);
		
		for (auto& Tuple : ModifiedPositionsAndDistances)
		{
			const FIntVector Point = Tuple.Get<0>();
			World->UpdateChunksAtPosition(Point);
		}
	}
}

void UVoxelTools::SetMaterialProjection(AVoxelWorld * World, const FVector StartPosition, const FVector Direction, const float Radius, const uint8 MaterialIndex, const EVoxelLayer Layer,
	const float FadeDistance, const float Exponent, const float ToolHeight, const float Precision, const bool bShowRaycasts, const bool bShowHitPoints, const bool bShowModifiedVoxels)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_SetMaterialProjection);

	if (!World)
	{
		UE_LOG(LogVoxel, Error, TEXT("SetMaterialProjection: World is NULL"));
		return;
	}

	const float VoxelDiagonalLength = 1.73205080757f * World->GetVoxelSize();

	TArray<TTuple<FIntVector, float>> ModifiedPositionsAndDistances;
	FindModifiedPositionsForRaycasts(World, StartPosition, Direction, Radius + FadeDistance + 2 * VoxelDiagonalLength, ToolHeight, Precision, bShowRaycasts, bShowHitPoints, bShowModifiedVoxels, ModifiedPositionsAndDistances);
	
	if (ModifiedPositionsAndDistances.Num() > 0)
	{
		FIntVector Min(MAX_int32, MAX_int32, MAX_int32);
		FIntVector Max(MIN_int32, MIN_int32, MIN_int32);
		for (auto& Tuple : ModifiedPositionsAndDistances)
		{
			const FIntVector Point = Tuple.Get<0>();
			Min.X = FMath::Min(Min.X, Point.X);
			Min.Y = FMath::Min(Min.Y, Point.Y);
			Min.Z = FMath::Min(Min.Z, Point.Z);

			Max.X = FMath::Max(Max.X, Point.X);
			Max.Y = FMath::Max(Max.Y, Point.Y);
			Max.Z = FMath::Max(Max.Z, Point.Z);
		}

		FVoxelData* Data = World->GetData();
		TArray<uint64> Octrees;
		{
			SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_BeginSet);
			Octrees = Data->BeginSet(FIntBox(Min, FIntVector(Max.X + 1, Max.Y + 1, Max.Z + 1)));
		}

		for (auto& Tuple : ModifiedPositionsAndDistances)
		{
			const FIntVector CurrentPosition = Tuple.Get<0>();
			const float Distance = Tuple.Get<1>();

			if (Distance <= Radius + FadeDistance + VoxelDiagonalLength)
			{
				FVoxelMaterial Material = Data->GetMaterial(CurrentPosition);

				// Set alpha
				float Blend = FMath::Clamp((Radius + FadeDistance - Distance) / FMath::Max(1.f, FadeDistance), 0.f, 1.f);
				if (Layer == EVoxelLayer::Layer1)
				{
					Blend = 1 - Blend;
				}

				int8 Alpha = FMath::Clamp<int>(FMath::Pow(Blend, Exponent) * 255, 0, 255);

				if ((Layer == EVoxelLayer::Layer1 ? Material.Index1 : Material.Index2) == MaterialIndex)
				{
					// Same index, don't override alpha if smaller
					Alpha = Layer == EVoxelLayer::Layer1 ? FMath::Min<uint8>(Alpha, Material.Alpha) : FMath::Max<uint8>(Alpha, Material.Alpha);
				}
				Material.Alpha = Alpha;

				// Set index
				if (Layer == EVoxelLayer::Layer1)
				{
					Material.Index1 = MaterialIndex;
				}
				else
				{
					Material.Index2 = MaterialIndex;
				}

				if (Data->IsInWorld(CurrentPosition))
				{
					Data->SetMaterial(CurrentPosition, Material);
				}
			}
		}

		Data->EndSet(Octrees);

		for (auto& Tuple : ModifiedPositionsAndDistances)
		{
			const FIntVector Point = Tuple.Get<0>();
			World->UpdateChunksAtPosition(Point);
		}
	}
}

void UVoxelTools::Flatten(AVoxelWorld* World, FVector Position, FVector Normal, float WorldRadius, float Strength, bool bDontModifyVoxelsAroundPosition, bool bDontModifyEmptyVoxels, bool bDontModifyFullVoxels, bool bShowModifiedVoxels, bool bShowTestedVoxels)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_Flatten);

	if (!World)
	{
		UE_LOG(LogVoxel, Error, TEXT("Flatten: World is NULL"));
		return;
	}

	const FVector LocalPosition = World->GlobalToLocalFloat(Position);
	const float Radius = WorldRadius / World->GetVoxelSize();
	const int IntRadius = FMath::CeilToInt(Radius);

	/**
	 * Create a 2D basis from (Tangent, Bitangent)
	 */
	 // Compute tangent
	FVector Tangent;
	{
		// N dot T = 0
	// <=> N.X * T.X + N.Y * T.Y + N.Z * T.Z = 0
	// <=> T.Z = -1 / N.Z * (N.X * T.X + N.Y * T.Y) if N.Z != 0
		if (Normal.Z != 0)
		{
			Tangent.X = 1;
			Tangent.Y = 1;
			Tangent.Z = -1 / Normal.Z * (Normal.X * Tangent.X + Normal.Y * Tangent.Y);
		}
		else
		{
			Tangent = FVector(1, 0, 0);
		}
		Tangent.Normalize();
	}

	// Compute bitangent
	const FVector Bitangent = FVector::CrossProduct(Tangent, Normal).GetSafeNormal();
	const FPlane Plane(LocalPosition, Normal);

	TSet<TTuple<FIntVector, float>> Positions;
	TSet<FIntVector> AddedPositions;

	for (int X = -IntRadius; X <= IntRadius; X++)
	{
		for (int Y = -IntRadius; Y <= IntRadius; Y++)
		{
			if (FVector2D(X, Y).Size() <= Radius)
			{
				for (float Z = -1; Z < 1 + KINDA_SMALL_NUMBER; Z += 0.5)
				{
					FVector P = Tangent * X + Bitangent * Y + LocalPosition + Z * Normal;
					for (auto& N : World->GetNeighboringPositions(World->LocalToGlobalFloat(P)))
					{
						if (!AddedPositions.Contains(N))
						{
							if (bShowTestedVoxels)
							{
								DrawDebugPoint(World->GetWorld(), World->LocalToGlobal(N), 5, Plane.PlaneDot((FVector)N) < 0 ? FColor::Purple : FColor::Cyan, false, 1);
							}

							Positions.Add(TTuple<FIntVector, float>(N, Plane.PlaneDot((FVector)N)));
							AddedPositions.Add(N);
						}
					}
				}
			}
		}
	}

	// We don't want to modify the normal
	if (bDontModifyVoxelsAroundPosition)
	{
		TSet<FIntVector> SafePoints(World->GetNeighboringPositions(Position));
		AddedPositions = AddedPositions.Difference(SafePoints);
	}

	FIntVector Min(MAX_int32, MAX_int32, MAX_int32);
	FIntVector Max(MIN_int32, MIN_int32, MIN_int32);
	for (auto& Point : AddedPositions)
	{
		Min.X = FMath::Min(Min.X, Point.X);
		Min.Y = FMath::Min(Min.Y, Point.Y);
		Min.Z = FMath::Min(Min.Z, Point.Z);

		Max.X = FMath::Max(Max.X, Point.X);
		Max.Y = FMath::Max(Max.Y, Point.Y);
		Max.Z = FMath::Max(Max.Z, Point.Z);
	}

	FIntBox Bounds(Min, FIntVector(Max.X + 1, Max.Y + 1, Max.Z + 1));

	FVoxelData* Data = World->GetData();
	TArray<uint64> Octrees;
	{
		SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_BeginSet);
		Octrees = Data->BeginSet(Bounds);
	}

	for (auto& T : Positions)
	{
		FIntVector P = T.Get<0>();
		float F = T.Get<1>();

		if (AddedPositions.Contains(P))
		{
			if (bShowModifiedVoxels)
			{
				DrawDebugPoint(World->GetWorld(), World->LocalToGlobal(P), 10, FColor::Red, false, 1);
			}
			float Value = Data->GetValue(P);
			if ((KINDA_SMALL_NUMBER - 1.f < Value || !bDontModifyFullVoxels) &&
				(Value < 1.f - KINDA_SMALL_NUMBER || !bDontModifyEmptyVoxels))
			{
				if (Data->IsInWorld(P))
				{
					Data->SetValue(P, FMath::Clamp<float>(Value + (F - Value) * Strength, -1, 1));
				}
			}
		}
	}

	Data->EndSet(Octrees);

	World->UpdateChunksOverlappingBox(Bounds);
}

void UVoxelTools::ImportAsset(AVoxelWorld* World,
							  UVoxelAsset* InAsset,
							  FVector Position,
						 	  bool bAdd /*= true*/,
							  EVoxelAssetPositionOffset XOffset /*= EVoxelAssetPositionOffset::Default*/,
							  EVoxelAssetPositionOffset YOffset /*= EVoxelAssetPositionOffset::Default*/,
							  EVoxelAssetPositionOffset ZOffset /*= EVoxelAssetPositionOffset::Default*/,
							  bool bForceUseOfAllVoxels /*= false*/)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_ImportAsset);

	if (!World)
	{
		UE_LOG(LogVoxel, Error, TEXT("ImportAsset: World is NULL"));
		return;
	}
	check(World);

	if (!InAsset)
	{
		UE_LOG(LogVoxel, Error, TEXT("ImportAsset: Asset is NULL"));
		return;
	}

	FIntVector P = World->GlobalToLocal(Position);

	TSharedRef<FVoxelAssetInstance> Asset = InAsset->GetAsset(FIntVector::ZeroValue);

	FIntBox Bounds = Asset->GetLocalBounds();
	FVoxelData* Data = World->GetData();
	FValueOctree* LastOctree = nullptr;

	switch (XOffset)
	{
	case EVoxelAssetPositionOffset::Default:
		break;
	case EVoxelAssetPositionOffset::PositionIsBottom:
		P.X -= Bounds.Min.X;
		break;
	case EVoxelAssetPositionOffset::PositionIsMiddle:
		P.X -= Bounds.Min.X + (Bounds.Max.X - Bounds.Min.X) / 2;
		break;
	case EVoxelAssetPositionOffset::PositionIsTop:
		P.X -= Bounds.Max.X;
		break;
	default:
		check(false);
		break;
	}

	switch (YOffset)
	{
	case EVoxelAssetPositionOffset::Default:
		break;
	case EVoxelAssetPositionOffset::PositionIsBottom:
		P.Y -= Bounds.Min.Y;
		break;
	case EVoxelAssetPositionOffset::PositionIsMiddle:
		P.Y -= Bounds.Min.Y + (Bounds.Max.Y - Bounds.Min.Y) / 2;
		break;
	case EVoxelAssetPositionOffset::PositionIsTop:
		P.Y -= Bounds.Max.Y;
		break;
	default:
		check(false);
		break;
	}

	switch (ZOffset)
	{
	case EVoxelAssetPositionOffset::Default:
		break;
	case EVoxelAssetPositionOffset::PositionIsBottom:
		P.Z -= Bounds.Min.Z;
		break;
	case EVoxelAssetPositionOffset::PositionIsMiddle:
		P.Z -= Bounds.Min.Z + (Bounds.Max.Z - Bounds.Min.Z) / 2;
		break;
	case EVoxelAssetPositionOffset::PositionIsTop:
		P.Z -= Bounds.Max.Z;
		break;
	default:
		check(false);
		break;
	}

	{
		TArray<uint64> Octrees;
		{
			SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_BeginSet);
			Octrees = Data->BeginSet(Bounds.TranslateBy(P));
		}
		for (int X = Bounds.Min.X; X <= Bounds.Max.X; X++)
		{
			for (int Y = Bounds.Min.Y; Y <= Bounds.Max.Y; Y++)
			{
				for (int Z = Bounds.Min.Z; Z <= Bounds.Max.Z; Z++)
				{
					float AssetValue;
					FVoxelMaterial AssetMaterial;
					FVoxelType VoxelType;
					Asset->GetValueAndMaterialAndVoxelType(X, Y, Z, AssetValue, AssetMaterial, VoxelType);
					AssetValue *= (bAdd ? 1 : -1);

					const FIntVector CurrentPosition(P.X + X, P.Y + Y, P.Z + Z);

					if (bForceUseOfAllVoxels)
					{
						if (LIKELY(Data->IsInWorld(CurrentPosition)))
						{
							Data->SetValueAndMaterial(CurrentPosition, AssetValue, AssetMaterial, LastOctree);
						}
					}
					else if (VoxelType != FVoxelType::IgnoreAll())
					{
						float OldValue;
						FVoxelMaterial OldMaterial;
						Data->GetValueAndMaterial(CurrentPosition, OldValue, OldMaterial);

						const FVoxelMaterial NewMaterial = (VoxelType.GetMaterialType() == EVoxelMaterialType::UseMaterial) ? AssetMaterial : OldMaterial;
						float NewValue;

						switch (VoxelType.GetValueType())
						{
						case EVoxelValueType::IgnoreValue:
							NewValue = OldValue;
							break;
						case EVoxelValueType::UseValueIfSameSign:
							NewValue = FVoxelUtilities::HaveSameSign(OldValue, AssetValue) ? AssetValue : OldValue;
							break;
						case EVoxelValueType::UseValue:
							NewValue = AssetValue;
							break;
						default:
							NewValue = 0;
							check(false);
						}

						if (LIKELY(Data->IsInWorld(CurrentPosition)))
						{
							Data->SetValueAndMaterial(CurrentPosition, NewValue, NewMaterial, LastOctree);
						}
					}
				}
			}
		}
		Data->EndSet(Octrees);
	}

	World->UpdateChunksOverlappingBox(Bounds.TranslateBy(P));
}

void UVoxelTools::GetVoxelWorld(FVector WorldPosition, FVector WorldDirection, float MaxDistance, APlayerController* PlayerController, bool bMultipleHits, AVoxelWorld*& World, FVector& HitPosition, FVector& HitNormal)
{
	if (!PlayerController)
	{
		UE_LOG(LogVoxel, Error, TEXT("GetVoxelWorld: Invalid PlayerController"));
		return;
	}

	TArray<FHitResult> HitResults;
	if (PlayerController->GetWorld()->LineTraceMultiByChannel(HitResults, WorldPosition, WorldPosition + WorldDirection * MaxDistance, ECC_WorldStatic))
	{
		for (int Index = 0; Index < HitResults.Num(); Index++)
		{
			auto& HitResult = HitResults[Index];
			HitPosition = HitResult.ImpactPoint;
			HitNormal = HitResult.ImpactNormal;

			if (HitResult.Actor->IsA(AVoxelWorld::StaticClass()))
			{
				World = Cast<AVoxelWorld>(HitResult.Actor.Get());
				check(World);
			}
		}
	}
}

void UVoxelTools::GetMouseWorldPositionAndDirection(APlayerController* PlayerController, FVector& WorldPosition, FVector& WorldDirection, EBlueprintSuccess& Branches)
{
	if (!PlayerController)
	{
		UE_LOG(LogVoxel, Error, TEXT("GetMouseWorldPositionAndDirection: Invalid PlayerController"));
		Branches = EBlueprintSuccess::Failed;
		return;
	}
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(PlayerController->Player);

	if (PlayerController->GetLocalPlayer() && PlayerController->GetLocalPlayer()->ViewportClient)
	{
		FVector2D MousePosition;
		if (PlayerController->GetLocalPlayer()->ViewportClient->GetMousePosition(MousePosition))
		{
			// Early out if we clicked on a HUD hitbox
			if (PlayerController->GetHUD() != NULL && PlayerController->GetHUD()->GetHitBoxAtCoordinates(MousePosition, true))
			{
				Branches = EBlueprintSuccess::Failed;
			}
			else
			{
				if (UGameplayStatics::DeprojectScreenToWorld(PlayerController, MousePosition, WorldPosition, WorldDirection) == true)
				{
					Branches = EBlueprintSuccess::Success;
				}
				else
				{
					Branches = EBlueprintSuccess::Failed;
				}
			}
		}
		else
		{
			Branches = EBlueprintSuccess::Failed;
		}
	}
	else
	{
		Branches = EBlueprintSuccess::Failed;
	}
}

void UVoxelTools::RemoveFloatingBlocks(AVoxelWorld* World, TArray<AVoxelPart*>& SpawnedActors, TSubclassOf<AVoxelPart> ClassToSpawn, FVector Position, float Radius)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_RemoveFloatingBlocks);
	// TODO: what if data change between the 2 parts? Maybe one BeginSet?
	if (World == nullptr)
	{
		UE_LOG(LogVoxel, Error, TEXT("Remove floating blocks: World is NULL"));
		return;
	}
	check(World);

	if (!ClassToSpawn)
	{
		ClassToSpawn = AVoxelPartSimpleMesh::StaticClass();
	}
	check(ClassToSpawn);

	// Position in voxel space
	const FIntVector LocalPosition = World->GlobalToLocal(Position);
	const int IntRadius = FMath::CeilToInt(Radius) + 2;

	FIntVector R;
	const FIntBox Bounds(LocalPosition - R, LocalPosition + R);

	TArray<bool> Visited;
	Visited.SetNumZeroed(2 * IntRadius * 2 * IntRadius * 2 * IntRadius);

	FVoxelData* WorldData = World->GetData();

	// Fill Visited array
	{
		TArray<FIntVector> Queue;

		// Add borders
		for (int X = -IntRadius; X < IntRadius; X++)
		{
			for (int Y = -IntRadius; Y < IntRadius; Y++)
			{
				Queue.Add(FIntVector(-IntRadius, X, Y));
				Queue.Add(FIntVector(IntRadius - 1, X, Y));

				Queue.Add(FIntVector(X, -IntRadius, Y));
				Queue.Add(FIntVector(X, IntRadius - 1, Y));

				Queue.Add(FIntVector(X, Y, -IntRadius));
				Queue.Add(FIntVector(X, Y, IntRadius - 1));
			}
		}

		{
			auto Octrees = WorldData->BeginGet(Bounds);
			while (Queue.Num() > 0)
			{
				const FIntVector RelativePosition = Queue.Pop(false);

				const int X = RelativePosition.X;
				const int Y = RelativePosition.Y;
				const int Z = RelativePosition.Z;

				const int Index = (X + IntRadius) + (Y + IntRadius) * 2 * IntRadius + (Z + IntRadius) * 2 * IntRadius * 2 * IntRadius;

				if ((-IntRadius <= X) && (X < IntRadius) &&
					(-IntRadius <= Y) && (Y < IntRadius) &&
					(-IntRadius <= Z) && (Z < IntRadius) &&
					!Visited[Index] &&
					(WorldData->GetValue(RelativePosition + LocalPosition) <= 0))
				{

					Visited[Index] = true;

					Queue.Add(FIntVector(X - 1, Y - 1, Z - 1));
					Queue.Add(FIntVector(X + 0, Y - 1, Z - 1));
					Queue.Add(FIntVector(X + 1, Y - 1, Z - 1));
					Queue.Add(FIntVector(X - 1, Y + 0, Z - 1));
					Queue.Add(FIntVector(X + 0, Y + 0, Z - 1));
					Queue.Add(FIntVector(X + 1, Y + 0, Z - 1));
					Queue.Add(FIntVector(X - 1, Y + 1, Z - 1));
					Queue.Add(FIntVector(X + 0, Y + 1, Z - 1));
					Queue.Add(FIntVector(X + 1, Y + 1, Z - 1));
					Queue.Add(FIntVector(X - 1, Y - 1, Z + 0));
					Queue.Add(FIntVector(X + 0, Y - 1, Z + 0));
					Queue.Add(FIntVector(X + 1, Y - 1, Z + 0));
					Queue.Add(FIntVector(X - 1, Y + 0, Z + 0));
					Queue.Add(FIntVector(X + 0, Y + 0, Z + 0));
					Queue.Add(FIntVector(X + 1, Y + 0, Z + 0));
					Queue.Add(FIntVector(X - 1, Y + 1, Z + 0));
					Queue.Add(FIntVector(X + 0, Y + 1, Z + 0));
					Queue.Add(FIntVector(X + 1, Y + 1, Z + 0));
					Queue.Add(FIntVector(X - 1, Y - 1, Z + 1));
					Queue.Add(FIntVector(X + 0, Y - 1, Z + 1));
					Queue.Add(FIntVector(X + 1, Y - 1, Z + 1));
					Queue.Add(FIntVector(X - 1, Y + 0, Z + 1));
					Queue.Add(FIntVector(X + 0, Y + 0, Z + 1));
					Queue.Add(FIntVector(X + 1, Y + 0, Z + 1));
					Queue.Add(FIntVector(X - 1, Y + 1, Z + 1));
					Queue.Add(FIntVector(X + 0, Y + 1, Z + 1));
					Queue.Add(FIntVector(X + 1, Y + 1, Z + 1));
				}
			}
			WorldData->EndGet(Octrees);
		}
	}

	uint8 LOD = FMath::CeilToInt(FMath::Log2(FMath::Max(2.f, 2 * IntRadius / (float)DATA_CHUNK_SIZE)));
	TSharedRef<FVoxelWorldGeneratorInstance> WorldGenerator = MakeShareable(new FEmptyWorldGeneratorInstance());
	TSharedRef<FVoxelData> Data = MakeShareable(new FVoxelData(LOD, WorldGenerator, false));

	// List of the points to remove
	TArray<FIntVector> PointPositions;
	TArray<FIntVector> PositionsToUpdate;
	{
		auto Octrees = WorldData->BeginGet(Bounds);
		bool bIsSet = false;

		FValueOctree* LastOctree = nullptr;
		FValueOctree* WorldLastOctree = nullptr;

		for (int Z = -IntRadius; Z < IntRadius; Z++)
		{
			for (int Y = -IntRadius; Y < IntRadius; Y++)
			{
				for (int X = -IntRadius; X < IntRadius; X++)
				{
					const int Index = (X + IntRadius) + (Y + IntRadius) * 2 * IntRadius + (Z + IntRadius) * 2 * IntRadius * 2 * IntRadius;
					const FIntVector RelativePosition = FIntVector(X, Y, Z);
					const FIntVector CurrentPosition = RelativePosition + LocalPosition;

					if (!Visited[Index] && WorldData->GetValue(CurrentPosition) <= 0)
					{
						// BeginSet is expensive, so we avoid calling it until we really have to
						if (!bIsSet)
						{
							WorldData->EndGet(Octrees);
							Octrees = WorldData->BeginSet(Bounds);
							bIsSet = true;
						}

						float Value;
						FVoxelMaterial Material;
						WorldData->GetValueAndMaterial(CurrentPosition, Value, Material);

						Data->SetValueAndMaterial(RelativePosition, Value, Material, LastOctree);

						// Set external colors
						TArray<FIntVector> L = {
							FIntVector(1,  0,  0),
							FIntVector(0,  1,  0),
							FIntVector(1,  1,  0),
							FIntVector(0,  0,  1),
							FIntVector(1,  0,  1),
							FIntVector(0,  1,  1),
							FIntVector(1,  1,  1),

							FIntVector(-1,  0,  0),
							FIntVector(0, -1,  0),
							FIntVector(-1, -1,  0),
							FIntVector(0,  0, -1),
							FIntVector(-1,  0, -1),
							FIntVector(0, -1, -1),
							FIntVector(-1, -1, -1)
						};
						for (auto P : L)
						{
							Data->SetMaterial(RelativePosition + P, WorldData->GetMaterial(CurrentPosition + P), LastOctree);
						}

						PointPositions.Add(RelativePosition);

						WorldData->SetValue(CurrentPosition, 1, WorldLastOctree);
						PositionsToUpdate.Add(CurrentPosition);
					}
				}
			}
		}

		if (bIsSet)
		{
			WorldData->EndSet(Octrees);
		}
		else
		{
			WorldData->EndGet(Octrees);
		}
	}

	for (auto P : PositionsToUpdate)
	{
		World->UpdateChunksAtPosition(P);
	}

	while (PointPositions.Num() > 0)
	{
		// Find all connected points to this point to create different VoxelParts for each section

		TSharedPtr<FVoxelData> CurrentData = MakeShareable(new FVoxelData(LOD, WorldGenerator, false));
		FValueOctree* CurrentLastOctree = nullptr;

		FIntVector PointPosition = PointPositions.Pop(false);

		const int Index = (PointPosition.X + IntRadius) + (PointPosition.Y + IntRadius) * 2 * IntRadius + (PointPosition.Z + IntRadius) * 2 * IntRadius * 2 * IntRadius;

		if (!Visited[Index])
		{
			TArray<FIntVector> Queue;
			Queue.Add(PointPosition);

			while (Queue.Num() > 0)
			{
				const FIntVector CurrentPosition = Queue.Pop(false);

				const int X = CurrentPosition.X;
				const int Y = CurrentPosition.Y;
				const int Z = CurrentPosition.Z;

				const int LocalIndex = (X + IntRadius) + (Y + IntRadius) * 2 * IntRadius + (Z + IntRadius) * 2 * IntRadius * 2 * IntRadius;

				if ((-IntRadius <= X) && (X < IntRadius) &&
					(-IntRadius <= Y) && (Y < IntRadius) &&
					(-IntRadius <= Z) && (Z < IntRadius) &&
					!Visited[LocalIndex])
				{

					float Value;
					FVoxelMaterial Material;
					Data->GetValueAndMaterial(X, Y, Z, Value, Material);

					if (Value <= 0)
					{
						Visited[LocalIndex] = true;
						CurrentData->SetValueAndMaterial(CurrentPosition, Value, Material, CurrentLastOctree);

						// Set external colors
						TArray<FIntVector> L = {
							FIntVector(1,  0,  0),
							FIntVector(0,  1,  0),
							FIntVector(1,  1,  0),
							FIntVector(0,  0,  1),
							FIntVector(1,  0,  1),
							FIntVector(0,  1,  1),
							FIntVector(1,  1,  1),
							FIntVector(-1,  0,  0),
							FIntVector(0, -1,  0),
							FIntVector(-1, -1,  0),
							FIntVector(0,  0, -1),
							FIntVector(-1,  0, -1),
							FIntVector(0, -1, -1),
							FIntVector(-1, -1, -1)
						};
						for (auto P : L)
						{
							CurrentData->SetMaterial(CurrentPosition + P, Data->GetMaterial(CurrentPosition));
						}

						Queue.Add(FIntVector(X - 1, Y - 1, Z - 1));
						Queue.Add(FIntVector(X + 0, Y - 1, Z - 1));
						Queue.Add(FIntVector(X + 1, Y - 1, Z - 1));
						Queue.Add(FIntVector(X - 1, Y + 0, Z - 1));
						Queue.Add(FIntVector(X + 0, Y + 0, Z - 1));
						Queue.Add(FIntVector(X + 1, Y + 0, Z - 1));
						Queue.Add(FIntVector(X - 1, Y + 1, Z - 1));
						Queue.Add(FIntVector(X + 0, Y + 1, Z - 1));
						Queue.Add(FIntVector(X + 1, Y + 1, Z - 1));
						Queue.Add(FIntVector(X - 1, Y - 1, Z + 0));
						Queue.Add(FIntVector(X + 0, Y - 1, Z + 0));
						Queue.Add(FIntVector(X + 1, Y - 1, Z + 0));
						Queue.Add(FIntVector(X - 1, Y + 0, Z + 0));
						Queue.Add(FIntVector(X + 0, Y + 0, Z + 0));
						Queue.Add(FIntVector(X + 1, Y + 0, Z + 0));
						Queue.Add(FIntVector(X - 1, Y + 1, Z + 0));
						Queue.Add(FIntVector(X + 0, Y + 1, Z + 0));
						Queue.Add(FIntVector(X + 1, Y + 1, Z + 0));
						Queue.Add(FIntVector(X - 1, Y - 1, Z + 1));
						Queue.Add(FIntVector(X + 0, Y - 1, Z + 1));
						Queue.Add(FIntVector(X + 1, Y - 1, Z + 1));
						Queue.Add(FIntVector(X - 1, Y + 0, Z + 1));
						Queue.Add(FIntVector(X + 0, Y + 0, Z + 1));
						Queue.Add(FIntVector(X + 1, Y + 0, Z + 1));
						Queue.Add(FIntVector(X - 1, Y + 1, Z + 1));
						Queue.Add(FIntVector(X + 0, Y + 1, Z + 1));
						Queue.Add(FIntVector(X + 1, Y + 1, Z + 1));
					}
				}
			}

			// Create the VoxelPart
			AVoxelPart* Part = Cast<AVoxelPart>(World->GetWorld()->SpawnActor(ClassToSpawn));
			SpawnedActors.Add(Part);

			Part->SetActorLocation(World->LocalToGlobal(LocalPosition));
			Part->Init(CurrentData.Get(), World);
		}
	}
}

UVoxelAsset* UVoxelTools::TransformVoxelAsset(UVoxelAsset* CompressedAsset, const FTransform& Transform)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_TransformVoxelAsset);

	if (!CompressedAsset)
	{
		UE_LOG(LogVoxel, Error, TEXT("TransformVoxelAsset: Invalid Asset"));
		return nullptr;
	}

	TSharedRef<FVoxelAssetInstance> InAsset = CompressedAsset->GetAsset(FIntVector::ZeroValue);
	UVoxelDataAsset* OutAsset = NewObject<UVoxelDataAsset>();

	// Compute new bounds
	FIntBox NewBounds;
	{
		const FIntVector Min = InAsset->GetLocalBounds().Min;
		const FIntVector Max = InAsset->GetLocalBounds().Max;

		TArray<FIntVector> Corners = {
			FIntVector(Min.X, Min.Y, Min.Z),
			FIntVector(Max.X, Min.Y, Min.Z),
			FIntVector(Min.X, Max.Y, Min.Z),
			FIntVector(Max.X, Max.Y, Min.Z),
			FIntVector(Min.X, Min.Y, Max.Z),
			FIntVector(Max.X, Min.Y, Max.Z),
			FIntVector(Min.X, Max.Y, Max.Z),
			FIntVector(Max.X, Max.Y, Max.Z),
		};

		FIntVector NewMin = FIntVector(MAX_int32, MAX_int32, MAX_int32);
		FIntVector NewMax = FIntVector(MIN_int32, MIN_int32, MIN_int32);

		for (auto& Corner : Corners)
		{
			FVector NewPosition = Transform.TransformPosition((FVector)Corner);

			NewMin.X = FMath::Min(NewMin.X, FMath::FloorToInt(NewPosition.X));
			NewMin.Y = FMath::Min(NewMin.Y, FMath::FloorToInt(NewPosition.Y));
			NewMin.Z = FMath::Min(NewMin.Z, FMath::FloorToInt(NewPosition.Z));

			NewMax.X = FMath::Max(NewMax.X, FMath::CeilToInt(NewPosition.X));
			NewMax.Y = FMath::Max(NewMax.Y, FMath::CeilToInt(NewPosition.Y));
			NewMax.Z = FMath::Max(NewMax.Z, FMath::CeilToInt(NewPosition.Z));
		}
		NewBounds = FIntBox(NewMin, FIntVector(NewMax.X + 1, NewMax.Y + 1, NewMax.Z + 1));
	}

	OutAsset->SetSize(NewBounds.Size(), true);

	FIntBox InBounds = InAsset->GetLocalBounds();
	FIntVector InSize = InBounds.Size();
	for (int X = 0; X < InSize.X; X++)
	{
		for (int Y = 0; Y < InSize.Y; Y++)
		{
			for (int Z = 0; Z < InSize.Z; Z++)
			{
				FVector NewPosition = Transform.TransformPosition(FVector(X, Y, Z));
				const int NewX = FMath::RoundToInt(NewPosition.X) - NewBounds.Min.X;
				const int NewY = FMath::RoundToInt(NewPosition.Y) - NewBounds.Min.Y;
				const int NewZ = FMath::RoundToInt(NewPosition.Z) - NewBounds.Min.Z;

				const int OldX = X + InBounds.Min.X;
				const int OldY = Y + InBounds.Min.Y;
				const int OldZ = Z + InBounds.Min.Z;

				check(0 <= NewX && NewX < OutAsset->GetSize().X);
				check(0 <= NewY && NewY < OutAsset->GetSize().Y);
				check(0 <= NewZ && NewZ < OutAsset->GetSize().Z);

				OutAsset->SetValue(NewX, NewY, NewZ, InAsset->GetValue(OldX, OldY, OldZ));
				OutAsset->SetMaterial(NewX, NewY, NewZ, InAsset->GetMaterial(OldX, OldY, OldZ));
				OutAsset->SetVoxelType(NewX, NewY, NewZ, InAsset->GetVoxelType(OldX, OldY, OldZ));
			}
		}
	}
	OutAsset->Save();

	return OutAsset;
}

void UVoxelTools::CreateMeshFromVoxels(const FIntVector& Size, const TArray<float>& Values, const TArray<FVoxelMaterial>& Materials, float VoxelSize, UMaterialInterface* Material, UObject* Parent, UVoxelProceduralMeshComponent*& Mesh)
{
	if (Values.Num() != 0 && Values.Num() != Size.X * Size.Y * Size.Z)
	{
		UE_LOG(LogVoxel, Error, TEXT("PolygonizeSections: Invalid Values"));
		return;
	}
	if (Materials.Num() != 0 && Materials.Num() != Size.X * Size.Y * Size.Z)
	{
		UE_LOG(LogVoxel, Error, TEXT("PolygonizeSections: Invalid Materials"));
		return;
	}
	if (Values.Num() == 0 && Materials.Num() == 0)
	{
		return;
	}

	Mesh = NewObject<UVoxelProceduralMeshComponent>(Parent, NAME_None, RF_Transient);
	Mesh->bUseAsyncCooking = true;
	if (Cast<AActor>(Parent))
	{
		Mesh->SetupAttachment(Cast<AActor>(Parent)->GetRootComponent());
	}
	else if (Cast<USceneComponent>(Parent))
	{
		Mesh->SetupAttachment(Cast<USceneComponent>(Parent));
	}
	Mesh->RegisterComponent();
	Mesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	Mesh->bUseComplexAsSimpleCollision = false;
	Mesh->bCastShadowAsTwoSided = true;
	Mesh->SetSimulatePhysics(false);
	Mesh->SetRelativeLocation(FVector::ZeroVector);

	uint8 LOD = FMath::Max<uint8>(1, FMath::CeilLogTwo(Size.GetMax() / (float)CHUNK_SIZE));
	FVoxelData Data(LOD, MakeShared<FEmptyWorldGeneratorInstance>(), false);

	for (int X = 0; X < Size.X; X++)
	{
		for (int Y = 0; Y < Size.Y; Y++)
		{
			for (int Z = 0; Z < Size.Z; Z++)
			{
				int Index = X + Size.X * Y + Size.X * Size.Y * Z;
				if (Values.Num() > 0)
				{
					Data.SetValue(X, Y, Z, Values[Index]);
				}
				if (Materials.Num() > 0)
				{
					Data.SetMaterial(X, Y, Z, Materials[Index]);
				}
			}
		}
	}

	const int S = Data.Size() / 2;
	int SectionIndex = 0;

	for (int X = -CHUNK_SIZE; X < S; X += CHUNK_SIZE)
	{
		for (int Y = -CHUNK_SIZE; Y < S; Y += CHUNK_SIZE)
		{
			for (int Z = -CHUNK_SIZE; Z < S; Z += CHUNK_SIZE)
			{
				const FIntVector Position = FIntVector(X, Y, Z);

				TSharedPtr<FAsyncPolygonizerWork> Thread = MakeShareable(new FAsyncPolygonizerWork(0, &Data, Position, FIntVector(0, 0, 0), nullptr));
				Thread->DoWork();

				// Mesh
				{
					FVoxelProcMeshSection Section;
					Section.bEnableCollision = true;
					Thread->Chunk.InitSectionBuffers(Section.ProcVertexBuffer, Section.ProcIndexBuffer, 0);

					TArray<FVector> Vertices;
					Vertices.SetNumUninitialized(Section.ProcVertexBuffer.Num());

					int i = 0;
					Section.SectionLocalBox.Init();
					for (FVoxelProcMeshVertex& ProcVertex : Section.ProcVertexBuffer)
					{
						ProcVertex.Position += (FVector)Position;
						ProcVertex.Position *= VoxelSize;
						Section.SectionLocalBox += ProcVertex.Position;
						Vertices[i] = ProcVertex.Position;
						i++;
					}

					Mesh->SetProcMeshSection(SectionIndex, Section);
					Mesh->SetMaterial(SectionIndex, Material);
					Mesh->AddCollisionConvexMesh(Vertices);
					SectionIndex++;
				}
			}
		}
	}
}

