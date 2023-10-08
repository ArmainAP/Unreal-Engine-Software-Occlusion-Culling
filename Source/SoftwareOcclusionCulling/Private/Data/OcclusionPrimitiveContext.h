// Copyright to Kat Code Labs, SRL. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OccluderMeshData.h"
#include "OcclusionPrimitiveProxy.h"
#include "Data/DefaultOcclusionSettings.h"
#include "OcclusionPrimitiveContext.generated.h"

UCLASS()
class UOcclusionPrimitiveContext : public UObject
{
	GENERATED_BODY()

public:
	UOcclusionPrimitiveContext()=default;
	void Setup(UStaticMeshComponent* InStaticMeshComponent, const FOcclusionSettings& NewOcclusionSettings);
	void SetMesh(UStaticMeshComponent* InStaticMeshComponent);
	bool PerformFrustumCull(const APlayerCameraManager* PlayerCameraManager) const;
	bool ShouldUpdateBounds() const;
	void SetHiddenInGame(const bool bHidden) const;
	void DebugBounds() const;

	FORCEINLINE void SetOcclusionSettings(const FOcclusionSettings& NewOcclusionSettings)
	{
		OcclusionSettings = NewOcclusionSettings;
	}

	FORCEINLINE void UpdateBounds()
	{
		if(ShouldUpdateBounds())
		{
			UpdateBoundsInternal();
		}
	}

	FORCEINLINE FOcclusionPrimitiveProxy* GetProxy()
	{
		return &PrimitiveProxy;
	}

private:
	void UpdateBoundsInternal();
	TWeakObjectPtr<UStaticMeshComponent> StaticMeshComponent;
	FOcclusionSettings OcclusionSettings;

	FOcclusionPrimitiveProxy PrimitiveProxy;
};