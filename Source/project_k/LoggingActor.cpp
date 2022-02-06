// Fill out your copyright notice in the Description page of Project Settings.


#include "LoggingActor.h"

//#include "VTS.h"
#include "Engine/GameEngine.h"

#include <vts-browser/log.h>


// Sets default values
ALoggingActor::ALoggingActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ALoggingActor::BeginPlay()
{
	Super::BeginPlay();

	uint32 logDefault = vtsLogLevelInfo3 | vtsLogLevelWarn2 | vtsLogLevelErr2;

	vtsLogSetMaskCode(logDefault);
	
	vtsLogAddSink(logDefault, UeLog);
	vtsLog(vtsLogLevelInfo4, "TEST");
	//vts::addLogSink(vts::LogLevel::default_, UeLog);
	//vts::log(vts::LogLevel::info3, "Creating OpenGL contexts");
	//UE_LOG(LogTemp, Warning, TEXT("%s"), *name);
	//FVTS::Log(TEXT("hello"));
}

// Called every frame
void ALoggingActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALoggingActor::UeLog(const char *msg) {
	//GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Yellow, FString(msg));
	FFileHelper::SaveStringToFile(FString(msg), TEXT("vts.log"));
}
