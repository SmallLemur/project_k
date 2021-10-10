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
		
		int32 index = meshActor->LoadMesh(meshInfo);
		//meshInfo->Info->userData = std::make_shared<int32>(index);
	};
	map->callbacks().mapconfigReady = [this]() {
		auto pos = map->getMapDefaultPosition();
		auto str = std::to_string(pos.point[0]) +
			" , " + std::to_string(pos.point[1]) +
			" , " + std::to_string(pos.point[2]);
		GEngine->AddOnScreenDebugMessage(-1, 150.f, FColor::Cyan, str.c_str());

		llaOrigin = vts2vector(pos.point);
		FVector temp;
		UCoordinateFunctions::LLAToECEF(llaOrigin, temp);
		UCoordinateFunctions::ECEFToUE4(temp, origin);
		
		orientation = vts2rotator(pos.orientation);
		
		Camera->SetActorLocation(origin);
		Camera->SetActorRotation(orientation);
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

	FVector temp;
	position = Camera->GetActorLocation();
	UCoordinateFunctions::UE4ToECEF(position, temp);
	UCoordinateFunctions::ECEFToLLA2(temp, llaPosition);
	
	double vec[3];
	vector2vts(llaPosition, vec);
	double vec2[3] = {
		vec[1],
		vec[0],
		vec[2]
	};
	nav->setPoint(vec2);

	auto d = cam->draws();
	for (auto o : d.colliders)
	{
		FMatrix m = vts2Matrix(o.mv) * *SwapXY;
		FTransform t = FTransform(m);
		t.AddToTranslation(origin);

		meshActor->UpdateMesh(o, t);
	}
}

void AVTSActor::UpdateFrom(AActor* camera)
{
	if (!map->getMapconfigReady() || !flag) {
		return;
	}
	//vts::Position pos = nav->getPosition();
	double p[3];
	nav->getPoint(p);

	auto str = std::to_string(p[0]) +
		" , " + std::to_string(p[1]) +
		" , " + std::to_string(p[2]);
	//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Cyan, str.c_str());

	auto proj = new double[16];
	cam->getProj(proj);
	FMatrix original = vts2Matrix(proj);

	//FMatrix mat = *vts2Matrix(proj);
	//mat *= *SwapXY;
	//FTransform* t = new FTransform(mat);
	//camera->SetWorldTransform(*t);
	//double point[] = {
	//	(double) camera->GetTargetLocation().Y,
	//	(double) camera->GetTargetLocation().X,
	//	(double) camera->GetTargetLocation().Z
	//};

	//FVector upoint = FVector(
	//	pos.point[0],
	//	pos.point[1],
	//	pos.point[2]
	//);
	/*
	double navPoint[3];
	map->convert(p, navPoint, vts::Srs::Navigation, vts::Srs::Physical);
	FVector unavPoint = FVector(
		navPoint[0],
		navPoint[1],
		navPoint[2]
	);

	FVector unavShifted = unavPoint + camera->GetActorLocation();
	//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, unavPoint.ToString());

	double navShifted[] = {
		unavShifted.X,
		unavShifted.Y,
		unavShifted.Z
	};
	double shifted[3];
	map->convert(navShifted, shifted, vts::Srs::Physical, vts::Srs::Navigation);

	//nav->setPoint(shifted);
	//camera->SetActorLocation(origin);


	//TODO: camera gets weirdly reset insead of updating
	// try flipping translation concat (order of operations)
	//FMatrix translate = FMatrix::Identity.ConcatTranslation(origin).ConcatTranslation(-camera->GetActorLocation());
	//FMatrix transformed = original.ConcatTranslation(origin);
	//FMatrix transformed = translate * original;
	//matrix2vts(transformed, proj);
	//cam->setProj(proj);
	
	//translate.TransformPosition(point)
	//nav->setPosition(pos);
	*/
}


FMatrix AVTSActor::vts2Matrix(float proj[16]) {
	return FMatrix(
		FPlane(
			proj[0],
			proj[1],
			proj[2],
			proj[3]
		),
		FPlane(
			proj[4],
			proj[5],
			proj[6],
			proj[7]
		),
		FPlane(
			proj[8],
			proj[9],
			proj[10],
			proj[11]
		),
		FPlane(
			proj[12],
			proj[13],
			proj[14],
			proj[15]
		)
	);

}FMatrix AVTSActor::vts2Matrix(double proj[16]) {
	return FMatrix(
		FPlane(
			proj[0],
			proj[1],
			proj[2],
			proj[3]
		),
		FPlane(
			proj[4],
			proj[5],
			proj[6],
			proj[7]
		),
		FPlane(
			proj[8],
			proj[9],
			proj[10],
			proj[11]
		),
		FPlane(
			proj[12],
			proj[13],
			proj[14],
			proj[15]
		)
	);
}

void AVTSActor::matrix2vts(FMatrix mat, double out[16]) {
	for (size_t i = 0; i < 4; i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
			out[j+i*4] = mat.M[i][j];
		}
	}
}

FVector AVTSActor::vts2vector(double vec[3]) {
	return FVector(
		vec[0],
		vec[1],
		vec[2]
	);
}

FRotator AVTSActor::vts2rotator(double vec[3]) {
	return FRotator(
		vec[0],
		vec[1],
		vec[2]
	);
}

void AVTSActor::vector2vts(FVector vec, double out[3]) {
	out[0] = vec.X;
	out[1] = vec.Y;
	out[2] = vec.Z;
}