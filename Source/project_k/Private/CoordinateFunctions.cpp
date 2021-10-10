// Fill out your copyright notice in the Description page of Project Settings.


#include "CoordinateFunctions.h"
#include "Math/UnrealMathVectorConstants.h"

void UCoordinateFunctions::ECEFToLLA(
    const FVector& position,
    FVector& lla //Return Lat, Lon, Altitude in that order
) {
    double e = 0.0934; // Eccenrticity of Mars ( ellipsoid model https://en.wikipedia.org/wiki/Mars)
    double a = 3396200; // WGS-84 semi - major axis
    double e2 = 0.0934 * 0.0934;

    double a1 = a * e2;
    double a2 = a1 * a1;
    double a3 = a1 * e2 / 2;
    double a4 = 2.5 * a2;
    double a5 = a1 + a3;
    double a6 = 1 - e2;


    double zp, w2, w, r2, r, s2, c2, s, c, ss;
    double g, rg, rf, u, v, m, f, p;

    zp = FMath::Abs(position.X);
    w2 = FMath::Square(position.X) + FMath::Square(position.Y);
    w = FMath::Sqrt(w2);
    r2 = w2 + FMath::Square(position.Z);
    r = FMath::Sqrt(r2);
    lla.Y = FMath::Atan2(position.Y, position.X);       //Lon (final)
    s2 = FMath::Square(position.Z) / r2;
    c2 = w2 / r2;
    u = a2 / r;
    v = a3 - a4 / r;

    if (c2 > 0.3) {
        s = (zp / r) * (1.0 + c2 * (a1 + u + s2 * v) / r);
        lla.X = FMath::Asin(s);      //Lat
        ss = s * s;
        c = FMath::Sqrt(1.0 - ss);
    }
    else {
        c = (w / r) * (1.0 - s2 * (a5 - u - c2 * v) / r);
        lla.X = FMath::Acos(c);      //Lat
        ss = 1.0 - c * c;
        s = FMath::Sqrt(ss);
    }

    g = 1.0 - e2 * ss;
    rg = a / FMath::Sqrt(g);
    rf = a6 * rg;
    u = w - rg * c;
    v = zp - rf * s;
    f = c * u + s * v;
    m = c * v - s * u;
    p = m / (rf / g + f);
    lla.X = lla.X + p;      //Lat
    lla.Z = f + m * p / 2.0;     //Altitude
    if (position.Z < 0.0) {
        lla.X *= -1.0;     //Lat
    }
};

void UCoordinateFunctions::ECEFToLLA2(
    const FVector& position,
    FVector& lla //Return Lat, Lon, Altitude in that order
) {
    // x, y and z are scalars or vectors in meters
    double x = position.X;
    double y = position.Y;
    double z = position.Z;

    // Earth 
    
    double a = 6378137;
    double e = 8.181919084261345e-2;
    
    /*
    double e = 0.0934; // Eccenrticity of Mars ( ellipsoid model https://en.wikipedia.org/wiki/Mars)
    double a = 3396200; // WGS-84 semi - major axis
*/
    double a_sq = FMath::Square(a);
    double e_sq = FMath::Square(e);

    double f = 1 / 298.257223563;
    double b = a * (1 - f);

    // calculations:;
    double r = FMath::Sqrt(FMath::Square(x) + FMath::Square(y));
    double ep_sq = (FMath::Square(a) - FMath::Square(b)) / FMath::Square(b);
    double ee = (FMath::Square(a) - FMath::Square(b));
    f = (54 * FMath::Square(b)) * (FMath::Square(z));
    double g = FMath::Square(r) + (1 - e_sq) * (FMath::Square(z)) - e_sq * ee * 2;
    double c = (FMath::Square(e_sq)) * f * FMath::Square(r) / FMath::Pow(g, 3);
    double s = FMath::Pow((1 + c + FMath::Sqrt(FMath::Square(c) + 2 * c)), (1 / 3.));
    double  p = f / (3. * FMath::Square(g) * FMath::Square(s + (1. / s) + 1));
    double q = FMath::Sqrt(1 + 2 * p * FMath::Square(e_sq));
    double r_0 = -(p * e_sq * r) / (1 + q) + FMath::Sqrt(0.5 * (FMath::Square(a)) * (1 + (1. / q)) - p * (FMath::Square(z)) * (1 - e_sq) / (q * (1 + q)) - 0.5 * p * (FMath::Square(r)));
    double u = FMath::Sqrt(FMath::Square(r - e_sq * r_0) + FMath::Square(z));
    double v = FMath::Sqrt(FMath::Square(r - e_sq * r_0) + (1 - e_sq) * FMath::Square(z));
    double z_0 = (FMath::Square(b)) * z / (a * v);
    double h = u * (1 - FMath::Square(b) / (a * v));
    double phi = FMath::Atan((z + ep_sq * z_0) / r);
    double lambd = FMath::Atan2(y, x);
    double lon = phi * 180 / PI;
    double lat = lambd * 180 / PI;
    double alt = h;

    lla.X = lon;
    lla.Y = lat;
    lla.Z = alt;

};

void UCoordinateFunctions::LLAToECEF(
    const FVector& lla,
    FVector& position
)
{

    // Earth 
    
    double a = 6378137;
    double e = 8.181919084261345e-2;

    /*
    double e = 0.0934; // Eccenrticity of Mars ( ellipsoid model https://en.wikipedia.org/wiki/Mars)
    double a = 3396200; // WGS-84 semi - major axis
    */
    double a_sq = FMath::Square(a);
    double e_sq = FMath::Square(e);
    double b_sq = a_sq * (1 - e_sq);

    double lon = lla.X * PI / 180;
    double lat = lla.Y * PI / 180;
    double alt = lla.Z;

    double N = a / FMath::Sqrt(1 - e_sq * FMath::Square(FMath::Sin(lat)));
    double x = (N + alt) * FMath::Cos(lat) * FMath::Cos(lon);
    double y = (N + alt) * FMath::Cos(lat) * FMath::Sin(lon);
    double z = ((b_sq / a_sq) * N + alt) * FMath::Sin(lat);

    position.X = x;
    position.Y = y;
    position.Z = z;

};

void UCoordinateFunctions::FlatToLLA(
    const FVector& position, // Flat Earth position coordinates, in meters.
    const FVector& LLA0, // Reference location, in degrees, of latitude and longitude, for the origin of the estimation and the origin of the flat Earth coordinate system.
    FVector& lla //geodetic coordinates (longitude, latitude, and altitude), in [degrees, degrees, meters].
)
{
    //double re = 6378137; // WGS-84 semi - major axis
    double re = 3396200; // Mars equatorial radius 
    double re_c = re * FMath::Cos((PI / 180) * FMath::Abs(LLA0.X));
    double lon = position.X * 180 / (PI * re_c) + LLA0.Y;
    double lat = position.Y * 180 / (PI * re) + LLA0.X;
    double alt = position.Z + LLA0.Z;

    lla.X = lon;
    lla.Y = lat;
    lla.Z = alt;
};


void UCoordinateFunctions::LLAToFlat(
    // NOT FINISHED
    const FVector& lla, //geodetic coordinates (latitude, longitude, and altitude), in [degrees, degrees, meters].
    const FVector& LLA0, // Reference location, in degrees, of latitude and longitude, for the origin of the estimation and the origin of the flat Earth coordinate system.
    FVector& position // Flat Earth position coordinates, in meters.
)
{
    double a = 3396190; // IAU2000 Mars https://spatialreference.org/ref/iau2000/mars-2000/mapfile/
    double f = 0.00589; // Mars 1 / 298.257223563;** Earth ref: https://en.wikipedia.org/wiki/Reference_ellipsoid#:~:text=In%201687%20Isaac%20Newton%20published,he%20termed%20an%20oblate%20spheroid.

    double lat = lla.X * PI / 180;
    double lon = lla.Y * PI / 180;
    double alt = lla.Z;
    double lat0 = LLA0.X * PI / 180;
    double lon0 = LLA0.Y * PI / 180;
    double alt0 = LLA0.Z;

    double e = FMath::Sqrt(f * (2 - f));
    double N = a / (FMath::Sqrt(1 - FMath::Pow(e, 2) * FMath::Pow(FMath::Sin(lat), 2)));
    double N0 = a / (FMath::Sqrt(1 - FMath::Pow(e, 2) * FMath::Pow(FMath::Sin(lat), 2)));
    double x_ec = (alt + N) * FMath::Cos(lat) * FMath::Cos(lon);
    double y_ec = (alt + N) * FMath::Cos(lat) * FMath::Sin(lon);
    double z_ec = (alt + (1 - FMath::Pow(e, 2)) * N) * FMath::Sin(lat);
    double x_ec0 = (alt0 + N0) * FMath::Cos(lat0) * FMath::Cos(lon0);
    double y_ec0 = (alt0 + N0) * FMath::Cos(lat0) * FMath::Sin(lon0);
    double z_ec0 = (alt0 + (1 - FMath::Pow(e, 2)) * N) * FMath::Sin(lat0);


}

void UCoordinateFunctions::UE4ToECEF(
    const FVector& ue4,
    FVector& ecef
)
{
    ecef.X = ue4.Y;
    ecef.Y = ue4.X;
    ecef.Z = ue4.Z;
}

void UCoordinateFunctions::ECEFToUE4(
    const FVector& ecef,
    FVector& ue4
)
{
    ue4.X = ecef.Y;
    ue4.Y = ecef.X;
    ue4.Z = ecef.Z;
}