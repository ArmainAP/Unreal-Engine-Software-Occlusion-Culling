// Copyright to Kat Code Labs, SRL. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/OcclusionPrimitiveContext.h"
#include "Data/SoftwareOcclusionSettings.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Async/TaskGraphInterfaces.h"
#include "Data/OcclusionFrameResults.h"
#include "Data/OcclusionSceneData.h"
#include "Data/OcclusionViewInfo.h"
#include "OcclusionCullingSubsystem.generated.h"

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

	virtual void PlayerControllerChanged(APlayerController* NewPlayerController) override;
	virtual void Deinitialize() override;

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
	void PopulateScene(TArray<FOcclusionPrimitiveProxy>& Scene);
	int32 ProcessScene(const TArray<FOcclusionPrimitiveProxy>& Scene);
	FOcclusionSceneData CollectSceneData(const TArray<FOcclusionPrimitiveProxy>& Scene, FOcclusionViewInfo View);
	int32 ApplyResults(const TArray<FOcclusionPrimitiveProxy> Scene);
	void FlushSceneProcessing();

	UPROPERTY()
	APlayerCameraManager* PlayerCameraManager;

	UPROPERTY()
	TMap<uint32, UOcclusionPrimitiveContext*> PrimitiveContextMap;

	UPROPERTY()
    FOcclusionFrameResults LastFrameResults;

	UPROPERTY()
	FOcclusionFrameResults FrameResults;

	FGraphEventRef TaskRef;
};
