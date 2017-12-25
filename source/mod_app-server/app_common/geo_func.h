#ifndef _GEO_FUNC_H_
#define _GEO_FUNC_H_

#ifdef __cplusplus
extern "C" {
#endif

double calculateDistSim(double lat1, double lon1, double lat2, double lon2);
double calculateDist(double lat1, double lon1, double lat2, double lon2);
double calculateDistSim1(double lat1, double lng1, double lat2, double lng2);

#ifdef __cplusplus
}
#endif

#endif
