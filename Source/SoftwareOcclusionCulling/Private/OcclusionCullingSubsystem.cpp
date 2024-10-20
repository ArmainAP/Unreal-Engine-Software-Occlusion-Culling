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

UOcclusionCullingSubsystem::UOcclusionCullingSubsystem() = default;
UOcclusionCullingSubsystem::~UOcclusionCullingSubsystem() = default;

UOcclusionCullingSubsystem::UOcclusionCullingSubsystem(FVTableHelper& Helper)
	: Super(Helper)
{
}

void UOcclusionCullingSubsystem::PlayerControllerChanged(APlayerController* NewPlayerController)
{
	Super::PlayerControllerChanged(NewPlayerController);

	PlayerCameraManager = NewPlayerController->PlayerCameraManager;
}

void UOcclusionCullingSubsystem::Deinitialize()
{
	Super::Deinitialize();

	FlushSceneProcessing();
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
	TArray<FOcclusionPrimitiveProxy> Scene;
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

		const FFramebufferBin& Bin = LastFrameResults.Bins[i];
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

	if(const auto FoundPrimitiveInfo = PrimitiveContextMap.Find(StaticMeshComponent->GetPrimitiveSceneId().PrimIDValue))
	{
		UOcclusionPrimitiveContext* PrimitiveInfo = *FoundPrimitiveInfo;
		PrimitiveInfo->SetOcclusionSettings(OcclusionSettings);
	}
	else
	{
		UOcclusionPrimitiveContext* PrimitiveInfo = NewObject<UOcclusionPrimitiveContext>();
		PrimitiveInfo->Setup(StaticMeshComponent, OcclusionSettings);
		PrimitiveContextMap.Add(StaticMeshComponent->GetPrimitiveSceneId().PrimIDValue, PrimitiveInfo);
	}
	return true;
}

void UOcclusionCullingSubsystem::UnregisterOcclusionSettings(const UStaticMeshComponent* StaticMeshComponent)
{
	PrimitiveContextMap.Remove(StaticMeshComponent->GetPrimitiveSceneId().PrimIDValue);
}

void UOcclusionCullingSubsystem::PopulateScene(TArray<FOcclusionPrimitiveProxy>& Scene)
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

		if(!PrimitiveContextMap.Contains(Component->GetPrimitiveSceneId().PrimIDValue))
		{
			const FOcclusionSettings& OcclusionSettings = GetDefault<UDefaultOcclusionSettings>()->DefaultOcclusionSettings;
			const bool bRegistered = RegisterOcclusionSettings(Component, OcclusionSettings);
			if(!bRegistered) continue;
		}

		UOcclusionPrimitiveContext* PrimitiveInfo = PrimitiveContextMap[Component->GetPrimitiveSceneId().PrimIDValue];
		if(!IsValid(PrimitiveInfo))
		{
			continue;
		}
		
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

int32 UOcclusionCullingSubsystem::ProcessScene(const TArray<FOcclusionPrimitiveProxy>& Scene)
{
	if (Scene.IsEmpty())
	{
		return 0;
	}

	// Make sure occlusion task issued last frame is completed
	FlushSceneProcessing();
	
	// Finished processing occlusion, set results as available
	LastFrameResults = MoveTemp(FrameResults);

	// Submit occlusion scene for next frame
	FrameResults = FOcclusionFrameResults();
	const FOcclusionViewInfo ViewInfo = FOcclusionViewInfo(PlayerCameraManager);
	const FOcclusionSceneData SceneData = CollectSceneData(Scene, ViewInfo);

	// Submit occlusion task
	TaskRef = FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
	{
		ProcessOcclusionFrame(SceneData, FrameResults);
	}, GET_STATID(STAT_SoftwareOcclusionProcess), NULL, GetOcclusionThreadName());

	// Apply available occlusion results
	return ApplyResults(Scene);
}

FOcclusionSceneData UOcclusionCullingSubsystem::CollectSceneData(const TArray<FOcclusionPrimitiveProxy>& Scene,
                                                                 FOcclusionViewInfo View)
{
	int32 NumCollectedOccluders = 0;
	int32 NumCollectedOccludees = 0;

	const FMatrix ViewProjMat = View.ViewMatrix * View.ProjectionMatrix;
	const FVector ViewOrigin = View.Origin;
	const float MaxDistanceSquared = FMath::Square(GSOMaxDistanceForOccluder);

	// Allocate occlusion scene
	FOcclusionSceneData SceneData;
	SceneData.ViewProj = ViewProjMat;

	constexpr int32 NumReserveOccludee = 1024;
	SceneData.OccludeeBoxPrimId.Reserve(NumReserveOccludee);
	SceneData.OccludeeBoxMinMax.Reserve(NumReserveOccludee * 2);
	SceneData.OccluderData.Reserve(GSOMaxOccluderNum);

	// Collect scene geometry for occluder/occluded
	{
		SCOPE_CYCLE_COUNTER(STAT_SoftwareOcclusionGather);

		FSWOccluderElementsCollector Collector(SceneData);

		TArray<FPotentialOccluderPrimitive> PotentialOccluders;
		PotentialOccluders.Reserve(GSOMaxOccluderNum);

		for (const FOcclusionPrimitiveProxy& Info : Scene)
		{
			FBoxSphereBounds Bounds = Info.Bounds;
			const FPrimitiveComponentId PrimitiveComponentId = Info.PrimitiveComponentId;
			FMatrix LocalToWorld = Info.LocalToWorld;

			const bool bHasHugeBounds = Bounds.SphereRadius > HALF_WORLD_MAX / 2.0f; // big objects like skybox
			float DistanceSquared = 0.f;
			float ScreenSize = 0.f;

			// Find out whether primitive can/should be occluder or occludee
			bool bCanBeOccluder = !bHasHugeBounds && Info.bOccluder;
			if (bCanBeOccluder)
			{
				// Size/distance requirements
				DistanceSquared = FMath::Max(OCCLUDER_DISTANCE_WEIGHT, (Bounds.Origin - ViewOrigin).SizeSquared() - FMath::Square(Bounds.SphereRadius));
				if (DistanceSquared < MaxDistanceSquared)
				{
					ScreenSize = ComputeBoundsScreenSize(Bounds.Origin, Bounds.SphereRadius, View.Origin, View.ProjectionMatrix);
				}

				bCanBeOccluder = GSOMinScreenRadiusForOccluder < ScreenSize;
			}

			if (bCanBeOccluder)
			{
				FPotentialOccluderPrimitive PotentialOccluder;
				PotentialOccluder.PrimitiveComponentId = PrimitiveComponentId;
				PotentialOccluder.OccluderData = Info.OccluderData;
				PotentialOccluder.LocalToWorld = LocalToWorld;
				PotentialOccluder.Weight = ComputePotentialOccluderWeight(ScreenSize, DistanceSquared);
				PotentialOccluders.Add(PotentialOccluder);
			}

			if (!bHasHugeBounds && Info.bOcluded)
			{
				// Collect occluded box
				CollectOccludeeGeom(Bounds, PrimitiveComponentId, SceneData);
				NumCollectedOccludees++;
			}
		}

		// Sort potential occluders by weight
		PotentialOccluders.Sort([&](const FPotentialOccluderPrimitive& A, const FPotentialOccluderPrimitive& B) {
			return A.Weight > B.Weight;
		});

		// Add sorted occluders to scene up to GSOMaxOccluderNum
		for (const FPotentialOccluderPrimitive& PotentialOccluder : PotentialOccluders)
		{
			const FPrimitiveComponentId PrimitiveComponentId = PotentialOccluder.PrimitiveComponentId;

			// Collect occluder geometry
			Collector.SetPrimitiveID(PrimitiveComponentId);
			Collector.AddElements(PotentialOccluder.OccluderData.Vertices, PotentialOccluder.OccluderData.Indices, PotentialOccluder.LocalToWorld);
			NumCollectedOccluders++;

			if (NumCollectedOccluders >= GSOMaxOccluderNum)
			{
				break;
			}
		}
	}

	INC_DWORD_STAT_BY(STAT_SoftwareOccluders, NumCollectedOccluders);
	INC_DWORD_STAT_BY(STAT_SoftwareOccludees, NumCollectedOccludees);

	return SceneData;
}

int32 UOcclusionCullingSubsystem::ApplyResults(const TArray<FOcclusionPrimitiveProxy> Scene)
{
	int32 NumOccluded = 0;

	for (const FOcclusionPrimitiveProxy& Proxy : Scene)
	{
		// Visible by default
		bool bHidden = false;
		FPrimitiveComponentId PrimId = Proxy.PrimitiveComponentId;
		if (const bool* bVisiblePtr = LastFrameResults.VisibilityMap.Find(PrimId))
		{
			if (*bVisiblePtr == false)
			{
				bHidden = true;
				NumOccluded++;
			}
		}

		if(!PrimitiveContextMap.Contains(Proxy.PrimitiveComponentId.PrimIDValue))
		{
			continue;
		}
		PrimitiveContextMap[Proxy.PrimitiveComponentId.PrimIDValue]->SetHiddenInGame(bHidden);
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