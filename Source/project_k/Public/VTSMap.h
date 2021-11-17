// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include <vts-browser/map.hpp>
#include <vts-browser/camera.hpp>
#include <vts-browser/cameraDraws.hpp>
#include <vts-browser/navigation.hpp>
#include <vts-browser/position.hpp>
#include <vts-browser/positionCommon.h>
#include <vts-browser/log.h>
#include <vts-browser/mapCallbacks.hpp>
#include <vts-browser/resources.hpp>
#include <CoordinateFunctions.h>
#include <VTSMeshActor.h>
#include <VTSUtil.h>

#include "VTSMap.generated.h"

UCLASS()
class PROJECT_K_API AVTSMap : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AVTSMap();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "VTS")
	FString ConfigURL = "https://cdn.melown.com/vts/melown2015/unity/world/mapConfig.json";

	UPROPERTY(EditAnywhere, Category = "VTS")
	FString AuthURL = "";

	UPROPERTY(EditAnywhere, Category = "VTS")
	FString CreateConfig = "";

	UPROPERTY(EditAnywhere, Category = "VTS")
	FString RunConfig = "{ \"targetResourcesMemoryKB\":500000 }";

	UPROPERTY(VisibleAnywhere, Category = "VTS Position")
	FVector LlaOrigin;

	UPROPERTY(VisibleAnywhere, Category = "VTS Position")
	FVector PhysicalOrigin;

	UPROPERTY(VisibleAnywhere, Category = "VTS Position")
	FVector UEOrigin;

	void MakeLocal(double navPt[3]);

	void VtsNavigation2Unity(double vect[3], FVector out);

	void Unity2VtsNavigation(FVector vect, double out[3]);

	std::shared_ptr<vts::Map> map;

};