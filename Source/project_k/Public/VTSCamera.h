// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include <vts-browser/map.hpp>
#include <vts-browser/camera.hpp>
#include <vts-browser/cameraDraws.hpp>
#include <vts-browser/navigation.hpp>
#include <vts-browser/position.hpp>
#include <vts-browser/positionCommon.h>
#include <vts-browser/log.h>
#include <vts-browser/mapCallbacks.hpp>
#include <vts-browser/resources.hpp>

#include <VTSMeshActor.h>
#include "VTSMap.h"
#include "VTSUtil.h"
#include <CoordinateFunctions.h>
#include "Runtime/Engine/Classes/Engine/World.h"

#include "VTSCamera.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_K_API UVTSCamera : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UVTSCamera();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UPROPERTY(EditAnywhere, Category = "VTS")
	AVTSMap* vtsMap;

	UPROPERTY(EditAnywhere, Category = "VTS")
	UCameraComponent* uecam;

	UPROPERTY(EditAnywhere, Category = "VTS")
	TSubclassOf<AActor> TileBP;

	UPROPERTY(EditAnywhere, Category = "VTS")
	FVector vector;

	FTransform uecamTransform;
	FTransform mapTransform;

	std::shared_ptr<vts::Camera> vcam;
	std::shared_ptr<vts::Navigation> vnav;

	void CameraDraw();

	AActor* InitTile(FVTSMesh* vtsMesh, FTransform transform);

	void UpdateTile(AActor* tile, FTransform transform);

	TMap<FString, TArray<AActor*>*> loadedMeshes;
	
	FMatrix ScaleVTS2UE = FMatrix::Identity.ApplyScale(100);

	bool flag = false;
};
