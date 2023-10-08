// Copyright to Kat Code Labs, SRL. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IXRTrackingSystem.h"
#include "Kismet/GameplayStatics.h"

struct FOcclusionViewInfo
{
	FOcclusionViewInfo() = default;
	FOcclusionViewInfo(const APlayerCameraManager* PlayerCameraManager)
	{
		const FMinimalViewInfo MinimalView = PlayerCameraManager->GetCameraCacheView();
		FMatrix ViewProjectionMatrix;
		UGameplayStatics::GetViewProjectionMatrix(MinimalView, ViewMatrix, ProjectionMatrix, ViewProjectionMatrix);
		Origin = MinimalView.Location;
		if (ShouldUseStereoRendering())
		{
			ProjectionMatrix = GEngine->StereoRenderingDevice->GetStereoProjectionMatrix(0);	
		}
	}

	static bool ShouldUseStereoRendering()
	{
#if WITH_EDITOR
		if (GEditor && !GEditor->IsVRPreviewActive())
		{
			return false;
		}
#endif
		
		return GEngine->StereoRenderingDevice.IsValid() && GEngine->StereoRenderingDevice->IsStereoEnabled() &&
			GEngine->XRSystem.IsValid() && GEngine->XRSystem->GetHMDDevice();
	}

	FVector Origin;
	FMatrix ViewMatrix;
	FMatrix ProjectionMatrix;
};