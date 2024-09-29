// Copyright to Kat Code Labs, SRL. All Rights Reserved.

#include "Data/OcclusionPrimitiveContext.h"
#include "DrawDebugHelpers.h"

void UOcclusionPrimitiveContext::Setup(UStaticMeshComponent* InStaticMeshComponent,
                                    const FOcclusionSettings& NewOcclusionSettings)
{
	SetOcclusionSettings(NewOcclusionSettings);
	SetMesh(InStaticMeshComponent);
}

void UOcclusionPrimitiveContext::SetMesh(UStaticMeshComponent* InStaticMeshComponent)
{
	StaticMeshComponent = InStaticMeshComponent;
	PrimitiveProxy.PrimitiveComponentId = StaticMeshComponent->GetPrimitiveSceneId();
	if(OcclusionSettings.bUseAsOccluder)
	{
		PrimitiveProxy.OccluderData = FOccluderMeshData(StaticMeshComponent->GetStaticMesh());
	}
	UpdateBoundsInternal();
}

bool UOcclusionPrimitiveContext::PerformFrustumCull(const APlayerCameraManager* PlayerCameraManager) const
{
	if(!IsValid(StaticMeshComponent))
	{
		return false;
	}

	// A CachedMaxDrawDistance of 0 indicates that the primitive should not be culled by distance.
	if (StaticMeshComponent->CachedMaxDrawDistance == 0)
	{
		return false;
	}

	// Skip objects where the bounds center is within the draw distance
	const float Distance = FVector::Distance(PlayerCameraManager->GetCameraLocation(), PrimitiveProxy.Bounds.Origin);
	if(FMath::IsWithin(Distance, StaticMeshComponent->MinDrawDistance, StaticMeshComponent->LDMaxDrawDistance))
	{
		return false;
	}
	
	// Skip objects in front of the player
	const FVector CameraForward = PlayerCameraManager->GetActorForwardVector();
	const FVector DirToOccluder = (PrimitiveProxy.Bounds.Origin - PlayerCameraManager->GetCameraLocation()).GetSafeNormal();
	if (CameraForward.Dot(DirToOccluder) > 0.0f)
	{
		return false;
	}

	StaticMeshComponent->SetHiddenInGame(true);
	return true;
}

void UOcclusionPrimitiveContext::UpdateBoundsInternal()
{
	if(!IsValid(StaticMeshComponent))
	{
		return;
	}
		
	const FMatrix NewLocalToWorld = StaticMeshComponent->GetComponentTransform().ToMatrixWithScale();

	// Store occlusion bounds.
	FBoxSphereBounds OcclusionBounds = StaticMeshComponent->Bounds;
	if (OcclusionSettings.bUseCustomBounds)
	{
		const FVector HalfExtent = OcclusionSettings.CustomBounds * 0.5f;
		const FVector BoxPoint = HalfExtent - -HalfExtent;
		OcclusionBounds = FBoxSphereBounds(OcclusionSettings.CustomBoundsOffset, BoxPoint, BoxPoint.Size()).TransformBy(NewLocalToWorld);
	}
		
	/** Factor by which to grow occlusion tests **/
	constexpr float OcclusionSlop = 1.0f;
	OcclusionBounds.BoxExtent.X = OcclusionBounds.BoxExtent.X + OcclusionSlop;
	OcclusionBounds.BoxExtent.Y = OcclusionBounds.BoxExtent.Y + OcclusionSlop;
	OcclusionBounds.BoxExtent.Z = OcclusionBounds.BoxExtent.Z + OcclusionSlop;
	OcclusionBounds.SphereRadius = OcclusionBounds.SphereRadius + OcclusionSlop;

	PrimitiveProxy.Bounds = OcclusionBounds;
	PrimitiveProxy.LocalToWorld = NewLocalToWorld;
	
	if (OcclusionSettings.bOccluderIsScaledUnitCube)
	{
		PrimitiveProxy.LocalToWorld = StaticMeshComponent->GetComponentTransform().ToMatrixNoScale();
		PrimitiveProxy.LocalToWorld.SetOrigin(PrimitiveProxy.Bounds.Origin);
		PrimitiveProxy.LocalToWorld = FScaleMatrix::Make(OcclusionSettings.UnitCubeScale) * PrimitiveProxy.LocalToWorld;
	}

	const bool bHasHugeBounds = PrimitiveProxy.Bounds.SphereRadius > HALF_WORLD_MAX / 2.0f;
	PrimitiveProxy.bOccluder = !bHasHugeBounds && OcclusionSettings.bUseAsOccluder;
	PrimitiveProxy.bOcluded = !bHasHugeBounds && OcclusionSettings.bCanBeOcluded;
}

bool UOcclusionPrimitiveContext::ShouldUpdateBounds() const
{
	if(!IsValid(StaticMeshComponent))
	{
		return false;
	}
	return OcclusionSettings.bAllowBoundsUpdate && StaticMeshComponent->Mobility == EComponentMobility::Movable;
}

void UOcclusionPrimitiveContext::SetHiddenInGame(const bool bHidden) const
{
	if(!IsValid(StaticMeshComponent))
	{
		return;
	}
		
	// TODO: A developer callback would be a nice additional to the override component.
	StaticMeshComponent->SetHiddenInGame(bHidden);
}

void UOcclusionPrimitiveContext::DebugBounds() const
{
	// Check if StaticMeshComponent is valid
	if (!IsValid(StaticMeshComponent))
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugBounds: StaticMeshComponent is null."));
		return;
	}

	const UWorld* World = StaticMeshComponent->GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugBounds: World is null."));
		return;
	}
	
	// Occluder && Occluded
	FColor BoundsColor = FColor::Red;

	// Only Occluder
	if (OcclusionSettings.bUseAsOccluder && !OcclusionSettings.bCanBeOcluded)
	{
		BoundsColor = FColor::Green;
	}

	// Only Occluded
	if (!OcclusionSettings.bUseAsOccluder && OcclusionSettings.bCanBeOcluded)
	{
		BoundsColor = FColor::Blue;
	}

	// Neither
	if (!OcclusionSettings.bUseAsOccluder && !OcclusionSettings.bCanBeOcluded)
	{
		BoundsColor = FColor::Yellow;
	}

	DrawDebugBox(World, PrimitiveProxy.Bounds.Origin, PrimitiveProxy.Bounds.BoxExtent, FQuat::Identity, BoundsColor, false);
}
