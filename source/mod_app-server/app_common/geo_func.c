#include "app_common/geo_func.h"
#include <math.h>

#define EARTH_RAD 6378.137
#define PI 3.141592653589793
#define PI_PER_DEG 0.017453292519943

double calculateDistSim(double lat1, double lon1, double lat2, double lon2)
{
    double d_lat = lat1 - lat2;
    double d_lon = lon1 - lon2;
    return sqrt(12392.1424 * d_lat * d_lat + 10000 * d_lon * d_lon);
}

double calculateDist(double lat1, double lon1, double lat2, double lon2)
{
    double radLat1 = lat1 * PI_PER_DEG;
    double radLat2 = lat2 * PI_PER_DEG;
    double a = radLat1 - radLat2;
    double b = (lon1 - lon2) * PI_PER_DEG;
    double sin_a_2 = sin(a/2);
    double sin_b_2 = sin(b/2);
    double s = 2 * asin( sqrt(sin_a_2*sin_a_2
                + cos(radLat1)*cos(radLat2)
                * sin_b_2 * sin_b_2));
    s *= EARTH_RAD;
    return s;
}

double calculateDistSim1(double lat1, double lng1, double lat2, double lng2)
{
    static const double EARTH_R = 6367000.0;
    // refer to http://tech.meituan.com/lucene-distance.html
    double dx = lng1 - lng2;
    double dy = lat1 - lat2;
    double b = (lat1 + lat2) / 2;
    double Lx = PI_PER_DEG * dx * EARTH_R * cos(PI_PER_DEG * b);
    double Ly = EARTH_R * PI_PER_DEG * dy;
    return sqrt(Lx * Lx + Ly * Ly) / 1000.0;
    
}
