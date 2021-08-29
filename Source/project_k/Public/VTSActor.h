// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"
#include "UObject/ObjectMacros.h"
#include "Math/Vector.h"

#include <vts-browser/map.hpp>
#include <vts-browser/camera.hpp>
#include <vts-browser/navigation.hpp>
#include <vts-browser/position.hpp>
#include <vts-browser/log.h>
#include <vts-browser/mapCallbacks.hpp>
#include <vts-browser/resources.hpp>
#include <VTSMeshActor.h>

// always last
#include "VTSActor.generated.h"

UCLASS()
class PROJECT_K_API AVTSActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AVTSActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "VTS")
	void UpdateFrom(UCameraComponent* camera);
	
	UPROPERTY(BlueprintReadWrite, Category = "VTS")
	AVTSMeshActor* meshActor;

private:
	std::shared_ptr<vts::Map> map;
	std::shared_ptr<vts::Camera> cam;
	std::shared_ptr<vts::Navigation> nav;
	bool flag;

	void SpawnSupportingActors();

	FMatrix* SwapXY = new FMatrix(
		FVector(0, 1, 0),
		FVector(1, 0, 0),
		FVector(0, 0, 1),
		FVector(0, 0, 0)
	);

};
