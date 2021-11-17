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
	uecam = GetOwner()->FindComponentByClass<UCameraComponent>();
	GEngine->AddOnScreenDebugMessage(-1, 150.f, FColor::Cyan, uecam->GetName());

	uecamTransform = uecam->GetComponentTransform();
	mapTransform = vtsMap->GetActorTransform(); // local to world transform
	// ...
	
}


// Called every frame
void UVTSCamera::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FVector2D Result = FVector2D(1, 1);
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize( /*out*/Result);
	}
	vcam->setViewportSize((uint32)Result.X, (uint32)Result.Y);

	// Assuming VTS data control (https://github.com/melowntech/vts-browser-unity-plugin/blob/722f4e591d08d4a0b5aa983e824f177caa4b7904/src/Vts/Scripts/BrowserUtil/VtsCameraBase.cs)
	// double[] Mu = Math.Mul44x44(VtsUtil.U2V44(mapTrans.localToWorldMatrix), VtsUtil.U2V44(VtsUtil.SwapYZ));

	FMatrix Mu = mapTransform.ToMatrixWithScale() * UVTSUtil::SwapYZ;
	
	double temp[16];
	vcam->getView(temp);
	FMatrix vcamView = UVTSUtil::vts2Matrix(temp);

	FMatrix a = (vcamView * Mu).Inverse();
	a.ScaleTranslation(FVector(1, 1, -1));
	uecamTransform = FTransform(a);
	//uecamTransform = UVTSUtil::matrix2Transform(a);

	double near;
	double far;
	vcam->suggestedNearFar(near, far);

	uecam->SetOrthoNearClipPlane(near);
	uecam->SetOrthoFarClipPlane(far);
	vcam->setProj(uecam->FieldOfView, uecam->OrthoNearClipPlane, uecam->OrthoFarClipPlane);

	double p[16];
	vcam->getProj(p);
	FMatrix proj = UVTSUtil::vts2Matrix(p);
	// uecam->SetProjection matrix 
	/*
	* TODO
	* [] - Figure out how to set the projection matrix of a UE camera
	* [] - Re implements the draw calls from vts camera base
	*/
}

