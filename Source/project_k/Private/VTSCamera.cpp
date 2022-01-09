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

	vcam->renderUpdate();


	// Assuming VTS data control (https://github.com/melowntech/vts-browser-unity-plugin/blob/722f4e591d08d4a0b5aa983e824f177caa4b7904/src/Vts/Scripts/BrowserUtil/VtsCameraBase.cs)
	// double[] Mu = Math.Mul44x44(VtsUtil.U2V44(mapTrans.localToWorldMatrix), VtsUtil.U2V44(VtsUtil.SwapYZ));
	
	
	FMatrix Mu = mapTransform.ToMatrixWithScale() * UVTSUtil::SwapYZ;
	
	double temp[16];
	vcam->getView(temp);
	FMatrix vcamView = UVTSUtil::vts2Matrix(temp);

	vcam->getProj(temp);
	FMatrix vcamProj = UVTSUtil::vts2Matrix(temp);

	//FMatrix a = (vcamView * Mu).Inverse();
	FMatrix a = UVTSUtil::SwapYZ.Inverse() * vcamView * UVTSUtil::SwapYZ;// * ScaleVTS2UE;
	a = a.Inverse().ConcatTranslation(UVTSUtil::SwapYZ.Inverse().TransformVector(vtsMap->PhysicalOrigin) * -1);
	

	//a.ScaleTranslation(FVector(1, 1, -1));
	uecamTransform = FTransform(a);
	//uecamTransform.SetLocation(FVector(0,0,0));

	//FRotator r = uecamTransform.GetRotation().Rotator();
	//r.Roll += 90;
	//uecamTransform.SetRotation(r.Quaternion());

	
	//uecamTransform = UVTSUtil::matrix2Transform(a);

	if (!vtsMap->map->getMapconfigReady()) {
		return;
	}

	uecam->GetOwner<AActor>()->SetActorTransform(uecamTransform);

	double near;
	double far;
	vcam->suggestedNearFar(near, far);

	uecam->SetOrthoNearClipPlane(near);
	uecam->SetOrthoFarClipPlane(far);
	vcam->setProj(uecam->FieldOfView, uecam->OrthoNearClipPlane, uecam->OrthoFarClipPlane);
	
	double rot[3];
	rot[0] = 1;
	vnav->rotate(rot);

	CameraDraw();
}

void UVTSCamera::CameraDraw() {

	double p[16];
	vcam->getProj(p);
	FMatrix inverseProj = UVTSUtil::vts2Matrix(p).Inverse();

	vcam->getView(p);
	FMatrix inverseView = UVTSUtil::vts2Matrix(p).Inverse();

	TArray<FVTSMesh*> loadedMeshIds;
	loadedMeshes.GetKeys(loadedMeshIds);

	TMap<FVTSMesh*, TArray<vts::DrawSurfaceTask>> tasksByMesh;
	
	auto d = vcam->draws();
	for (auto o : d.opaque)
	{
		if (isnan<float>(o.mv[0])) {
			continue;
		}

		FVTSMesh* vtsMesh = (FVTSMesh*)o.mesh.get();

		if (!vtsMesh) {
			return;
		}

		if (tasksByMesh.Contains(vtsMesh)) {
			tasksByMesh[vtsMesh].Add(o);
		} else {
			tasksByMesh.Add(vtsMesh, { o });
		}
	}
	
	TArray<FVTSMesh*> missingMeshes = {};
	for (auto id : loadedMeshIds)
	{
		if (!tasksByMesh.Contains(id)) {
			missingMeshes.Add(id);
		}
	}
	loadedMeshIds.Empty();

	for (FVTSMesh* toDestroy : missingMeshes)
	{
		for (auto a : loadedMeshes[toDestroy])
		{
			a->Destroy(); // maybe object pooling?
		}
		loadedMeshes[toDestroy].Empty();
		loadedMeshes.Remove(toDestroy);
	}
	missingMeshes.Empty();

	TArray<FVTSMesh*> incoming;
	tasksByMesh.GetKeys(incoming);
	for(auto vtsMesh : incoming)
	{
		auto index = 0;

		TArray<AActor*>* list = loadedMeshes.Find(vtsMesh);
		if (list == nullptr) {
			list = new TArray<AActor*>();
			loadedMeshes.Add(vtsMesh, *list);
		}

		for (auto o : tasksByMesh[vtsMesh])
		{
			FMatrix m = UVTSUtil::SwapYZ.Inverse() * (UVTSUtil::vts2Matrix(o.mv) * inverseView) * UVTSUtil::SwapYZ * ScaleVTS2UE;
			m = m.ConcatTranslation(UVTSUtil::SwapYZ.Inverse().TransformVector(vtsMap->PhysicalOrigin * 100) * -1);
			FTransform t = FTransform(m);

			if (list->Num() > index) {
				AActor* tile = list->GetData()[index];
				UpdateTile(tile, t);
			}else {
				AActor* tile = InitTile(vtsMesh, t);
				list->Add(tile);
			}
			index++;
		}
		if (index > list->Num()) {
			for (auto i = list->Num(); i < index; i++)
			{
				list->GetData()[i]->Destroy();
				list->RemoveAt(i);
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