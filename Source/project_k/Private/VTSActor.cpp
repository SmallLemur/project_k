// Fill out your copyright notice in the Description page of Project Settings.


#include "VTSActor.h"
#include "..\Public\VTSActor.h"

// Sets default values
AVTSActor::AVTSActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


}

// Called when the game starts or when spawned
void AVTSActor::BeginPlay()
{
	Super::BeginPlay();

	map = std::make_shared<vts::Map>();
	cam = map->createCamera();
	nav = cam->createNavigation();

	SpawnSupportingActors();
	
	FVector2D Result = FVector2D(1, 1);

	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize( /*out*/Result);
	}

	cam->setViewportSize((uint32) Result.X, (uint32)Result.Y);

	map->setMapconfigPath("https://cdn.melown.com/mario/store/melown2015/map-config/melown/VTS-Tutorial-map/mapConfig.json");
	map->callbacks().loadMesh = [this](vts::ResourceInfo& info, vts::GpuMeshSpec& spec, const std::string& debugId) {
		//LoadMesh(info, spec, debugId);
		// https://github.com/melowntech/vts-browser-unity-plugin/blob/9ba1d85cfdc8f4bde621b4ed5f16938ac846b108/src/Vts/Scripts/BrowserUtil/VtsResources.cs
		auto meshInfo = new FVTSLoadMesh(
			&info,
			&spec,
			FString(debugId.c_str())
		);
		meshActor->LoadMesh(meshInfo);
	};
	flag = true;
}

void AVTSActor::SpawnSupportingActors() {
	meshActor = GetWorld()->SpawnActor<AVTSMeshActor>();
}

void AVTSActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (map != NULL) {
		map->renderFinalize();
	}
}

// Called every frame
void AVTSActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	map->renderUpdate(DeltaTime);
	map->dataUpdate();
	cam->renderUpdate();

	if (!map->getMapconfigReady()) {
		return;
	}

	map->renderUpdate(DeltaTime);
	cam->renderUpdate();

	// ***********************************************************************************************************
	// todo refactoring
	// split this bitch into:
	// - [x] vts actor which only constructs VTS things
	// - [x] vts mesh actor which will be loading meshes
	// - [ ] vts draw actor which will be taking draws and mositioning messes loaded by the above

	//TODO
	// 
	// construct mesh disctionary based on pointers in userdata
	// 
	// get camera draws
	//auto draws = cam->draws();
	// get tasks
	// for each task find mesh based on a pointer
	// use .mv as a transform matrix and plaec the meash
}

void AVTSActor::UpdateFrom(UCameraComponent* camera)
{
	if (!map->getMapconfigReady() || !flag) {
		return;
	}
	vts::Position pos = nav->getPosition();

	auto str = std::to_string(pos.point[0]) +
		" , " + std::to_string(pos.point[1]) +
		" , " + std::to_string(pos.point[2]);


	auto proj = new double[16];
	cam->getView(proj);
	//FMatrix mat = *vts2Matrix(proj);
	//mat *= *SwapXY;
	//FTransform* t = new FTransform(mat);
	//camera->SetWorldTransform(*t);
}
