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
		FTransform tr = uecam->GetOwner<AActor>()->GetActorTransform();
		uecamTransform = uecam->GetComponentTransform();

		FVector o = tr.GetTranslation();
		FMatrix mc = UVTSUtil::SwapYZ * FMatrix::Identity.ConcatTranslation((o * .01) + vtsMap->PhysicalOrigin);// *UVTSUtil::SwapYZ.Inverse();
		FVector mco = mc.GetOrigin();
		double pos[3];
		double trans[3];
		UVTSUtil::vector2vts(mco, trans);
		vtsMap->map->convert(trans, pos, vts::Srs::Physical, vts::Srs::Navigation);
		vnav->setPoint(pos);
		
		double rot[3];
		UVTSUtil::rotator2vts(uecam->GetOwner<AActor>()->GetActorRotation(), rot);
		vnav->setRotation(rot);
		
	}

	vcam->renderUpdate();

	if (!vtsMap->map->getMapconfigReady()) {
		return;
	}
	
	if (!flag) {
		auto mapPosition = vnav->getPosition();
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

		double rot[3];
		vnav->getRotation(rot);
		uecam->GetOwner<AActor>()->SetActorRotation(UVTSUtil::vts2rotator(rot));

		flag = true;
		return;
	}
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





















 
