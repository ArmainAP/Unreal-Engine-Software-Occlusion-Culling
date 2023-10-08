// Copyright to Kat Code Labs, SRL. All Rights Reserved.

#include "OcclusionCullingSubsystem.h"
#include "CanvasTypes.h"
#include "Data/OcclusionPrimitiveProxy.h"
#include "Data/OcclusionViewInfo.h"
#include "Engine/Canvas.h"
#include "Legacy//SceneSoftwareOcclusion.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

static bool CVarEnableSoftwareOcclusionCulling = true;
static FAutoConsoleVariableRef CVarEnableSoftwareOcclusionCullingRef(
	TEXT("r.SoftwareOcclusionCulling.Enable"),
	CVarEnableSoftwareOcclusionCulling,
	TEXT("Enable/Disable Software Occlusion Culling at runtime"),
	ECVF_Cheat
);

static bool CVarVisualizeSoftwareOcclusionCullingBounds = false;
static FAutoConsoleVariableRef CVarVisualizeBoundsRef(
	TEXT("r.SoftwareOcclusionCulling.VisualizeBounds"),
	CVarVisualizeSoftwareOcclusionCullingBounds,
	TEXT("Visualize Software Occlusion Culling bounds"),
	ECVF_Cheat
);

void UOcclusionCullingSubsystem::Deinitialize()
{
	Super::Deinitialize();
	
	FlushSceneProcessing();
}

void UOcclusionCullingSubsystem::PlayerControllerChanged(APlayerController* NewPlayerController)
{
	Super::PlayerControllerChanged(NewPlayerController);

	PlayerCameraManager = NewPlayerController->PlayerCameraManager;
}

TStatId UOcclusionCullingSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UOcclusionCullingEngineSubsystem, STATGROUP_Tickables);
}

bool UOcclusionCullingSubsystem::IsAllowedToTick() const
{
#if WITH_EDITOR
	if (GEditor && GEditor->IsSimulatingInEditor())
	{
		return false;
	}
#endif

	if(!IsValid(PlayerCameraManager))
	{
		return false;
	}
	
	return CVarEnableSoftwareOcclusionCulling;
}

void UOcclusionCullingSubsystem::Tick(float DeltaTime)
{
	TArray<FOcclusionPrimitiveProxy*> Scene;
	PopulateScene(Scene);
	ProcessScene(Scene);
}

inline bool BinRowTestBit(const uint64 Mask, const int32 Bit)
{
	return (Mask & (1ull << Bit)) != 0;
}

void UOcclusionCullingSubsystem::DebugDrawToCanvas(const UCanvas* Canvas, int32 InX, int32 InY)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	const FOcclusionFrameResults* Results = Available.Get();
	if (Results == nullptr)
	{
		return;
	}

	const FLinearColor ColorBuffer[2] =
	{
		FLinearColor(0.1f, 0.1f, 0.1f), // Un-Occluded
		FLinearColor::White // Occluded
	};

	FBatchedElements* BatchedElements = Canvas->Canvas->GetBatchedElements(FCanvas::ET_Line);

	for (int32 i = 0; i < BIN_NUM; ++i)
	{
		const int32 BinStartX = InX + i * BIN_WIDTH;
		const int32 BinStartY = InY;

		// vertical line for each bin border
		BatchedElements->AddLine(FVector(BinStartX, BinStartY, 0.f), FVector(BinStartX, BinStartY + FRAMEBUFFER_HEIGHT, 0.f), FColor::Blue, FHitProxyId());

		const FFramebufferBin& Bin = Results->Bins[i];
		for (int32 j = 0; j < FRAMEBUFFER_HEIGHT; ++j)
		{
			const uint64 RowData = Bin.Data[j];
			const int32 BitY = (FRAMEBUFFER_HEIGHT + InY) - j; // flip image by Y axis

			FVector Pos0 = FVector(BinStartX, BitY, 0.f);
			int32 Bit0 = BinRowTestBit(RowData, 0) ? 1 : 0;

			for (int32 k = 1; k < BIN_WIDTH; ++k)
			{
				if (const int32 Bit1 = BinRowTestBit(RowData, k) ? 1 : 0; Bit0 != Bit1 || (k == (BIN_WIDTH - 1)))
				{
					const int32 BitX = BinStartX + k;
					FVector Pos1 = FVector(BitX, BitY, 0.f);
					BatchedElements->AddLine(Pos0, Pos1, ColorBuffer[Bit0], FHitProxyId());
					Pos0 = Pos1;
					Bit0 = Bit1;
				}
			}
		}
	}

	// Vertical line for last bin border
	const int32 BinX = InX + BIN_NUM * BIN_WIDTH;
	const int32 BinY = InY;
	BatchedElements->AddLine(FVector(BinX, BinY, 0.f), FVector(BinX, BinY + FRAMEBUFFER_HEIGHT, 0.f), FColor::Blue, FHitProxyId());
#endif//!(UE_BUILD_SHIPPING || UE_BUILD_TEST)
}

bool UOcclusionCullingSubsystem::RegisterOcclusionSettings(UStaticMeshComponent* StaticMeshComponent, const FOcclusionSettings& OcclusionSettings)
{
	if(!IsValid(StaticMeshComponent))
	{
		return false;
	}

	// Do not register meshes that do no have valid owners
	const AActor* MeshOwner = StaticMeshComponent->GetOwner();
	if(!IsValid(MeshOwner))
	{
		return false;
	}

	// Do not register meshes that are hidden in game 
	if(StaticMeshComponent->bHiddenInGame || MeshOwner->IsHidden())
	{
		return false;
	}

	if(StaticMeshComponent->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
	{
		return false;
	}

	if(const auto FoundPrimitiveInfo = PrimitiveContextMap.Find(StaticMeshComponent->ComponentId))
	{
		UOcclusionPrimitiveContext* PrimitiveInfo = *FoundPrimitiveInfo;
		PrimitiveInfo->SetOcclusionSettings(OcclusionSettings);
	}
	else
	{
		UOcclusionPrimitiveContext* PrimitiveInfo = NewObject<UOcclusionPrimitiveContext>();
		PrimitiveInfo->Setup(StaticMeshComponent, OcclusionSettings);
		PrimitiveContextMap.Add(StaticMeshComponent->ComponentId, PrimitiveInfo);
	}
	return true;
}

void UOcclusionCullingSubsystem::UnregisterOcclusionSettings(const UStaticMeshComponent* StaticMeshComponent)
{
	PrimitiveContextMap.Remove(StaticMeshComponent->ComponentId);
}

void UOcclusionCullingSubsystem::PopulateScene(TArray<FOcclusionPrimitiveProxy*>& Scene)
{
	for (TObjectIterator<UStaticMeshComponent> Itr; Itr; ++Itr)
	{
		UStaticMeshComponent* Component = *Itr;
		if(!IsValid(Component))
		{
			continue;
		}

		if(Component->GetWorld() != GetLocalPlayer()->GetWorld())
		{
			continue;
		}

		if(!PrimitiveContextMap.Contains(Component->ComponentId))
		{
			const FOcclusionSettings& OcclusionSettings = GetDefault<UDefaultOcclusionSettings>()->DefaultOcclusionSettings;
			const bool bRegistered = RegisterOcclusionSettings(Component, OcclusionSettings);
			if(!bRegistered) continue;
		}

		UOcclusionPrimitiveContext* PrimitiveInfo = PrimitiveContextMap[Component->ComponentId];
		PrimitiveInfo->UpdateBounds();
		if (PrimitiveInfo->PerformFrustumCull(PlayerCameraManager))
		{
			continue;
		}

		if (CVarVisualizeSoftwareOcclusionCullingBounds)
		{
			PrimitiveInfo->DebugBounds();
		}

		Scene.Add(PrimitiveInfo->GetProxy());
	}
}

int32 UOcclusionCullingSubsystem::ProcessScene(const TArray<FOcclusionPrimitiveProxy*>& Scene)
{
	if (Scene.IsEmpty())
	{
		return 0;
	}
	
	// Make sure occlusion task issued last frame is completed
	FlushSceneProcessing();

	// Finished processing occlusion, set results as available
	Available = MoveTemp(Processing);

	// Submit occlusion scene for next frame
	Processing = MakeUnique<FOcclusionFrameResults>();
	TaskRef = SubmitScene(Scene, FOcclusionViewInfo(PlayerCameraManager), Processing.Get());

	// Apply available occlusion results
	int32 NumCulled = 0;
	if (Available.IsValid())
	{
		NumCulled = ApplyResults(Scene, *Available);
	}

	return NumCulled;
}

int32 UOcclusionCullingSubsystem::ApplyResults(const TArray<FOcclusionPrimitiveProxy*> Scene, const FOcclusionFrameResults& Results)
{
	int32 NumOccluded = 0;

	for (const FOcclusionPrimitiveProxy* Proxy : Scene)
	{
		// Visible by default
		bool bHidden = false;
		FPrimitiveComponentId PrimId = Proxy->PrimitiveComponentId;
		if (const bool* bVisiblePtr = Results.VisibilityMap.Find(PrimId))
		{
			if (*bVisiblePtr == false)
			{
				bHidden = true;
				NumOccluded++;
			}
		}

		if(!PrimitiveContextMap.Contains(Proxy->PrimitiveComponentId))
		{
			continue;
		}
		PrimitiveContextMap[Proxy->PrimitiveComponentId]->SetHiddenInGame(bHidden);
	}

	INC_DWORD_STAT_BY(STAT_SoftwareCulledPrimitives, NumOccluded);

	return NumOccluded;
}

void UOcclusionCullingSubsystem::FlushSceneProcessing()
{
	if (TaskRef.IsValid())
	{
		FTaskGraphInterface::Get().WaitUntilTaskCompletes(TaskRef);
		TaskRef = nullptr;
	}
}
