// Copyright Epic Games, Inc. All Rights Reserved.

#include "VTS.h"
#include "Core.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"

THIRD_PARTY_INCLUDES_START
//#include <ThirdParty/VTSLibrary/include/vts-browser/log.h>
//#include <log.h>
#include <vts-browser/log.h>
THIRD_PARTY_INCLUDES_END
#define LOCTEXT_NAMESPACE "FVTS"

void FVTS::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	// Get the base directory of this plugin
	FString BaseDir = IPluginManager::Get().FindPlugin("VTS")->GetBaseDir();

	// Add on the relative location of the third party dll and load it
	FString LibraryPath;
#if PLATFORM_WINDOWS
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/VTSLibrary/bin/vts-browser.dll"));
#endif // PLATFORM_WINDOWS

	VTSLibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;
	//VTSLibraryHandle = nullptr;
	/*
	if (VTSLibraryHandle)
	{
		//FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ThirdPartyLibraryError", "Loaded vts browser"));
		//vtsLogSetConsole(true);
		//vtsLogSetFile("vtsLog.txt");
		//vtsLogAddSink(0, FVTS::UeLog);
		//vtsLog(vtsLogLevelErr1,"blah");
		//UE_LOG(LogTemp, Warning, TEXT(">>>>>> this"));
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ThirdPartyLibraryError", "Failed to load example third party library"));
	}
	*/
}

void FVTS::UeLog(const char* msg) {
	UE_LOG(LogTemp, Warning, TEXT("%s"), *msg);
}

void FVTS::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	// Free the dll handle
	FPlatformProcess::FreeDllHandle(VTSLibraryHandle);
	VTSLibraryHandle = nullptr;
}

void FVTS::Log(FString message)
{
	vtsLogSetConsole(true);
	vtsLog(vtsLogLevelInfo1, TCHAR_TO_ANSI(*message));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FVTS, VTS)
