#pragma once

#include "Provided.h"

//***********************************************************************************************************************************************************************************************************************//
// Support
//***********************************************************************************************************************************************************************************************************************//


bool operator<=(const GeoCoord& g1, const GeoCoord& g2);

bool operator>(const GeoCoord& g1, const GeoCoord& g2);

bool operator==(const GeoCoord& g1, const GeoCoord& g2);

bool operator==(const StreetSegment& seg1, const StreetSegment& seg2);

bool operator==(const GeoSegment& seg1, const GeoSegment& seg2);
