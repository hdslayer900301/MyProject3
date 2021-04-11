#include "TbTNav.h"
#include "Provided.h"
#include "Map.h"
#include <queue>
#include <vector>
using namespace std;

TArray<FVector> UTbTNav::CalculatePath(FString mapFile, FString origin, FString destination) {

    
    TArray<FString> infile;
    const TCHAR* stream = *mapFile;
    FFileHelper::LoadANSITextFileToStrings(stream, NULL, infile);
    
    TArray<FVector> vectorMap;

    ///
    ///
    /// 
    /// 
    

    MapLoader map;
    if (map.load(mapFile))
    {
        //cout << "Map loaded..." << endl;
        //cout << "Segments: " << map.getNumSegments() << endl;
    }
    else {
        //cout << "Map not found. Exiting..." << endl;
    }
    AttractionMapper amap;
    amap.init(map);
    GeoCoord geo;
    if (amap.getGeoCoord(origin, geo))
    {
        //cout << "Found origin location..." << endl;
        //cout << geo.latitude << " " << geo.longitude << endl;
    }
    else {
        //cout << "Origin Location not found..." << endl;
    }
    SegmentMapper smap;
    smap.init(map);
    TArray<StreetSegment> v = smap.getSegments(geo);
    if (v.Num() >= 0)
    {
        //cout << "Found Origin Segment..." << endl;
        for (int32 i = 0; i < v.Num(); i++)
        {
            //cout << v[i].streetName << endl;
            //cout << "Segment Start: " << endl;
            //cout << "Latitude: " << v[i].segment.start.latitude << " Longitude: " << v[i].segment.start.longitude << endl;
            //cout << "Segment end: " << endl;
            //cout << "Latitude:" << v[i].segment.end.latitude << " Longitude: " << v[i].segment.start.longitude << endl;;
        }
    }
    else {
        //cout << "No route found..." << endl;
    }
    Navigator nav;
    nav.loadMapData(mapFile);
    TArray<NavSegment> directions;
    FVector startLocation;
    FVector endLocation;
    if (nav.navigate(origin, destination, directions) == NAV_SUCCESS)
    {
        for (auto i : directions) {
            //cout << "Segment start:" << endl << "Latitude: " << i.m_geoSegment.start.latitude << " Longitude: " << i.m_geoSegment.start.longitude << endl << "Segment end:" << endl << " Latitude: " << i.m_geoSegment.end.latitude << " Longitude: " << i.m_geoSegment.end.longitude << endl << "Direction: " << i.m_direction << " on " << i.m_streetName << endl;
            startLocation.Set(i.m_geoSegment.start.latitude, i.m_geoSegment.start.longitude, 0.0);
            endLocation.Set(i.m_geoSegment.end.latitude, i.m_geoSegment.end.longitude, 0.0);
            /*if (i = 0) {
                vectorMap.Add(startLocation);
            }
            else {
                vectorMap.Add(endLocation);
            } */   

            vectorMap.Add(startLocation);
            vectorMap.Add(endLocation);
        }
    }
    return vectorMap;
}