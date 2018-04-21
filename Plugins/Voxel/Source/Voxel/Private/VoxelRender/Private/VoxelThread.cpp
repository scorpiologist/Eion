// Copyright 2018 Phyronnaz

#include "VoxelThread.h"
#include "VoxelPrivate.h"
#include "VoxelData.h"
#include "VoxelWorld.h"
#include "VoxelWorldGenerator.h"
#include "Kismet/KismetMathLibrary.h"
#include "VoxelCrashReporter.h"
#include "VoxelActor.h"
#include "Async.h"

DECLARE_CYCLE_STAT(TEXT("FVoxelAsyncWork::WaitForCompletion"), STAT_FVoxelAsyncWork_WaitForCompletion, STATGROUP_Voxel);

DECLARE_CYCLE_STAT(TEXT("FAsyncPolygonizerWork::DoWork"), STAT_FAsyncPolygonizerWork_DoWork, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FAsyncPolygonizerWork::DoWork.Mesh"), STAT_FAsyncPolygonizerWork_DoWork_Mesh, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FAsyncPolygonizerWork::DoWork.Grass"), STAT_FAsyncPolygonizerWork_DoWork_Grass, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FAsyncPolygonizerWork::DoWork.Actors"), STAT_FAsyncPolygonizerWork_DoWork_Actors, STATGROUP_Voxel);

DECLARE_CYCLE_STAT(TEXT("FAsyncPolygonizerWorkForTransitions::DoWork"), STAT_FAsyncPolygonizerWorkForTransitions_DoWork, STATGROUP_Voxel);

FVoxelAsyncWork::FVoxelAsyncWork()
{
	DoneEvent = FPlatformProcess::GetSynchEventFromPool(true);
	DoneEvent->Reset();
}

FVoxelAsyncWork::~FVoxelAsyncWork()
{
	if (DoneEvent)
	{
		FPlatformProcess::ReturnSynchEventToPool(DoneEvent);
		DoneEvent = nullptr;
	}
}

void FVoxelAsyncWork::DoThreadedWork()
{
	DoWork();

	{
		FScopeLock Lock(&DoneSection);
		DoneEvent->Trigger();
		IsDoneCounter.Increment();
	}
}

void FVoxelAsyncWork::Abandon()
{
	FScopeLock Lock(&DoneSection);

	DoneEvent->Trigger();
	IsDoneCounter.Increment();
}

bool FVoxelAsyncWork::IsDone() const
{
	return IsDoneCounter.GetValue() > 0;
}

void FVoxelAsyncWork::WaitForCompletion()
{
	SCOPE_CYCLE_COUNTER(STAT_FVoxelAsyncWork_WaitForCompletion);

	DoneEvent->Wait();
	// Make sure IsDoneCounter had the time to increment
	FScopeLock Lock(&DoneSection);
}

///////////////////////////////////////////////////////////////////////////////

FAsyncPolygonizerWork::FAsyncPolygonizerWork(
	int LOD,
	FVoxelData* Data,
	const FIntVector& ChunkPosition,
	const FIntVector& PositionOffset,
	AVoxelWorld* World
	,bool bComputeGrass,
	const TArray<TSet<FIntVector>>& OldGrassPositionsArray,
	bool bComputeVoxelActors
	)
	: LOD(LOD)
	, Data(Data)
	, ChunkPosition(ChunkPosition)
	, PositionOffset(PositionOffset)
	, World(World)
	, bComputeGrass(bComputeGrass)
	, OldGrassPositionsArray(OldGrassPositionsArray)
	, bComputeVoxelActors(bComputeVoxelActors)
	, IsDoneCounter(0)
{
}

void FAsyncPolygonizerWork::DoWork()
{
	CONDITIONAL_SCOPE_CYCLE_COUNTER(STAT_FAsyncPolygonizerWork_DoWork, VOXEL_MULTITHREAD_STAT);

	// Mesh
	{
		CONDITIONAL_SCOPE_CYCLE_COUNTER(STAT_FAsyncPolygonizerWork_DoWork_Mesh, VOXEL_MULTITHREAD_STAT);

		TSharedPtr<FVoxelPolygonizer> Builder = MakeShareable(new FVoxelPolygonizer(LOD, Data, ChunkPosition, World ? World->GetCreateAdditionalVerticesForMaterialsTransitions() : true, World ? World->GetEnableNormals() : true));

		bool bSuccess = Builder->CreateChunk(Chunk);
		if (!bSuccess)
		{
			AsyncTask(ENamedThreads::GameThread, []() { FVoxelCrashReporter::ShowApproximationError(); });
			Chunk.Reset();
		}
		for (auto& Vertex : Chunk.VertexBuffer)
		{
			Vertex.Position += (FVector)PositionOffset;
		}
	}
	
	// Grass
	if (bComputeGrass && World)
	{
		CONDITIONAL_SCOPE_CYCLE_COUNTER(STAT_FAsyncPolygonizerWork_DoWork_Grass, VOXEL_MULTITHREAD_STAT);

		const FVoxelGrassSpawner_ThreadSafe& Config = World->GetGrassSpawner();

		const float VoxelSize = World->GetVoxelSize();
		const FVoxelWorldGeneratorInstance* Generator = World->GetWorldGenerator();
		const int32 Seed = World->GetSeed();

		int OldPositionsIndex = 0;
		for (const auto& GrassTypeID : Config.GrassTypes)
		{
			const uint8 Material = GrassTypeID.Material;
			int GrassVarietyIndex = 0;
			for (const auto& GrassVariety : GrassTypeID.GrassType.GrassVarieties)
			{
				GrassVarietyIndex++;

				TArray<FMatrix> InstanceTransforms;
				uint32 InstanceTransformsCount = 0;

				while (OldGrassPositionsArray.Num() <= OldPositionsIndex)
				{
					OldGrassPositionsArray.AddZeroed();
				}

				const auto& OldPositions = OldGrassPositionsArray[OldPositionsIndex];
				OldPositionsIndex++;

				bool bFirstTime = OldPositions.Num() == 0;
				TSet<FIntVector> NewPositions;
				// Avoid having an empty one which leads to false first time
				NewPositions.Add(FIntVector(MAX_int32, MAX_int32, MAX_int32));

				const float VoxelTriangleArea = (VoxelSize * VoxelSize) / 2;
				const float MeanGrassPerTrig = GrassVariety.GrassDensity * VoxelTriangleArea / 100000 /* 10m² in cm² */;

				for (int Index = 0; Index < Chunk.IndexBuffer.Num(); Index += 3)
				{
					int IndexA = Chunk.IndexBuffer[Index];
					int IndexB = Chunk.IndexBuffer[Index + 1];
					int IndexC = Chunk.IndexBuffer[Index + 2];

					FVoxelMaterial MatA = FVoxelMaterial(Chunk.VertexBuffer[IndexA].Color);
					FVoxelMaterial MatB = FVoxelMaterial(Chunk.VertexBuffer[IndexB].Color);
					FVoxelMaterial MatC = FVoxelMaterial(Chunk.VertexBuffer[IndexC].Color);

					if ((Material == MatA.Index1) || (Material == MatA.Index2) ||
						(Material == MatB.Index1) || (Material == MatB.Index2) ||
						(Material == MatC.Index1) || (Material == MatC.Index2))
					{
						const float AlphaA = (Material == MatA.Index1) ? (255 - MatA.Alpha) : ((Material == MatA.Index2) ? MatA.Alpha : 0);
						const float AlphaB = (Material == MatB.Index1) ? (255 - MatB.Alpha) : ((Material == MatB.Index2) ? MatB.Alpha : 0);
						const float AlphaC = (Material == MatC.Index1) ? (255 - MatC.Alpha) : ((Material == MatC.Index2) ? MatC.Alpha : 0);

						const float AlphaMean = (AlphaA + AlphaB + AlphaC) / 3.f;
						const float CurrentMeanGrassPerTrig = MeanGrassPerTrig * AlphaMean / 255.f;


						const FVector A = Chunk.VertexBuffer[IndexA].Position;
						const FVector B = Chunk.VertexBuffer[IndexB].Position;
						const FVector C = Chunk.VertexBuffer[IndexC].Position;

						const FVector Normal = (Chunk.VertexBuffer[IndexA].Normal + Chunk.VertexBuffer[IndexB].Normal + Chunk.VertexBuffer[IndexC].Normal).GetSafeNormal();

						const FVector WorldUp = Generator->GetUpVector(A.X, A.Y, A.Z).GetSafeNormal();
						const float Angle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Normal, WorldUp)));

						if (GrassVariety.MinAngleWithWorldUp <= Angle && Angle <= GrassVariety.MaxAngleWithWorldUp)
						{
							FVector X = B - A;
							FVector Y = C - A;

							const float SizeX = X.Size();
							const float SizeY = Y.Size();

							X.Normalize();
							Y.Normalize();

							// Not exact, but all we want is a random generator that depends only on position
							const FVector ExactCenter = (A + B + C) / 3 * 1000000;
							const FIntVector IntCenter = ChunkPosition + FIntVector(FMath::RoundToInt(ExactCenter.X), FMath::RoundToInt(ExactCenter.Y), FMath::RoundToInt(ExactCenter.Z));

							FRandomStream Stream((181194 + Seed * GrassVarietyIndex) * (Seed + 939232) * (IntCenter.X + Seed + 28211) * (IntCenter.Y + Seed * Seed + 2929283) * (IntCenter.Z + Seed * Seed * Seed + 9293389));

							int Count = 2 * CurrentMeanGrassPerTrig;
							if (Stream.GetFraction() < 2 * CurrentMeanGrassPerTrig - Count)
							{
								Count++;
							}

							for (float i = 0.5; i < Count; i++)
							{
								if (Stream.GetFraction() > 0.5f)
								{
									float CoordX = Stream.GetFraction() * SizeY;
									float CoordY = Stream.GetFraction() * SizeX;

									if (SizeY - CoordX * SizeY / SizeX < CoordY)
									{
										CoordX = SizeX - CoordX;
										CoordY = SizeY - CoordY;
									}

									const FVector CurrentRelativePosition = A + X * CoordX + Y * CoordY;


									// Compute scale

									FVector Scale(1.0f);
									switch (GrassVariety.Scaling)
									{
									case EGrassScaling::Uniform:
										Scale.X = GrassVariety.ScaleX.Interpolate(Stream.GetFraction());
										Scale.Y = Scale.X;
										Scale.Z = Scale.X;
										break;
									case EGrassScaling::Free:
										Scale.X = GrassVariety.ScaleX.Interpolate(Stream.GetFraction());
										Scale.Y = GrassVariety.ScaleY.Interpolate(Stream.GetFraction());
										Scale.Z = GrassVariety.ScaleZ.Interpolate(Stream.GetFraction());
										break;
									case EGrassScaling::LockXY:
										Scale.X = GrassVariety.ScaleX.Interpolate(Stream.GetFraction());
										Scale.Y = Scale.X;
										Scale.Z = GrassVariety.ScaleZ.Interpolate(FMath::RandRange(0.f, 1.f));
										break;
									default:
										check(0);
									}


									// Compute rotation

									FRotator Rotation;
									if (GrassVariety.AlignToSurface)
									{
										if (GrassVariety.RandomRotation)
										{
											Rotation = UKismetMathLibrary::MakeRotFromZX(Normal, Stream.GetFraction() * X + Stream.GetFraction() * Y);
										}
										else
										{
											Rotation = UKismetMathLibrary::MakeRotFromZX(Normal, X + Y);
										}
									}
									else
									{
										if (GrassVariety.RandomRotation)
										{
											Rotation.Yaw = Stream.FRand() * 360.f;
											Rotation.Pitch = Stream.FRand() * 360.f;
											Rotation.Roll = Stream.FRand() * 360.f;
										}
										else
										{
											Rotation = FRotator::ZeroRotator;
										}
									}

									FVector Position = (CurrentRelativePosition - (FVector)PositionOffset) * VoxelSize;
									FIntVector RoundedPosition;
									RoundedPosition.X = FMath::RoundToInt(Position.X * 100);
									RoundedPosition.Y = FMath::RoundToInt(Position.Y * 100);
									RoundedPosition.Z = FMath::RoundToInt(Position.Z * 100);

									if (bFirstTime || OldPositions.Contains(RoundedPosition))
									{
										InstanceTransforms.Add(FTransform(Rotation, VoxelSize * CurrentRelativePosition, Scale).ToMatrixWithScale());
										InstanceTransformsCount++;
										NewPositions.Add(RoundedPosition);
									}
								}
							}
						}
					}
				}

				NewGrassPositionsArray.Add(NewPositions);

				TSharedPtr<FVoxelGrassBuffer> Buffer = MakeShared<FVoxelGrassBuffer>();
				Buffer->GrassVariety = GrassVariety;

				if (InstanceTransformsCount)
				{
					Buffer->InstanceBuffer.AllocateInstances(InstanceTransformsCount, true);
					int32 InstanceIndex = 0;
					for (auto InstanceTransform : InstanceTransforms)
					{
						Buffer->InstanceBuffer.SetInstance(InstanceIndex, InstanceTransform, 0);
						InstanceIndex++;
					}

					TArray<int32> SortedInstances;
					TArray<int32> InstanceReorderTable;
					UHierarchicalInstancedStaticMeshComponent::BuildTreeAnyThread(InstanceTransforms, GrassVariety.GrassMesh->GetBounds().GetBox(), Buffer->ClusterTree, SortedInstances, InstanceReorderTable, Buffer->OutOcclusionLayerNum, /*DesiredInstancesPerLeaf*/1);

					//SORT
					// in-place sort the instances
#if ENGINE_MINOR_VERSION < 19
					const uint32 InstanceStreamSize = Buffer->InstanceBuffer.GetStride();

					FInstanceStream32 SwapBuffer;
					check(sizeof(SwapBuffer) >= InstanceStreamSize);
#endif

					for (int32 FirstUnfixedIndex = 0; FirstUnfixedIndex < InstanceTransforms.Num(); FirstUnfixedIndex++)
					{
						int32 LoadFrom = SortedInstances[FirstUnfixedIndex];
						if (LoadFrom != FirstUnfixedIndex)
						{
							check(LoadFrom > FirstUnfixedIndex);
#if ENGINE_MINOR_VERSION < 19
							FMemory::Memcpy(&SwapBuffer, Buffer->InstanceBuffer.GetInstanceWriteAddress(FirstUnfixedIndex), InstanceStreamSize);
							FMemory::Memcpy(Buffer->InstanceBuffer.GetInstanceWriteAddress(FirstUnfixedIndex), Buffer->InstanceBuffer.GetInstanceWriteAddress(LoadFrom), InstanceStreamSize);
							FMemory::Memcpy(Buffer->InstanceBuffer.GetInstanceWriteAddress(LoadFrom), &SwapBuffer, InstanceStreamSize);
#else
							Buffer->InstanceBuffer.SwapInstance(FirstUnfixedIndex, LoadFrom);
#endif

							int32 SwapGoesTo = InstanceReorderTable[FirstUnfixedIndex];
							check(SwapGoesTo > FirstUnfixedIndex);
							check(SortedInstances[SwapGoesTo] == FirstUnfixedIndex);
							SortedInstances[SwapGoesTo] = LoadFrom;
							InstanceReorderTable[LoadFrom] = SwapGoesTo;

							InstanceReorderTable[FirstUnfixedIndex] = FirstUnfixedIndex;
							SortedInstances[FirstUnfixedIndex] = FirstUnfixedIndex;
						}
					}
				}

				GrassBuffers.Add(Buffer);
			}
		}
	}

	// Actors
	if (bComputeVoxelActors && World)
	{
		CONDITIONAL_SCOPE_CYCLE_COUNTER(STAT_FAsyncPolygonizerWork_DoWork_Actors, VOXEL_MULTITHREAD_STAT);

		const FVoxelActorSpawner_ThreadSafe& ActorSpawnerConfig = World->GetActorSpawner();

		const float VoxelSize = World->GetVoxelSize();
		const int Seed = World->GetSeed();
		const FVoxelWorldGeneratorInstance* Generator = World->GetWorldGenerator();
		const float VoxelTriangleArea = (VoxelSize * VoxelSize) / 2;

		int RandomIndex = 0;
		for (const auto& GroupID : ActorSpawnerConfig.ActorConfigs)
		{
			const uint8 ID = GroupID.ID;
			for (const auto& Config : GroupID.Group.ActorConfigs)
			{
				RandomIndex++;
				const float MeanGrassPerTrig = Config.Density * VoxelTriangleArea / 10000000 /* 1000m² in cm² */;

				for (int Index = 0; Index < Chunk.IndexBuffer.Num(); Index += 3)
				{
					const int& IndexA = Chunk.IndexBuffer[Index];
					const int& IndexB = Chunk.IndexBuffer[Index + 1];
					const int& IndexC = Chunk.IndexBuffer[Index + 2];

					const FVoxelMaterial& MatA = FVoxelMaterial(Chunk.VertexBuffer[IndexA].Color);
					const FVoxelMaterial& MatB = FVoxelMaterial(Chunk.VertexBuffer[IndexB].Color);
					const FVoxelMaterial& MatC = FVoxelMaterial(Chunk.VertexBuffer[IndexC].Color);

					if (MatA.VoxelActor == ID || MatB.VoxelActor == ID || MatC.VoxelActor == ID)
					{
						const FVector& A = Chunk.VertexBuffer[IndexA].Position;
						const FVector& B = Chunk.VertexBuffer[IndexB].Position;
						const FVector& C = Chunk.VertexBuffer[IndexC].Position;

						const FVector Normal = (Chunk.VertexBuffer[IndexA].Normal + Chunk.VertexBuffer[IndexB].Normal + Chunk.VertexBuffer[IndexC].Normal).GetSafeNormal();

						const FVector WorldUp = Generator->GetUpVector(A.X, A.Y, A.Z).GetSafeNormal();
						const float Angle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Normal, WorldUp)));

						if (Config.MinAngleWithWorldUp <= Angle && Angle <= Config.MaxAngleWithWorldUp)
						{
							FVector X = B - A;
							FVector Y = C - A;

							const float SizeX = X.Size();
							const float SizeY = Y.Size();

							X /= SizeX;
							Y /= SizeY;

							// Not exact, but all we want is a random generator that depends only on position
							const FVector ExactCenter = (A + B + C) / 3 * 1000000;
							const FIntVector IntCenter = ChunkPosition + FIntVector(FMath::RoundToInt(ExactCenter.X), FMath::RoundToInt(ExactCenter.Y), FMath::RoundToInt(ExactCenter.Z));

							FRandomStream Stream((181194 + Seed * 191 + ID * 29983983) * (Seed + 939232 + RandomIndex * 192928) * (IntCenter.X + Seed + 28211) * (IntCenter.Y + Seed * Seed + 2929283) * (IntCenter.Z + Seed * Seed * Seed + 9293389));

							int Count = 2 * MeanGrassPerTrig;
							if (Stream.GetFraction() < 2 * MeanGrassPerTrig - Count)
							{
								Count++;
							}

							for (float i = 0.5; i < Count; i++)
							{
								if (Stream.GetFraction() > 0.5f)
								{
									float CoordX = Stream.GetFraction() * SizeY;
									float CoordY = Stream.GetFraction() * SizeX;

									if (SizeY - CoordX * SizeY / SizeX < CoordY)
									{
										CoordX = SizeX - CoordX;
										CoordY = SizeY - CoordY;
									}

									const FVector CurrentRelativePosition = A + X * CoordX + Y * CoordY;


									// Compute scale

									FVector Scale(1.0f);
									switch (Config.Scaling)
									{
									case EGrassScaling::Uniform:
										Scale.X = Config.ScaleX.Interpolate(Stream.GetFraction());
										Scale.Y = Scale.X;
										Scale.Z = Scale.X;
										break;
									case EGrassScaling::Free:
										Scale.X = Config.ScaleX.Interpolate(Stream.GetFraction());
										Scale.Y = Config.ScaleY.Interpolate(Stream.GetFraction());
										Scale.Z = Config.ScaleZ.Interpolate(Stream.GetFraction());
										break;
									case EGrassScaling::LockXY:
										Scale.X = Config.ScaleX.Interpolate(Stream.GetFraction());
										Scale.Y = Scale.X;
										Scale.Z = Config.ScaleZ.Interpolate(FMath::RandRange(0.f, 1.f));
										break;
									default:
										check(0);
									}


									// Compute rotation

									FRotator Rotation;
									if (Config.AlignToSurface)
									{
										if (Config.RandomRotation)
										{
											Rotation = UKismetMathLibrary::MakeRotFromZX(Normal, Stream.GetFraction() * X + Stream.GetFraction() * Y);
										}
										else
										{
											Rotation = UKismetMathLibrary::MakeRotFromZX(Normal, X + Y);
										}
									}
									else
									{
										if (Config.RandomRotation)
										{
											Rotation.Yaw = Stream.FRand() * 360.f;
											Rotation.Pitch = Stream.FRand() * 360.f;
											Rotation.Roll = Stream.FRand() * 360.f;
										}
										else
										{
											Rotation = FRotator::ZeroRotator;
										}
									}

									const FIntVector P = ChunkPosition + FIntVector(FMath::RoundToInt(CurrentRelativePosition.X), FMath::RoundToInt(CurrentRelativePosition.Y), FMath::RoundToInt(CurrentRelativePosition.Z));

									FVector Position = CurrentRelativePosition * VoxelSize + World->LocalToGlobal(ChunkPosition - PositionOffset);
									UClass* ClassToSpawn = Config.Actor;

									FIntVector IntPosition = World->GlobalToLocal(Position + FVector::UpVector * VoxelSize / 2);
									if (World->GetValue(IntPosition) > 0)
									{
										float Height = FMath::CeilToInt(AVoxelActor::GetActorHeight(ClassToSpawn)) * Scale.Z;
										FVector Dummy1;
										FIntVector Dummy2;
										if (!World->GetIntersection(IntPosition, IntPosition + FIntVector(0, 0, Height / VoxelSize), Dummy1, Dummy2))
										{
											ActorsSpawnInfo.Add(FVoxelActorSpawnInfo(ClassToSpawn, Height, Position, Rotation, Scale));
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}


int FAsyncPolygonizerWork::GetPriority() const
{
	return -LOD;
}

///////////////////////////////////////////////////////////////////////////////

FAsyncPolygonizerForTransitionsWork::FAsyncPolygonizerForTransitionsWork(int LOD, FVoxelData* Data, const FIntVector& ChunkPosition, uint8 TransitionsMask)
	: LOD(LOD)
	, Data(Data)
	, ChunkPosition(ChunkPosition)
	, TransitionsMask(TransitionsMask)
{

}

void FAsyncPolygonizerForTransitionsWork::DoWork()
{
	CONDITIONAL_SCOPE_CYCLE_COUNTER(STAT_FAsyncPolygonizerWorkForTransitions_DoWork, VOXEL_MULTITHREAD_STAT);

	TSharedPtr<FVoxelPolygonizerForTransitions> Builder = MakeShareable(new FVoxelPolygonizerForTransitions(LOD, Data, ChunkPosition));

	bool bSuccess = Builder->CreateTransitions(VertexBuffer, IndexBuffer, TransitionsMask);
	if (!bSuccess)
	{
		AsyncTask(ENamedThreads::GameThread, []() { FVoxelCrashReporter::ShowApproximationError(); });
		VertexBuffer.Reset();
		IndexBuffer.Reset();
	}
}

int FAsyncPolygonizerForTransitionsWork::GetPriority() const
{
	return -LOD;
}
