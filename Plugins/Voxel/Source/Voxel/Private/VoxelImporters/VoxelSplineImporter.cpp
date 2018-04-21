// Copyright 2018 Phyronnaz

#include "VoxelImporters/VoxelSplineImporter.h"
#include "VoxelPrivate.h"
#include "Components/CapsuleComponent.h"
#include "Components/SplineComponent.h"
#include "DrawDebugHelpers.h"
#include "VoxelUtilities.h"
#include "Engine/World.h"


AVoxelSplineImporter::AVoxelSplineImporter()
	: VoxelSize(100)
{
#if WITH_EDITOR
	auto TouchCapsule = CreateDefaultSubobject<UCapsuleComponent>(FName("Capsule"));
	TouchCapsule->InitCapsuleSize(0.1f, 0.1f);
	TouchCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TouchCapsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	RootComponent = TouchCapsule;

	PrimaryActorTick.bCanEverTick = true;
#endif
}

void AVoxelSplineImporter::ImportToAsset(UVoxelDataAsset& Asset)
{
	// Calculate bounds & max scale
	FBox Bounds(EForceInit::ForceInitToZero);
	float MaxScale = 0;
	for (auto Spline : Splines)
	{
		Bounds += Spline->Bounds.GetBox();
		FVector Min, Max;
		Spline->GetSplinePointsScale().CalcBounds(Min, Max);
		MaxScale = FMath::Max(MaxScale, Max.Y);
	}

	FVector Origin = Bounds.GetCenter();
	FVector BoxExtent = Bounds.GetExtent();
	BoxExtent += FVector::OneVector * (MaxScale + 2 * VoxelSize);

	DrawDebugBox(GetWorld(), Origin, BoxExtent, FColor::Blue, false, 10, 0, 10);

	FIntVector HalfSize =
		FIntVector(
			FMath::CeilToInt(BoxExtent.X / VoxelSize),
			FMath::CeilToInt(BoxExtent.Y / VoxelSize),
			FMath::CeilToInt(BoxExtent.Z / VoxelSize)
		);

	Asset.SetSize(HalfSize * 2, false);

	for (int X = 0; X < 2 * HalfSize.X; X++)
	{
		for (int Y = 0; Y < 2 * HalfSize.Y; Y++)
		{
			for (int Z = 0; Z < 2 * HalfSize.Z; Z++)
			{
				float NewValue = 1;

				for (auto Spline : Splines)
				{
					FVector Position = Origin + FVector(X - HalfSize.X, Y - HalfSize.Y, Z - HalfSize.Z) * VoxelSize;
					const float Key = Spline->FindInputKeyClosestToWorldLocation(Position);
					const float Distance = (Spline->GetLocationAtSplineInputKey(Key, ESplineCoordinateSpace::World) - Position).Size();
					const float ScaleSize = Spline->GetScaleAtSplineInputKey(Key).Y;

					NewValue = FMath::Min(NewValue, FMath::Clamp((Distance - ScaleSize) / VoxelSize, -2.f, 2.f) / 2.f);
				}

				Asset.SetValue(X, Y, Z, NewValue);
				Asset.SetMaterial(X, Y, Z, Material);
				Asset.SetVoxelType(X, Y, Z, FVoxelType(FVoxelUtilities::GetValueTypeFromValue(NewValue), bSetMaterial ? FVoxelUtilities::GetMaterialTypeFromValue(NewValue) : EVoxelMaterialType::IgnoreMaterial));
			}
		}
	}
}

#if WITH_EDITOR
void AVoxelSplineImporter::Tick(float DeltaTime)
{
	if (GetWorld()->WorldType == EWorldType::Editor)
	{
		if (IsSelectedInEditor())
		{
			int SphereIndex = 0;
			for (auto Spline : Splines)
			{
				auto Positions = Spline->SplineCurves.Position.Points;
				auto Scales = Spline->SplineCurves.Scale.Points;
				for (int i = 0; i < Spline->GetNumberOfSplinePoints(); i++)
				{
					if (SphereIndex >= Spheres.Num() || !Spheres[SphereIndex])
					{
						USphereComponent* Component = NewObject<USphereComponent>(this);
						Component->RegisterComponent();
						if (SphereIndex >= Spheres.Num())
						{
							Spheres.Add(Component);
						}
						else
						{
							Spheres[SphereIndex] = Component;
						}
					}
					Spheres[SphereIndex]->SetRelativeLocation(Spline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Local));

					Spheres[SphereIndex]->SetSphereRadius(Spline->GetScaleAtSplinePoint(i).Y);
					SphereIndex++;
				}
			}
			for (int i = SphereIndex; i < Spheres.Num(); i++)
			{
				if (Spheres[i])
				{
					Spheres[i]->DestroyComponent();
				}
			}
			Spheres.SetNum(SphereIndex);
		}
	}
}

bool AVoxelSplineImporter::ShouldTickIfViewportsOnly() const
{
	return true;
}

void AVoxelSplineImporter::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	for (USplineComponent*& Spline : Splines)
	{
		if (!Spline)
		{
			Spline = NewObject<USplineComponent>(this);
			Spline->OnComponentCreated();
			Spline->RegisterComponent();
			Spline->SetWorldLocation(GetActorLocation());
			Spline->RemoveSplinePoint(0);
			Spline->RemoveSplinePoint(0);

			FSplinePoint Point;
			Point.Position = FVector::RightVector * 200;
			Point.Scale = FVector(1, 100, 1);
			Spline->AddPoint(Point);

			Point.Position *= -1;
			Point.InputKey = 1;
			Spline->AddPoint(Point);
		}
	}

	for (auto Component : GetComponentsByClass(USplineComponent::StaticClass()))
	{
		USplineComponent* Spline = (USplineComponent*)Component;
		if (Spline && !Splines.Contains(Spline))
		{
			Spline->DestroyComponent();
		}
	}
}
#endif
