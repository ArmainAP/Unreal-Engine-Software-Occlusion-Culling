// Copyright to Kat Code Labs, SRL. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OccluderMeshData.h"
#include "OcclusionPrimitiveProxy.generated.h"

USTRUCT()
struct FOcclusionPrimitiveProxy
{
	GENERATED_BODY()

	FPrimitiveComponentId PrimitiveComponentId;

	UPROPERTY()
	FOccluderMeshData OccluderData;

	UPROPERTY()
	FBoxSphereBounds Bounds;

	UPROPERTY()
	FMatrix LocalToWorld;

	UPROPERTY()
	bool bOccluder = true;

	UPROPERTY()
	bool bOcluded = true;
};
