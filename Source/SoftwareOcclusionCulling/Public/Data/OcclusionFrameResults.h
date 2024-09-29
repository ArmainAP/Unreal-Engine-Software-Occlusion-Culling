#pragma once

#include "CoreMinimal.h"
#include "OcclusionFrameResults.generated.h"

static constexpr int32 BIN_WIDTH = 64;
static constexpr int32 BIN_NUM = 6;
static constexpr int32 FRAMEBUFFER_WIDTH = BIN_WIDTH * BIN_NUM;
static constexpr int32 FRAMEBUFFER_HEIGHT = 256;


USTRUCT()
struct FFramebufferBin
{
	GENERATED_BODY()
	uint64 Data[FRAMEBUFFER_HEIGHT];
};

USTRUCT()
struct FOcclusionFrameResults
{
	GENERATED_BODY()

	UPROPERTY()
	FFramebufferBin	Bins[BIN_NUM];

	TMap<FPrimitiveComponentId, bool> VisibilityMap;
};