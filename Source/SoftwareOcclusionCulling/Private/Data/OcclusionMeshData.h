#pragma once

#include "CoreMinimal.h"
#include "OccluderMeshData.h"
#include "OcclusionMeshData.generated.h"

USTRUCT()
struct FOcclusionMeshData
{
	GENERATED_BODY()

	UPROPERTY()
	FMatrix	LocalToWorld;

	UPROPERTY()
	FOccluderMeshData Data;
	
	FPrimitiveComponentId PrimId;
};