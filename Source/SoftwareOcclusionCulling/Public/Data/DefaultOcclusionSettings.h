// Copyright to Kat Code Labs, SRL. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "DefaultOcclusionSettings.generated.h"

USTRUCT(Blueprintable, BlueprintType)
struct FOcclusionSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseAsOccluder = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanBeOcluded = true;
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

/**
 * 
 */
UCLASS(Config = Engine, DefaultConfig)
class SOFTWAREOCCLUSIONCULLING_API UDefaultOcclusionSettings : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FOcclusionSettings DefaultOcclusionSettings;
};
