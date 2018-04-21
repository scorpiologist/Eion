// Copyright 2018 Phyronnaz

#include "VoxelImporters/VoxelMeshImporter.h"

#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"

#include "Engine/StaticMesh.h"
#include "KismetProceduralMeshLibrary.h"

#include "VoxelUtilities.h"

#if !PLATFORM_ANDROID
#include "basetsd.h"

#define VOXELIZER_IMPLEMENTATION
//#define VOXELIZER_DEBUG // Only if assertions need to be checked
#include "voxelizer.h"
#endif // !PLATFORM_ANDROID

AVoxelMeshImporter::AVoxelMeshImporter()
	: MeshVoxelSize(10)
	, UpscalingFactor(2)
{
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	RootComponent = MeshComponent;
};

void AVoxelMeshImporter::ImportToAsset(UVoxelDataAsset& Asset)
{
#if !PLATFORM_ANDROID
	check(StaticMesh);

	StaticMesh->bAllowCPUAccess = true;

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FProcMeshTangent> Tangents;
	UKismetProceduralMeshLibrary::GetSectionFromStaticMesh(StaticMesh, 0, 0, Vertices, Triangles, Normals, UVs, Tangents);

	for (auto& Vertice : Vertices)
	{
		Vertice = GetTransform().TransformPosition(Vertice);
	}

	vx_mesh_t* mesh;
	vx_mesh_t* result;

	mesh = vx_mesh_alloc(Vertices.Num(), Triangles.Num());

	for (int i = 0; i < Vertices.Num(); i++)
	{
		mesh->vertices[i] = { Vertices[i].X, Vertices[i].Y, Vertices[i].Z };
		mesh->normals[i] = { Normals[i].X, Normals[i].Y, Normals[i].Z };
	}

	for (int i = 0; i < Triangles.Num(); i++)
	{
		mesh->indices[i] = Triangles[i];
	}

	// Precision factor to reduce "holes" artifact
	float precision = 0.01;

	// Run voxelization
	result = vx_voxelize(mesh, MeshVoxelSize, MeshVoxelSize, MeshVoxelSize, precision);

	int32 MinX = MAXINT32;
	int32 MinY = MAXINT32;
	int32 MinZ = MAXINT32;
	int32 MaxX = MININT32;
	int32 MaxY = MININT32;
	int32 MaxZ = MININT32;

	for (int i = 0; i < result->nvertices; i++)
	{
		vx_vertex V = result->vertices[i];

		int X = FMath::RoundToInt((V.x - MeshVoxelSize / 2.f) / (float)MeshVoxelSize);
		int Y = FMath::RoundToInt((V.y - MeshVoxelSize / 2.f) / (float)MeshVoxelSize);
		int Z = FMath::RoundToInt((V.z - MeshVoxelSize / 2.f) / (float)MeshVoxelSize);

		MinX = FMath::Min(MinX, X);
		MaxX = FMath::Max(MaxX, X);

		MinY = FMath::Min(MinY, Y);
		MaxY = FMath::Max(MaxY, Y);

		MinZ = FMath::Min(MinZ, Z);
		MaxZ = FMath::Max(MaxZ, Z);
	}

	FIntVector Size;
	Size.X = MaxX - MinX + 1;
	Size.Y = MaxY - MinY + 1;
	Size.Z = MaxZ - MinZ + 1;

	TArray<bool> IsInside;
	IsInside.SetNumZeroed(Size.X * Size.Y * Size.Z);

	for (int i = 0; i < result->nvertices; i++)
	{
		vx_vertex V = result->vertices[i];

		int X = FMath::RoundToInt((V.x - MeshVoxelSize / 2.f) / (float)MeshVoxelSize);
		int Y = FMath::RoundToInt((V.y - MeshVoxelSize / 2.f) / (float)MeshVoxelSize);
		int Z = FMath::RoundToInt((V.z - MeshVoxelSize / 2.f) / (float)MeshVoxelSize);

		check(0 <= X - MinX && X - MinX < Size.X);
		check(0 <= Y - MinY && Y - MinY < Size.Y);
		check(0 <= Z - MinZ && Z - MinZ < Size.Z);
		IsInside[(X - MinX) + Size.X * (Y - MinY) + Size.X * Size.Y * (Z - MinZ)] = true;
	}

	vx_mesh_free(result);
	vx_mesh_free(mesh);

	for (auto ActorInsideTheMesh : ActorsInsideTheMesh)
	{
		if (!ActorInsideTheMesh)
		{
			continue;
		}

		TArray<FIntVector> Stack;
		FVector PointInsideTheMesh = ActorInsideTheMesh->GetActorLocation() / MeshVoxelSize;
		Stack.Add(FIntVector(PointInsideTheMesh.X - MinX, PointInsideTheMesh.Y - MinY, PointInsideTheMesh.Z - MinZ));

		while (Stack.Num() > 0)
		{
			FIntVector V = Stack.Pop(false);			

			if (0 <= V.X && V.X < Size.X && 0 <= V.Y && V.Y < Size.Y && 0 <= V.Z && V.Z < Size.Z)
			{
				int Index = V.X + Size.X * V.Y + Size.X * Size.Y * V.Z;

				if (!IsInside[Index])
				{
					IsInside[Index] = true;

					const TArray<FIntVector> Positions = {
						FIntVector(-1, 0, 0),
						FIntVector(1, 0, 0),
						FIntVector(0, -1, 0),
						FIntVector(0, 1, 0),
						FIntVector(0, 0, -1),
						FIntVector(0, 0, 1)
					};

					for (auto P : Positions)
					{
						Stack.Add(V + P);
					}
				}
			}
		}
	}

	if (bDrawPoints)
	{
		for (int X = 0; X < Size.X; X++)
		{
			for (int Y = 0; Y < Size.Y; Y++)
			{
				for (int Z = 0; Z < Size.Z; Z++)
				{
					int Index = X + Size.X * Y + Size.X * Size.Y * Z;
					if (IsInside[Index])
					{
						DrawDebugPoint(GetWorld(), FVector(X + MinX, Y + MinY, Z + MinZ) * MeshVoxelSize, 7, FColor::Red, true, 10);
					}
				}
			}
		}
	}

	FIntVector AssetSize =
		FIntVector(
			FMath::CeilToInt(Size.X / (float)UpscalingFactor) + 1,
			FMath::CeilToInt(Size.Y / (float)UpscalingFactor) + 1,
			FMath::CeilToInt(Size.Z / (float)UpscalingFactor) + 1
		);

	Asset.SetSize(AssetSize, false);

	for (int X = 0; X < AssetSize.X; X++)
	{
		for (int Y = 0; Y < AssetSize.Y; Y++)
		{
			for (int Z = 0; Z < AssetSize.Z; Z++)
			{
				int VoxelInsideCount = 0;
				int TotalVoxelCount = 0;
				for (int I = -UpscalingFactor / 2; I < UpscalingFactor / 2; I++)
				{
					for (int J = -UpscalingFactor / 2; J < UpscalingFactor / 2; J++)
					{
						for (int K = -UpscalingFactor / 2; K < UpscalingFactor / 2; K++)
						{
							int A = UpscalingFactor * X + I;
							int B = UpscalingFactor * Y + J;
							int C = UpscalingFactor * Z + K;
							if (0 <= A && A < Size.X && 0 <= B && B < Size.Y && 0 <= C && C < Size.Z)
							{
								int Index = A + Size.X * B + Size.X * Size.Y * C;
								TotalVoxelCount++;
								if (IsInside[Index])
								{
									VoxelInsideCount++;
								}
							}
						}
					}
				}
				float NewValue = 1 - 2 * (VoxelInsideCount / (float)TotalVoxelCount);
				Asset.SetValue(X, Y, Z, NewValue);
				Asset.SetMaterial(X, Y, Z, Material);
				Asset.SetVoxelType(X, Y, Z, FVoxelUtilities::GetVoxelTypeFromValue(NewValue));
			}
		}
	}
#endif // PLATFORM_ANDROID
}

#if WITH_EDITOR
void AVoxelMeshImporter::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	UpscalingFactor = (UpscalingFactor / 2) * 2;

	MeshComponent->SetStaticMesh(StaticMesh);
}
#endif
