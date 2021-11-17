// Fill out your copyright notice in the Description page of Project Settings.


#include "VTSActor.h"
#include "..\Public\VTSActor.h"
#include "VTSUtil.h"

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

	map->setMapconfigPath("https://cdn.melown.com/mario/store/melown2015/map-config/melown/VTS-Tutorial-map/mapConfig.json"); // Earth demp
	// map->setMapconfigPath("https://cdn.melown.com/mario/store/mercury-provisional/map-config/melown/mercury-messenger/mapConfig.json"); // Mercury demo


	map->callbacks().loadMesh = [this](vts::ResourceInfo& info, vts::GpuMeshSpec& spec, const std::string& debugId) {
		// https://github.com/melowntech/vts-browser-unity-plugin/blob/9ba1d85cfdc8f4bde621b4ed5f16938ac846b108/src/Vts/Scripts/BrowserUtil/VtsResources.cs
		auto meshInfo = new FVTSLoadMesh(
			&info,
			&spec,
			FString(debugId.c_str())
		);
		
		meshActor->LoadMesh(meshInfo);
	};
	map->callbacks().loadTexture = [this](vts::ResourceInfo& info, vts::GpuTextureSpec& spec, const std::string& id) {};

	map->callbacks().mapconfigReady = [this]() {
		auto pos = map->getMapDefaultPosition();
		auto str = std::to_string(pos.point[0]) +
			" , " + std::to_string(pos.point[1]) +
			" , " + std::to_string(pos.point[2]);
		GEngine->AddOnScreenDebugMessage(-1, 150.f, FColor::Cyan, str.c_str());

		llaOrigin = UVTSUtil::vts2vector(pos.point);
		//FVector temp;

		//UCoordinateFunctions::LLAToECEF(llaOrigin, temp);
		double temp[3];
		map->convert(pos.point, temp, vts::Srs::Navigation, vts::Srs::Physical);
		UCoordinateFunctions::ECEFToUE4(UVTSUtil::vts2vector(temp), origin);
		
		orientation = UVTSUtil::vts2rotator(pos.orientation);
		
		//Camera->SetActorLocation(origin);
		Camera->SetActorRotation(orientation);
	};
	flag = true;
}

void AVTSActor::SpawnSupportingActors() {
	//meshActor = GetWorld()->SpawnActor<AVTSMeshActor>();
	//meshActor = GetWorld()->fin
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
	
	position = (Camera->GetActorLocation()/100) + origin;
	
	double temp[3];
	FVector pos;
	UCoordinateFunctions::UE4ToECEF(position, pos);
	double tempPos[3];
	UVTSUtil::vector2vts(pos, tempPos);
	map->convert(tempPos, temp, vts::Srs::Physical, vts::Srs::Navigation);
	llaPosition = UVTSUtil::vts2vector(temp);

	nav->setPoint(temp);
	//auto tm = Camera->GetActorTransform().ToMatrixNoScale() * *SwapXY;
	//double view[16];
	//matrix2vts(tm, view);
	//cam->setView(view);
	
	/*
	FVector temp;
	position = Camera->GetActorLocation()+origin;
	UCoordinateFunctions::UE4ToECEF(position, temp);
	UCoordinateFunctions::ECEFToLLA2(temp, llaPosition);
	
	double vec[3];
	vector2vts(llaPosition, vec);
	nav->setPoint(vec);
	*/
	

	double originalNavigationPoint[3];
	ue2vtsNavigation(FVector::ZeroVector, originalNavigationPoint);

	double targetNavigationPoint[3];
	ue2vtsNavigation(Camera->GetActorLocation(), targetNavigationPoint);

	//makeLocal(targetNavigationPoint);
	
	auto v = Camera->GetActorLocation();
	//meshActor->SetActorLocation(FVector(0, -v.Size(), 0));
	/*
	meshActor->SetActorRotation(
		FQuat::MakeFromEuler(FVector(0, 0, -90.0))  *
		FQuat::FindBetweenVectors(-v, GetActorLocation())
	);
	*/
	FVector move = -Camera->GetActorLocation();
	float Yrot = (float)(targetNavigationPoint[0] - originalNavigationPoint[0]) * FMath::Sign((float)originalNavigationPoint[1]);

	auto d = cam->draws();
	auto conv = FMatrix::Identity;// **TransformCoord* vts2Matrix(d.camera.view).Inverse();
	for (auto o : d.opaque)
	{
		FMatrix m = conv * UVTSUtil::vts2Matrix(o.mv) ;// * *TransformCoord;
		//FMatrix m = conv * vts2Matrix(o.mv); //* *SwapXY;
		FTransform t = FTransform(m);




		/*
		t.SetTranslation(m.GetColumn(3));
		
			FVector(
				m.GetColumn(0).Size() * sxs,
				m.GetColumn(1).Size
		float sxs = m.Determinant() < 0 ? -1 : 1;
		t.SetScale3D((),
				m.GetColumn(2).Size()
			)
		);
		t.SetRotation(
			FRotationMatrix::MakeFromXZ(
				m.GetColumn(2) / t.GetScale3D().Z,
				m.GetColumn(1) / t.GetScale3D().Y
			).ToQuat()
		);
		
		transformStuff = t;
		//t.AddToTranslation(-vts2vector(vec2));
		//t.AddToTranslation(origin-position);
		//t.AddToTranslation(origin);
		//t.SetTranslation(t.GetTranslation() - position);
		*/

		//t.AddToTranslation(move);
		//t.SetRotation(FVector::ZeroVector.RotateAngleAxis(Yrot, FVector::UpVector).Rotation().Quaternion());

		auto tt = vts::DrawColliderTask();
		tt.mesh = o.mesh;
		meshActor->UpdateMesh(tt, t);
	}

	// get the nomenclature data
	/*for (auto j : d.geodata)
	{
		auto geodata = j.geodata;
	}

	*/
}

void AVTSActor::ue2vtsNavigation(FVector vec, double out[3]) {
	FVector4 point4 = GetActorTransform().ToMatrixWithScale().Inverse().TransformFVector4(FVector4(vec, 1));
	FVector point = FVector(point4);

	FVector ecef;
	UCoordinateFunctions::UE4ToECEF(point, ecef);
	double ecefTemp[3];
	UVTSUtil::vector2vts(ecef, ecefTemp);

	double temp[3];
	map->convert(ecefTemp, temp, vts::Srs::Physical, vts::Srs::Navigation);
	out[0] = temp[0];
	out[1] = temp[1];
	out[2] = temp[2];
}

void AVTSActor::makeLocal(double navPt[3]) {
	double p[3];
	map->convert(navPt, p , vts::Srs::Navigation, vts::Srs::Physical);
	{ // swap YZ
		double tmp = p[0];
		p[0] = p[1];
		p[1] = tmp;
	}

	FVector v = UVTSUtil::vts2vector(p) * GetActorTransform().GetScale3D();
	if (map->getMapProjected())
	{
		SetActorLocation(-v);
	}
	else
	{
		float m = v.Size();
		SetActorLocation(FVector(0, -m, 0));
		SetActorRotation(
			FQuat::MakeFromEuler(FVector(0, navPt[0] + 90.0, 0)) * 
			FQuat::FindBetweenVectors(-v, GetActorLocation())
		);
	}
}
