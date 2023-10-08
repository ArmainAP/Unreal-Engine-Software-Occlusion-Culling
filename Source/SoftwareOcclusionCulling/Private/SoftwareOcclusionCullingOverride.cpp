// Copyright to Kat Code Labs, SRL. All Rights Reserved.


#include "SoftwareOcclusionCullingOverride.h"
#include "OcclusionCullingSubsystem.h"

// Sets default values for this component's properties
USoftwareOcclusionCullingOverride::USoftwareOcclusionCullingOverride()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USoftwareOcclusionCullingOverride::BeginPlay()
{
	Super::BeginPlay();

	UOcclusionCullingSubsystem* OcclusionCullingSubsystem = GetWorld()->GetFirstPlayerController()->GetLocalPlayer()->GetSubsystem<UOcclusionCullingSubsystem>();
	checkf(OcclusionCullingSubsystem, TEXT("USoftwareOcclusionCullingOverride used without a UOcclusionCullingSubsystem present! Make sure the WorldFoundation plugin is enabled"));
	
	TArray<UStaticMeshComponent*> StaticMeshComponents;
	GetOwner()->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
	for(UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
	{
		OcclusionCullingSubsystem->RegisterOcclusionSettings(StaticMeshComponent, OcclusionSettings);	
	}
}
