// Fill out your copyright notice in the Description page of Project Settings.


#include "VTSCamera.h"

// Sets default values for this component's properties
UVTSCamera::UVTSCamera()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UVTSCamera::BeginPlay()
{
	Super::BeginPlay();

	vcam = vtsMap->map->createCamera();
	vnav = vcam->createNavigation();
	uecam = GetOwner()->FindComponentByClass<UCameraComponent>();
	GEngine->AddOnScreenDebugMessage(-1, 150.f, FColor::Cyan, uecam->GetName());

	uecamTransform = uecam->GetComponentTransform();
	mapTransform = vtsMap->GetActorTransform(); // local to world transform
	// ...

	FVector2D Result = FVector2D(1, 1);
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize( /*out*/Result);
	}
	vcam->setViewportSize((uint32)Result.X, (uint32)Result.Y);

	uecam->SetWorldLocation(FVector(0, 0, 0));
}


// Called every frame
void UVTSCamera::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (vtsMap->map->getMapconfigReady() && flag) {
		double p[16];
		vcam->getView(p);

		FVector loc = uecam->GetOwner<AActor>()->GetActorLocation();
		FTransform tr = uecam->GetOwner<AActor>()->GetActorTransform();
		uecamTransform = uecam->GetComponentTransform();

		auto v = UVTSUtil::vts2Matrix(p);
		v.RemoveTranslation();

		//FMatrix translation = UVTSUtil::SwapYZ * FMatrix::Identity.ConcatTranslation((loc + vtsMap->PhysicalOrigin) * .01);
		//.ConcatTranslation(vtsMap->PhysicalOrigin)
		FMatrix translation = UVTSUtil::SwapYZ * FMatrix::Identity.ConcatTranslation(loc*.01).ConcatTranslation(vtsMap->PhysicalOrigin) * UVTSUtil::SwapYZ.Inverse();
		//FMatrix transformed = /*UVTSUtil::SwapYZ.Inverse() * */ translation * UVTSUtil::SwapYZ;
		//v = v.ConcatTranslation((loc + vtsMap->PhysicalOrigin) * .01);
		//double t[16];
		//UVTSUtil::matrix2vts(translation, t);
		//vcam->setView(t);
		double pos[3];
		double trans[3];
		UVTSUtil::vector2vts(translation.GetOrigin(), trans);
		vtsMap->map->convert(trans, pos, vts::Srs::Physical, vts::Srs::Navigation);


		
//		double trans[3];
		//double pos[3];
		//UVTSUtil::vector2vts(UVTSUtil::SwapYZ.TransformVector((loc + vtsMap->PhysicalOrigin) * .01), trans);
		//vtsMap->map->convert(trans, pos, vts::Srs::Physical, vts::Srs::Navigation);
		//vnav->setPoint(pos);
		//trans[0] = 10;
		//trans[2] = 1000;
		//vnav->pan(trans);
		vnav->setPoint(pos);
	}


	vcam->renderUpdate();


	// Assuming VTS data control (https://github.com/melowntech/vts-browser-unity-plugin/blob/722f4e591d08d4a0b5aa983e824f177caa4b7904/src/Vts/Scripts/BrowserUtil/VtsCameraBase.cs)
	// double[] Mu = Math.Mul44x44(VtsUtil.U2V44(mapTrans.localToWorldMatrix), VtsUtil.U2V44(VtsUtil.SwapYZ));
	
	if (!vtsMap->map->getMapconfigReady()) {
		return;
	}
	
	/*
	double temp[16];
	vcam->getView(temp);
	FMatrix vcamView = UVTSUtil::vts2Matrix(temp);

	//FMatrix m = UVTSUtil::SwapYZ.Inverse() * vcamView * UVTSUtil::SwapYZ;// *ScaleVTS2UE;
	//m = m.Inverse().ConcatTranslation(UVTSUtil::SwapYZ.Inverse().TransformVector(vtsMap->PhysicalOrigin) * -1);

	FMatrix m = UVTSUtil::SwapYZ.Inverse() * vcamView;// ;
	m = m.ConcatTranslation(UVTSUtil::SwapYZ.Inverse().TransformVector(vtsMap->PhysicalOrigin) * -1);
	m = m * UVTSUtil::SwapYZ;

	uecamTransform = FTransform(m);
	uecam->GetOwner<AActor>()->SetActorTransform(uecamTransform);
	*/
	if (!flag) {
		auto mapPosition = vnav->getPosition();//vtsMap->map->getMapDefaultPosition();
		double pos[3];
		vtsMap->map->convert(mapPosition.point, pos, vts::Srs::Navigation, vts::Srs::Physical);


		FMatrix m = UVTSUtil::SwapYZ.Inverse() * FMatrix::Identity.ConcatTranslation((UVTSUtil::vts2vector(pos) - vtsMap->PhysicalOrigin) * 100) * UVTSUtil::SwapYZ;
		uecam->GetOwner<AActor>()->SetActorTransform(FTransform(m));

		double near;
		double far;
		vcam->suggestedNearFar(near, far);

		uecam->SetOrthoNearClipPlane(near);
		uecam->SetOrthoFarClipPlane(far);
		vcam->setProj(uecam->FieldOfView, uecam->OrthoNearClipPlane, uecam->OrthoFarClipPlane);

		flag = true;
		return;
	}
	
	//mPos = transformed.TransformPosition(FVector::ZeroVector);
	//double camPosition[3];
	//UVTSUtil::vector2vts(mPos, camPosition);
	//vtsMap->map->convert(camPosition, pos, vts::Srs::Physical, vts::Srs::Navigation);


	//double rot[3];
	//rot[2] = 1;
	//vnav->rotate(rot);

	//double pos[3];
	//vnav->getPoint(pos);
	// Get current time
	//double t = FPlatformTime::Seconds()/100;

	//pos[2] = 2000 + 100000 + 100000*FMath::Sin(FMath::Fmod(t,2*PI));
	
	//pos[2] += 10000;
	//vnav->setPoint(pos);

	CameraDraw();
}

void UVTSCamera::CameraDraw() {

	double p[16];
	vcam->getProj(p);
	FMatrix inverseProj = UVTSUtil::vts2Matrix(p).Inverse();

	vcam->getView(p);
	FMatrix inverseView = UVTSUtil::vts2Matrix(p).Inverse();

	TArray<FString> loadedMeshIds;
	loadedMeshes.GetKeys(loadedMeshIds);

	TMap<FString, TArray<vts::DrawSurfaceTask>> tasksByMesh;
	
	auto d = vcam->draws();
	
	for (auto o : d.opaque)
	{
		if (isnan<float>(o.mv[0])) {
			continue;
		}

		FVTSMesh* vtsMesh = (FVTSMesh*)o.mesh.get();
		
		if (!vtsMesh) {
			continue;
		}

		if (tasksByMesh.Contains(vtsMesh->DebugId)) {
			tasksByMesh[vtsMesh->DebugId].Add(o);
		} else {
			tasksByMesh.Add(vtsMesh->DebugId, { o });
		}
	}
	
	TArray<FString> missingMeshIds = {};
	for (auto id : loadedMeshIds)
	{
		if (!tasksByMesh.Contains(id)) {
			missingMeshIds.Add(id);
		}
	}
	loadedMeshIds.Empty();

	for (FString id : missingMeshIds)
	{
		TArray<AActor*>* actors = loadedMeshes[id];
		for (auto a : *actors)
		{
			a->Destroy(); // maybe object pooling?
			
		}
		actors->Empty();
		loadedMeshes.Remove(id);
	}
	missingMeshIds.Empty();

	TArray<FString> incoming;
	tasksByMesh.GetKeys(incoming);
	for(auto id : incoming)
	{
		auto index = 0;
		
		TArray<AActor*>* actors = loadedMeshes.FindOrAdd(id);
		if (actors == nullptr) {
			actors = new TArray<AActor*>();
			loadedMeshes.Add(id, actors);
		}
		
		for (auto o : tasksByMesh[id])
		{
			FMatrix m = UVTSUtil::SwapYZ.Inverse() * (UVTSUtil::vts2Matrix(o.mv) * inverseView) * UVTSUtil::SwapYZ * ScaleVTS2UE;
			m = m.ConcatTranslation(UVTSUtil::SwapYZ.Inverse().TransformVector(vtsMap->PhysicalOrigin * 100) * -1);
			FTransform t = FTransform(m);

			if (actors->Num() > index) {
				AActor* tile = actors->GetData()[index];
				UpdateTile(tile, t);
			}else {
				FVTSMesh* mesh = (FVTSMesh*) o.mesh.get();
				AActor* tile = InitTile(mesh, t);
				actors->Add(tile);
			}
			index++;
		}
		if (index > actors->Num()) {
			for (auto i = actors->Num(); i < index; i++)
			{
				actors->GetData()[i]->Destroy();
				actors->RemoveAt(i);
			}
		}
	}
	incoming.Empty();
}

AActor* UVTSCamera::InitTile(FVTSMesh* vtsMesh, FTransform transform) {

	FActorSpawnParameters SpawnParams;
	AActor* tile = GetWorld()->SpawnActor<AActor>(TileBP, transform, SpawnParams);

	TArray<UProceduralMeshComponent*> Comps;

	tile->GetComponents(Comps);
	UProceduralMeshComponent* TargetMesh;
	if (Comps.Num() > 0)
	{
		TargetMesh = Comps[0];
		TargetMesh->CreateMeshSection_LinearColor(
			0,
			*vtsMesh->Vertices,
			*vtsMesh->Triangles,
			*vtsMesh->Normals,
			*vtsMesh->UVs,
			*vtsMesh->Colors,
			*vtsMesh->Tangents,
			false
		);
	}
	return tile;
}

void UVTSCamera::UpdateTile(AActor* tile, FTransform transform) {
	tile->SetActorTransform(transform);
}