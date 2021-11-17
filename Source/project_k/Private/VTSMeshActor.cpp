// Fill out your copyright notice in the Description page of Project Settings.


#include "VTSMeshActor.h"

// Sets default values
AVTSMeshActor::AVTSMeshActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AVTSMeshActor::BeginPlay()
{
	Super::BeginPlay();

	arrow = NewObject<UArrowComponent>(this, FName("arrow", 0));
	arrow->RegisterComponent();
}

// Called every frame
void AVTSMeshActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

int32 AVTSMeshActor::LoadMesh(FVTSLoadMesh* loadMeshInfo)
{
	TArray<FVector>* vertices = UVTSUtil::ExtractBuffer3(*loadMeshInfo->Spec, 0);
	TArray<FVector> normals;
	TArray<int32>* triangles = UVTSUtil::LoadTrianglesIndices(*loadMeshInfo->Spec);
	TArray<FVector2D> uvs;
	TArray<FLinearColor> colors;
	TArray<FProcMeshTangent> tangents;
	for (size_t i = 0; i < vertices->Num(); i++)
	{
		normals.Add(FVector(0, 0, 1));
		uvs.Add(FVector2D(0, 0));
		colors.Add(FLinearColor(0xff, 0, 0));
		tangents.Add(FProcMeshTangent(0, 1, 0));
	}

	FLoadedMesh* loadedMesh = new FLoadedMesh(
		currentMeshSectionIndex, vertices, &normals, &uvs, &colors, &tangents
	);
	currentMeshSectionIndex++;
	/*
	UProceduralMeshComponent* TargetMesh = NewObject<UProceduralMeshComponent>(this, FName("mesh", currentMeshSectionIndex));
	TargetMesh->RegisterComponent();

	TargetMesh->CreateMeshSection_LinearColor(
		0,
		*loadedMesh->Vertices,
		*triangles,
		*loadedMesh->Normals,
		*loadedMesh->UVs,
		*loadedMesh->Colors,
		*loadedMesh->Tangents,
		false
	);*/

	FActorSpawnParameters SpawnParams;
	AActor* tile = GetWorld()->SpawnActor<AActor>(TileBP, GetTransform(), SpawnParams);

	TArray<UProceduralMeshComponent*> Comps;

	tile->GetComponents(Comps);
	UProceduralMeshComponent* TargetMesh;
	if (Comps.Num() > 0)
	{
		TargetMesh = Comps[0];
		TargetMesh->CreateMeshSection_LinearColor(
			0,
			*loadedMesh->Vertices,
			*triangles,
			*loadedMesh->Normals,
			*loadedMesh->UVs,
			*loadedMesh->Colors,
			*loadedMesh->Tangents,
			false
		);
	}

	AVTSMeshActor::loadedMeshes.Add(loadedMesh->SectionIndex, loadedMesh);
	
	std::shared_ptr<FLoadedMeshIndex> sp = std::make_shared<FLoadedMeshIndex>();
	sp->SectionIndex = loadedMesh->SectionIndex;
	sp->TargetMesh = tile;

	loadMeshInfo->Info->userData = sp;
	return loadedMesh->SectionIndex;
}

void AVTSMeshActor::UpdateMesh(vts::DrawColliderTask task, FTransform transform) {
	FLoadedMeshIndex* loadedMeshIndex = (FLoadedMeshIndex*)task.mesh.get();
	int32 meshIndex = loadedMeshIndex->SectionIndex;
	
	FLoadedMesh* loadedMesh = AVTSMeshActor::loadedMeshes[meshIndex];
	
	if (!loadedMesh){
		return;
	}

	loadedMeshIndex->TargetMesh->SetActorTransform(transform);
	/*
	TArray<FVector> transformed;
	for (auto vec : *loadedMesh->Vertices) {
		transformed.Add(transform.TransformPosition(vec));
	}
	//loadedMesh->Vertices = &transformed;
	loadedMeshIndex->TargetMesh->UpdateMeshSection_LinearColor(
		0,//loadedMesh->SectionIndex,
		transformed,//*loadedMesh->Vertices,
		TArray<FVector>(),//*loadedMesh->Normals,
		TArray<FVector2D>(), //*loadedMesh->UVs,
		TArray<FLinearColor>(), //*loadedMesh->Colors,
		TArray<FProcMeshTangent>() //*loadedMesh->Tangents
	);*/
}
