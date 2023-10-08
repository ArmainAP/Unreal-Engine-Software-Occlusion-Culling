// Copyright to Kat Code Labs, SRL. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "OcclusionSettings.generated.h"

/**
 * 
 */
UCLASS(Config = Engine, DefaultConfig)
class SOFTWAREOCCLUSIONCULLING_API UOcclusionSettings : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseAsOccluder = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanBeOccludee = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowBoundsUpdate = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOccluderIsScaledUnitCube = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bOccluderIsScaledUnitCube"))
	FVector UnitCubeScale = FVector::OneVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseCustomBounds = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bUseCustomBounds"))
	FVector CustomBounds = FVector::OneVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bUseCustomBounds"))
	FVector CustomBoundsOffset = FVector::ZeroVector;
};
