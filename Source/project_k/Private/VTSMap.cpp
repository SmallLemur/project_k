// Fill out your copyright notice in the Description page of Project Settings.


#include "VTSMap.h"

// Sets default values
AVTSMap::AVTSMap()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	urls.Add(urlUnity);
	urls.Add(urlMercury);
	urls.Add(urlMars);
	urls.Add(urlSurface);
	urls.Add(urlSurfaceSpheroid);
	urls.Add(urlVTStuto);

}

// Called when the game starts or when spawned
void AVTSMap::BeginPlay()
{
	Super::BeginPlay();

	map = std::make_shared<vts::Map>();
	std::string const s = TCHAR_TO_UTF8(*ConfigURL);
	map->setMapconfigPath(s);
	
	map->callbacks().loadMesh = [this](vts::ResourceInfo& info, vts::GpuMeshSpec& spec, const std::string& debugId) {
		// https://github.com/melowntech/vts-browser-unity-plugin/blob/9ba1d85cfdc8f4bde621b4ed5f16938ac846b108/src/Vts/Scripts/BrowserUtil/VtsResources.cs
		LoadMesh(info, spec, FString(debugId.c_str()));
	};

	map->callbacks().loadTexture = [this](vts::ResourceInfo& info, vts::GpuTextureSpec& spec, const std::string& id) {};
	map->callbacks().loadFont = [this](vts::ResourceInfo& info, vts::GpuFontSpec& spec, const std::string& id) {};
	map->callbacks().loadGeodata = [this](vts::ResourceInfo& info, vts::GpuGeodataSpec& spec, const std::string& id) {
		LoadGeodata(info, spec, FString(id.c_str()));
	};

	map->callbacks().mapconfigReady = [this]() {
		auto pos = map->getMapDefaultPosition();
		LlaOrigin = UVTSUtil::vts2vector(pos.point);
	

		double temp[3];
		map->convert(pos.point, temp, vts::Srs::Navigation, vts::Srs::Physical);
		PhysicalOrigin = UVTSUtil::vts2vector(temp);

		UCoordinateFunctions::ECEFToUE4(PhysicalOrigin, UEOrigin);

		GEngine->AddOnScreenDebugMessage(-1, 150.f, FColor::Cyan, LlaOrigin.ToString());

		//orientation = UVTSUtil::vts2rotator(pos.orientation);

		//Camera->SetActorLocation(origin);
		//Camera->SetActorRotation(orientation);
	};

}

void AVTSMap::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (map != NULL) {
		map->renderFinalize();
		map = NULL;
	}
}

// Called every frame
void AVTSMap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	map->renderUpdate(DeltaTime);
	map->dataUpdate();
	
	if (!map->getMapconfigReady()) {
		return;
	}
	// Handle the VTS make local here
	vts::Position pos = map->getMapDefaultPosition();
	MakeLocal(pos.point);

	// vts::MapStatistics stat = () &map->statistics();
}

FVector AVTSMap::VTSNavToPhysical(FVector point) {
	//double temp[3];
	double p[3];
	UVTSUtil::vector2vts(point,p);
	FVector out = FVector();
	VtsNavigation2Unity(p, out);
	return out;
	//map->convert(p, temp, vts::Srs::Navigation, vts::Srs::Physical);
	//return UVTSUtil::vts2vector(temp);
}

FVector AVTSMap::VTSPhysicalToNav(FVector point) {
	//double temp[3];
	double p[3];
	//UVTSUtil::vector2vts(point, p);
	Unity2VtsNavigation(point, p);
	return UVTSUtil::vts2vector(p);

	//map->convert(p, temp, vts::Srs::Physical, vts::Srs::Navigation);
	//return UVTSUtil::vts2vector(temp);
}

void AVTSMap::LoadMesh(vts::ResourceInfo& info, vts::GpuMeshSpec& spec, const FString debugId) {
	TArray<FVector>* vertices = UVTSUtil::ExtractBuffer3(spec, 0);

	TArray<FVector>* transformed = new TArray<FVector>();
	for (FVector vec : *vertices) {
		transformed->Add(UVTSUtil::SwapYZ.Inverse().TransformPosition(vec));
		//transformed->Add(vec);
	}
	vertices = transformed;

	TArray<FVector>* normals = new TArray<FVector>();
	TArray<int32>* triangles = UVTSUtil::LoadTrianglesIndices(spec);
	TArray<FVector2D>* uvs = new TArray<FVector2D>();
	TArray<FLinearColor>* colors = new TArray<FLinearColor>();
	TArray<FProcMeshTangent>* tangents = new TArray<FProcMeshTangent>();
	for (size_t i = 0; i < vertices->Num(); i++)
	{
		//normals->Add(FVector(0, 0, 1));
		uvs->Add(FVector2D(0, 0));
		colors->Add(FLinearColor(158, 140, 115)); //0xff,0,0
		//tangents->Add(FProcMeshTangent(0, 1, 0));
	}

	UKismetProceduralMeshLibrary::CalculateTangentsForMesh(*vertices, *triangles, *uvs, *normals, *tangents);
	
	std::shared_ptr<FVTSMesh> sp = std::make_shared<FVTSMesh>();
	sp->Vertices = vertices;
	sp->Triangles = triangles;
	sp->Normals = normals;
	sp->UVs = uvs;
	sp->Colors = colors;
	sp->Tangents = tangents;
	sp->DebugId = debugId;

	info.userData = sp;
}

void AVTSMap::LoadGeodata(vts::ResourceInfo& info, vts::GpuGeodataSpec& spec, const FString debugId) {
	GEngine->AddOnScreenDebugMessage(-1, 150.f, FColor::Cyan, debugId);
	if (debugId != "" && spec.type == vts::GpuGeodataSpec::Type::LabelScreen) {
		std::shared_ptr<FVTSLabel> sp = std::make_shared<FVTSLabel>();
		sp->Text = FString(spec.texts[0].c_str());
		spec.positions[0]
		//sp->Mv = UVTSUtil::vts2Matrix(spec.model);
		sp->DebugId = debugId;
		info.userData = sp;
	}
}

void AVTSMap::MakeLocal(double navPt[3]) {
	if (!map->getMapconfigAvailable()) {
		return;
	}
	double p[3];
	map->convert(navPt, p, vts::Srs::Navigation, vts::Srs::Physical);
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
		SetActorLocation(FVector(0, -m, 0)); // altitude
		SetActorRotation(
			FQuat::MakeFromEuler(FVector(0, navPt[0] + 90.0, 0)) * // align to north
			FQuat::FindBetweenVectors(-v, GetActorLocation()) // latlon
		);
	}
}


void AVTSMap::VtsNavigation2Unity(double point[3], FVector out) {
	if (!map->getMapconfigAvailable()) {
		return;
	}

	double physical[3];
	map->convert(point, physical, vts::Srs::Navigation, vts::Srs::Physical);

	// swap YZ
	double navigation[3];
	navigation[0] = physical[0];
	navigation[1] = physical[2];
	navigation[2] = physical[1];

	// convert from (local) vts physical to unity world
	FVector4 point4 = FVector4(navigation[0], navigation[1], navigation[2], 1);
	out = FVector(this->GetTransform().TransformFVector4(point4));
}


void  AVTSMap::Unity2VtsNavigation(FVector vect, double out[3]){
	if (!map->getMapconfigAvailable()) {
		return;
	}
	// convert from unity world to (local) vts physical
	FVector4 point4 = FVector4(vect, 1);
	FVector p = FVector(this->GetTransform().Inverse().TransformFVector4(point4));

	// swap YZ
	double physical[3];
	physical[0] = p.X;
	physical[1] = p.Z;
	physical[2] = p.Y;

	double navigation[3];
	map->convert(physical, navigation, vts::Srs::Physical, vts::Srs::Navigation);
	
	out = navigation;
}

