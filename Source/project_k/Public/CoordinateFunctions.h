// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CoordinateFunctions.generated.h"


/**
 *
 */
UCLASS()
class PROJECT_K_API UCoordinateFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	//Convert Earth-Centered-Earth-Fixed (ECEF) to lat, Lon, Altitude
	//Input is a three element array containing x, y, z in meters
	//Returned array contains lat and lon in radians, and altitude in meters
	UFUNCTION(BlueprintCallable)
		static void ECEFToLLA(
			const FVector& position,
			FVector& lla
		);
	UFUNCTION(BlueprintCallable)
		static void LLAToECEF(
			const FVector& lla,
			FVector& position
		);


	UFUNCTION(BlueprintCallable)
		static void ECEFToLLA2(
			const FVector& position,
			FVector& lla
		);

	// Convert xyz position to LLA
	UFUNCTION(BlueprintCallable)
		static void FlatToLLA(
			const FVector& position, // Flat Earth position coordinates, in meters.
			const FVector& LLA0, // Reference location, in degrees, of latitude and longitude, for the origin of the estimation and the origin of the flat Earth coordinate system.
			FVector& lla //geodetic coordinates (latitude, longitude, and altitude), in [degrees, degrees, meters].
		);

	// Convert LLA position to XYZ
	UFUNCTION(BlueprintCallable)
		static void LLAToFlat(
			const FVector& lla, //geodetic coordinates (latitude, longitude, and altitude), in [degrees, degrees, meters].
			const FVector& LLA0, // Reference location, in degrees, of latitude and longitude, for the origin of the estimation and the origin of the flat Earth coordinate system.
			FVector& position // Flat Earth position coordinates, in meters.
		);


	UFUNCTION(BlueprintCallable)
		static void UE4ToECEF(
			const FVector& ue4,
			FVector& ecef
		);

	UFUNCTION(BlueprintCallable)
		static void ECEFToUE4(
			const FVector& ecef,
			FVector& ue4
		);

};

