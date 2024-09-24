// Copyright to Kat Code Labs, SRL. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/OcclusionPrimitiveContext.h"
#include "Data/DefaultOcclusionSettings.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Async/TaskGraphInterfaces.h"
#include "OcclusionCullingSubsystem.generated.h"

struct FOcclusionFrameResults;

/**
 * 
 */
UCLASS(BlueprintType)
class SOFTWAREOCCLUSIONCULLING_API UOcclusionCullingSubsystem : public ULocalPlayerSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	UOcclusionCullingSubsystem();
	~UOcclusionCullingSubsystem();
	UOcclusionCullingSubsystem(FVTableHelper& Helper);

	virtual void Deinitialize() override;
	virtual void PlayerControllerChanged(APlayerController* NewPlayerController) override;

	virtual TStatId GetStatId() const override;
	virtual bool IsAllowedToTick() const override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void DebugDrawToCanvas(const UCanvas* Canvas, int32 InX, int32 InY);

	UFUNCTION(BlueprintCallable)
	bool RegisterOcclusionSettings(UStaticMeshComponent* StaticMeshComponent,
	                               const FOcclusionSettings& OcclusionSettings);

	UFUNCTION(BlueprintCallable)
	void UnregisterOcclusionSettings(const UStaticMeshComponent* StaticMeshComponent);

private:
	void PopulateScene(TArray<FOcclusionPrimitiveProxy*>& Scene);
	int32 ProcessScene(const TArray<FOcclusionPrimitiveProxy*>& Scene);
	int32 ApplyResults(const TArray<FOcclusionPrimitiveProxy*> Scene, const FOcclusionFrameResults& Results);
	void FlushSceneProcessing();

	UPROPERTY()
	APlayerCameraManager* PlayerCameraManager;
	TMap<FPrimitiveComponentId, UOcclusionPrimitiveContext*> PrimitiveContextMap;

	FGraphEventRef TaskRef;
    TUniquePtr<FOcclusionFrameResults> Available;
    TUniquePtr<FOcclusionFrameResults> Processing;
};
