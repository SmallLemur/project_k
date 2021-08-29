// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FVTS : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	static void Log(FString message);
	static void UeLog(const char* msg);

private:
	/** Handle to the test dll we will load */
	void* VTSLibraryHandle;
};
