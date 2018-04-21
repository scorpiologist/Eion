// Copyright 2017 Phyronnaz

#include "CubicVoxelRender.h"
#include "VoxelPrivate.h"
#include "VoxelWorld.h"
#include "VoxelData.h"
#include "VoxelChunkOctree.h"
#include "VoxelInvokerComponent.h"
#include "VoxelProceduralMeshComponent.h"
#include "QueuedThreadPool.h"
#include "DrawDebugHelpers.h"
#include "VoxelGlobals.h"
#include "VoxelThreadPool.h"
#include "VoxelActor.h"

DECLARE_CYCLE_STAT(TEXT("FCubicVoxelRender::UpdateChunks"), STAT_CubicVoxelRender_UpdateChunks, STATGROUP_Voxel);

FVoxelCubicChunk::FVoxelCubicChunk(const FIntVector& Position, FCubicVoxelRender* Render)
	: Position(Position)
	, Render(Render)
	, Mesh(nullptr)
{

}

void FVoxelCubicChunk::Tick()
{
	if (Task.IsValid() && Task->IsDone())
	{
		FVoxelProcMeshSection& Section = Task->Section;

		// Mesh
		{
			if (Section.ProcVertexBuffer.Num() > 0)
			{
				// Create only if needed

				if (!Mesh)
				{
					Mesh = Render->GetMesh(Position);
				}
				Mesh->SetProcMeshSection(0, Section);
			}
			else
			{
				if (Mesh)
				{
					Mesh->SetProcMeshSection(0, FVoxelProcMeshSection());
				}
			}
		}


		// Grass
		{
			OldGrassPositionsArray = Task->NewGrassPositionsArray;
			int BufferIndex = 0;
			for (auto& Buffer : Task->GrassBuffers)
			{
				BufferIndex++;

				while (GrassMeshes.Num() <= BufferIndex)
				{
					GrassMeshes.AddZeroed();
				}

				auto& NewGrass = GrassMeshes[BufferIndex];
				
#if ENGINE_MINOR_VERSION < 19
				if (Buffer->InstanceBuffer.NumInstances())
#else
				if (Buffer->InstanceBuffer.GetNumInstances())
#endif
				{
					if (!NewGrass)
					{
						NewGrass = Render->GetNewGrass(Position);
						FVoxelGrassUtilities::InitGrass(NewGrass, Buffer);
					}

					FVoxelGrassUtilities::SetNewPositions(NewGrass, Buffer);
				}
				else if (NewGrass)
				{
					// If all the grass has been removed
					NewGrass->ClearInstances();
				}
			}
		}

		// Actors
		if (Task->bComputeVoxelActors)
		{
			Render->World->NotifyActorsAreCreated(Position);

			for (auto& ActorInfo : Task->ActorsSpawnInfo)
			{
				//DrawDebugLine(Render->World->GetWorld(), ActorInfo.Position, ActorInfo.Position + FVector::UpVector * ActorInfo.Height, FColor::Red, true, 100.f, 0, 10);
				AVoxelActor* Actor = Render->World->GetWorld()->SpawnActor<AVoxelActor>(ActorInfo.ClassToSpawn);
				Actor->SetActorLocation(ActorInfo.Position);
				Actor->SetActorRotation(FQuat(ActorInfo.Rotation));
				Actor->SetActorScale3D(ActorInfo.Scale);
				Render->World->AddActor(Actor);
			}
		}

		Task.Reset();
	}
}


void FVoxelCubicChunk::Update()
{
	if (!Task.IsValid())
	{
		Task = MakeShared<FAsyncCubicPolygonizerWork>(
			Render->World->GetData(),
			Position, FIntVector(0, 0, 0),
			Render->World
			, true,
			OldGrassPositionsArray,
			!Render->World->HasActorsBeenCreated(Position)
			);
		Render->MeshThreadPool->AddQueuedWork(Task.Get());
	}
}

void FVoxelCubicChunk::DestroyGrass()
{
	for (auto& Grass : GrassMeshes)
	{
		if (Grass)
		{
			Render->RemoveGrass(Grass);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

FCubicVoxelRender::FCubicVoxelRender(AVoxelWorld* World, AActor* ChunksOwner)
	: IVoxelRender(World, ChunksOwner)
	, TimeSinceUpdate(0)
	, MeshThreadPool(new FVoxelQueuedThreadPool())
{
	MeshThreadPool->Create(World->GetMeshThreadCount(), 1024 * 1024);
}


FCubicVoxelRender::~FCubicVoxelRender()
{
	MeshThreadPool->Destroy();
	delete MeshThreadPool;
}

void FCubicVoxelRender::Tick(float DeltaTime)
{
	{
		TimeSinceUpdate += DeltaTime;
		if (TimeSinceUpdate > 1)
		{
			TimeSinceUpdate = 0;
			UpdateChunks();
		}
	}

	for (auto& Chunk : ChunksArray)
	{
		Chunk->Tick();
	}

	{
		for (auto& Task : TasksToDelete)
		{
			if (Task->IsDone())
			{
				Task.Reset();
			}
		}
		TasksToDelete.RemoveAll([&](TSharedPtr<FAsyncCubicPolygonizerWork>& Ptr) { return !Ptr.IsValid(); });
	}
}

void FCubicVoxelRender::AddInvoker(TWeakObjectPtr<UVoxelInvokerComponent> Invoker)
{
	Invokers.Add(Invoker);
}

void FCubicVoxelRender::UpdateBoxInternal(const FIntBox& Box)
{
	const FIntVector Min(
		FMath::FloorToInt((float)Box.Min.X / CHUNK_SIZE) * CHUNK_SIZE,
		FMath::FloorToInt((float)Box.Min.Y / CHUNK_SIZE) * CHUNK_SIZE,
		FMath::FloorToInt((float)Box.Min.Z / CHUNK_SIZE) * CHUNK_SIZE);
	const FIntVector Max(
		FMath::CeilToInt((float)(Box.Max.X + 1) / CHUNK_SIZE) * CHUNK_SIZE,
		FMath::CeilToInt((float)(Box.Max.Y + 1) / CHUNK_SIZE) * CHUNK_SIZE,
		FMath::CeilToInt((float)(Box.Max.Z + 1) / CHUNK_SIZE) * CHUNK_SIZE);

	for (int X = Min.X; X < Max.X; X += CHUNK_SIZE)
	{
		for (int Y = Min.Y; Y < Max.Y; Y += CHUNK_SIZE)
		{
			for (int Z = Min.Z; Z < Max.Z; Z += CHUNK_SIZE)
			{
				const FIntVector Position = FIntVector(X, Y, Z);

				auto Chunk = Chunks.Find(Position);

				if (Chunk)
				{
					(*Chunk)->Update();
				}
			}
		}
	}
}

uint8 FCubicVoxelRender::GetLODAtPosition(const FIntVector& Position) const
{
	return 0;
}


UVoxelProceduralMeshComponent* FCubicVoxelRender::GetMesh(const FIntVector& Position)
{
	UVoxelProceduralMeshComponent* NewChunk;
	if (InactiveChunks.Num() > 0)
	{
		NewChunk = InactiveChunks.Pop();
	}
	else
	{
		NewChunk = NewObject<UVoxelProceduralMeshComponent>(ChunksOwner, NAME_None, RF_Transient);
		NewChunk->bUseAsyncCooking = true;
		NewChunk->SetupAttachment(ChunksOwner->GetRootComponent(), NAME_None);
		NewChunk->RegisterComponent();
		NewChunk->SetMaterial(0, World->GetVoxelMaterial());
		NewChunk->SetWorldScale3D(FVector::OneVector * World->GetVoxelSize());
	}

	NewChunk->SetWorldLocation(World->LocalToGlobal(Position) + ChunksOwner->GetActorLocation() - World->GetActorLocation());

	return NewChunk;
}


void FCubicVoxelRender::RemoveMesh(UVoxelProceduralMeshComponent* Mesh)
{
	Mesh->SetProcMeshSection(0, FVoxelProcMeshSection());
	InactiveChunks.Add(Mesh);
}

void FCubicVoxelRender::RemoveGrass(UHierarchicalInstancedStaticMeshComponent* Grass)
{
	Grass->ClearInstances();
	InactiveGrasses.Add(Grass);
}


UHierarchicalInstancedStaticMeshComponent* FCubicVoxelRender::GetNewGrass(const FIntVector& Position)
{
	UHierarchicalInstancedStaticMeshComponent* NewGrass;
	if (InactiveGrasses.Num() > 0)
	{
		// Create only if needed
		NewGrass = InactiveGrasses.Pop();
	}
	else
	{
		NewGrass = NewObject<UHierarchicalInstancedStaticMeshComponent>(ChunksOwner, NAME_None, RF_Transient);
		NewGrass->SetupAttachment(ChunksOwner->GetRootComponent(), NAME_None);

		NewGrass->OnComponentCreated();
		NewGrass->RegisterComponent();
		if (NewGrass->bWantsInitializeComponent) NewGrass->InitializeComponent();

		NewGrass->Mobility = EComponentMobility::Movable;
		NewGrass->bCastStaticShadow = false;
	}

	NewGrass->SetWorldLocation(World->LocalToGlobal(Position) + ChunksOwner->GetActorLocation() - World->GetActorLocation());

	return NewGrass;
}

void FCubicVoxelRender::UpdateChunks()
{
	SCOPE_CYCLE_COUNTER(STAT_CubicVoxelRender_UpdateChunks);

	for (auto& Chunk : ChunksArray)
	{
		Chunk->bIsValid = false;
	}

	Invokers.RemoveAll([](auto Ptr) { return !Ptr.IsValid(); });
	for (const auto& Invoker : Invokers)
	{
		check(Invoker.IsValid());
		if (Invoker->UseForRender())
		{
			const FIntBox Bounds = Invoker->GetCameraBounds(World);

			const FIntVector Min(
				FMath::FloorToInt((float)Bounds.Min.X / CHUNK_SIZE) * CHUNK_SIZE,
				FMath::FloorToInt((float)Bounds.Min.Y / CHUNK_SIZE) * CHUNK_SIZE,
				FMath::FloorToInt((float)Bounds.Min.Z / CHUNK_SIZE) * CHUNK_SIZE);
			const FIntVector Max(
				FMath::CeilToInt((float)(Bounds.Max.X + 1) / CHUNK_SIZE) * CHUNK_SIZE,
				FMath::CeilToInt((float)(Bounds.Max.Y + 1) / CHUNK_SIZE) * CHUNK_SIZE,
				FMath::CeilToInt((float)(Bounds.Max.Z + 1) / CHUNK_SIZE) * CHUNK_SIZE);

			for (int X = Min.X; X <= Max.X; X += CHUNK_SIZE)
			{
				for (int Y = Min.Y; Y <= Max.Y; Y += CHUNK_SIZE)
				{
					for (int Z = Min.Z ; Z <= Max.Z ; Z += CHUNK_SIZE)
					{
						check(X % CHUNK_SIZE == 0);
						check(Y % CHUNK_SIZE == 0);
						check(Z % CHUNK_SIZE == 0);

						const FIntVector CurrentPosition = FIntVector(X, Y, Z);
						
						auto Chunk = Chunks.Find(CurrentPosition);
						if (!Chunk)
						{
							TSharedRef<FVoxelCubicChunk> NewChunk = MakeShared<FVoxelCubicChunk>(CurrentPosition, this);
							NewChunk->Update();
							Chunks.Add(CurrentPosition, NewChunk);
						}
						else
						{
							(*Chunk)->bIsValid = true;
						}
					}
				}
			}
		}
	}

	for (auto& Chunk : ChunksArray)
	{
		if (!Chunk->bIsValid)
		{
			if (Chunk->Mesh)
			{
				RemoveMesh(Chunk->Mesh);
			}

			Chunk->DestroyGrass();

			if (Chunk->Task.IsValid())
			{
				if (!MeshThreadPool->RetractQueuedWork(Chunk->Task.Get()))
				{
					TasksToDelete.Add(Chunk->Task);
				}
			}

			Chunks.Remove(Chunk->Position);
		}
	}
	
	// Rebuild chunks array
	Chunks.GenerateValueArray(ChunksArray);
}