// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "UObject/ObjectMacros.h"
#include "Math/Vector.h"

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
#include <CoordinateFunctions.h>
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
	void UpdateFrom(AActor* camera);
	
	UPROPERTY(BlueprintReadWrite, Category = "VTS")
	AVTSMeshActor* meshActor;

	UPROPERTY(EditAnywhere, Category = "VTS")
	AActor* Camera;

	UPROPERTY(VisibleAnywhere, Category = "VTS")
	FVector llaOrigin;

	UPROPERTY(VisibleAnywhere, Category = "VTS")
	FVector origin;

	UPROPERTY(VisibleAnywhere, Category = "VTS")
	FRotator orientation;

	UPROPERTY(VisibleAnywhere, Category = "VTS")
	FVector llaPosition;

	UPROPERTY(VisibleAnywhere, Category = "VTS")
	FVector position;

private:
	FMatrix vts2Matrix(float proj[16]);
	FMatrix vts2Matrix(double proj[16]);
	FVector vts2vector(double vec[3]);
	FRotator vts2rotator(double vec[3]);
	void matrix2vts(FMatrix mat, double out[16]);
	void vector2vts(FVector vec, double out[3]);
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
