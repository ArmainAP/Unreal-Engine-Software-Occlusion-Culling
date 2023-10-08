// Copyright to Kat Code Labs, SRL. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OccluderMeshData.h"

struct FOcclusionPrimitiveProxy
{
	FPrimitiveComponentId PrimitiveComponentId;
	TUniquePtr<FOccluderMeshData> OccluderData = nullptr;
	FBoxSphereBounds Bounds;
	FMatrix LocalToWorld;
	bool bOccluder = true;
	bool bOcluded = true;
};
