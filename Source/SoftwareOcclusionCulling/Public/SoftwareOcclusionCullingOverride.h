// Copyright to Kat Code Labs, SRL. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/DefaultOcclusionSettings.h"
#include "SoftwareOcclusionCullingOverride.generated.h"


UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOFTWAREOCCLUSIONCULLING_API USoftwareOcclusionCullingOverride : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USoftwareOcclusionCullingOverride();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FOcclusionSettings OcclusionSettings;
};
