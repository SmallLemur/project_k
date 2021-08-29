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

// always last
#include "VTSMeshActor.generated.h"


struct FVTSLoadMesh {

	vts::ResourceInfo* Info;
	vts::GpuMeshSpec* Spec;
	FString DebugId;

	FVTSLoadMesh(vts::ResourceInfo* info, vts::GpuMeshSpec* spec, FString debugId) {
		Info = info;
		Spec = spec;
		DebugId = debugId;
	}
};



DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnLoadMeshDelegate,
	const TArray<FVector>&, Vertices,
	const TArray<FVector>&, Normals,
	const TArray<int32>&, Triangles,
	const TArray<FVector2D>&, Uvs
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLoadMeshSimpleDelegate,
	const TArray<FVector>&, Vertices,
	const TArray<int32>&, Triangles
);


UCLASS()
class PROJECT_K_API AVTSMeshActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVTSMeshActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	UPROPERTY(BlueprintAssignable)
	FOnLoadMeshDelegate OnLoadMesh;

	UPROPERTY(BlueprintAssignable)
	FOnLoadMeshSimpleDelegate OnLoadMeshSimple;


	void LoadMesh(FVTSLoadMesh* loadMeshInfo);

private:
	FMatrix* vts2Matrix(double proj[16]);
	TArray<FVector>* ExtractBuffer3(vts::GpuMeshSpec& spec, int attributeIndex);
	TArray<int32>* LoadTrianglesIndices(vts::GpuMeshSpec& spec);
	float ExtractFloat(vts::GpuMeshSpec& spec, uint32 byteOffset, vts::GpuTypeEnum type, bool normalized);
	short BytesToInt16(vts::Buffer& input, uint32 startOffset);
	unsigned short BytesToUInt16(vts::Buffer& input, uint32 startOffset);
	int BytesToInt32(vts::Buffer& input, uint32 startOffset);
	float BytesToSingle(vts::Buffer& input, uint32 startOffset);
};
