#include "Provided.h"
#include "Misc/FileHelper.h"
#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "GenericPlatform/GenericPlatform.h"

//***********************************************************************************************************************************************************************************************************************//
// Map Loader
//***********************************************************************************************************************************************************************************************************************//

class MapLoaderImpl
{
public:
    MapLoaderImpl();
    ~MapLoaderImpl();
    bool load(FString mapFile);
    size_t getNumSegments() const;
    bool getSegment(size_t segNum, StreetSegment& seg) const;
private:
    TArray<StreetSegment> segments;
    int32 numSeg;
    TArray<FString> processToStrings(FString line);
    GeoSegment processToGeoSegment(FString line);
    GeoCoord processToGeoCoord(FString line);
    Attraction processToAttraction(FString line);
};

MapLoaderImpl::MapLoaderImpl()
{
    numSeg = 0;
}

MapLoaderImpl::~MapLoaderImpl()
{
}

bool MapLoaderImpl::load(FString mapFile)
{
    TArray<FString> infile;
    const TCHAR* stream = *mapFile;
    FFileHelper::LoadANSITextFileToStrings(stream, NULL, infile);
    if (!stream) // error opening file
        return false;
    FString s;
    for (int32 i = 0; i <= infile.Num(); i++)
    {
        // get street name
        StreetSegment seg;
        seg.streetName = s;

        // get street segment
        s = infile[i];
        seg.segment = processToGeoSegment(s);

        // get attractions if any
        s = infile[i];
        int32 numAtt = FCString::Atoi(*s);
        for (int32 j = 0; j < numAtt; j++)
        {
            s = infile[j];
            seg.attractions.Push(processToAttraction(s));
        }

        // add segment to vector of street segments
        segments.Push(seg);
        numSeg++;
    }
    return true;
}

size_t MapLoaderImpl::getNumSegments() const
{
    return numSeg;
}

bool MapLoaderImpl::getSegment(size_t segNum, StreetSegment& seg) const
{
    if (segNum >= getNumSegments())
        return false;
    seg = segments[segNum];
    return true;
}

TArray<FString> MapLoaderImpl::processToStrings(FString line)
{
    int32 i = 0;
    int32 c = 0;
    FString str = "";
    TArray<FString> v;
    v.Push(str);
    while (i < line.Len())
    {
        if (line[i] == ',' || line[i] == ' ')
        {
            if ((line[i] == ' ' && line[i - 1] != ',') || line[i] == ',')
            {
                c++;
                v.Push(str);
            }
        }
        else
            v[c] += line[i];
        i++;
    }
    return v;
}

GeoSegment MapLoaderImpl::processToGeoSegment(FString line)
{
    TArray<FString> v = processToStrings(line);
    return GeoSegment(GeoCoord(v[0], v[1]), GeoCoord(v[2], v[3]));
}

GeoCoord MapLoaderImpl::processToGeoCoord(FString line)
{
    TArray<FString> v = processToStrings(line);
    return GeoCoord(v[0], v[1]);
}

Attraction MapLoaderImpl::processToAttraction(FString line)
{
    int32 c = 0;
    Attraction a;
    while (line[c] != '|')
    {
        a.name += line[c];
        c++;
    }
    c++;
    a.geocoordinates = processToGeoCoord(line.Mid(c, line.Len() - c));
    return a;
}

//******************** MapLoader functions ************************************

// These functions simply delegate to MapLoaderImpl's functions.
// You probably don't want to change any of this code.

MapLoader::MapLoader()
{
    m_impl = new MapLoaderImpl;
}

MapLoader::~MapLoader()
{
    delete m_impl;
}

bool MapLoader::load(FString mapFile)
{
    return m_impl->load(mapFile);
}

size_t MapLoader::getNumSegments() const
{
    return m_impl->getNumSegments();
}

bool MapLoader::getSegment(size_t segNum, StreetSegment& seg) const
{
    return m_impl->getSegment(segNum, seg);
}
