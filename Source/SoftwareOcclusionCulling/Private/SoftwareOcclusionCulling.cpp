// Copyright Epic Games, Inc. All Rights Reserved.

#include "SoftwareOcclusionCulling.h"

#include "ISettingsModule.h"
#include "Data/OcclusionSettings.h"

#define LOCTEXT_NAMESPACE "FSoftwareOcclusionCullingModule"

void FSoftwareOcclusionCullingModule::StartupModule()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))	
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "SoftwareOcclusionCulling",
			LOCTEXT("SoftwareOcclusionCullingName", "Software Occlusion Culling"),
			LOCTEXT("SoftwareOcclusionCullingDescription", "Default settings for Software Occlusion Culling"),
			GetMutableDefault<UOcclusionSettings>()
		);
	}
}

void FSoftwareOcclusionCullingModule::ShutdownModule()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "SoftwareOcclusionCulling");
	}
}
#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSoftwareOcclusionCullingModule, SoftwareOcclusionCulling)