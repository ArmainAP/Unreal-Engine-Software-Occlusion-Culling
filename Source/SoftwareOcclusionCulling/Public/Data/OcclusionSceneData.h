#pragma once

#include "CoreMinimal.h"
#include "Data/OcclusionMeshData.h"
#include "OcclusionSceneData.generated.h"

USTRUCT()
struct FOcclusionSceneData
{
	GENERATED_BODY()

	UPROPERTY()
	FMatrix ViewProj;

	UPROPERTY()
	TArray<FVector> OccludeeBoxMinMax;

	TArray<FPrimitiveComponentId> OccludeeBoxPrimId;

	UPROPERTY()
	TArray<FOcclusionMeshData> OccluderData;

	UPROPERTY()
	int32 NumOccluderTriangles = 0;
};
