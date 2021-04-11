#include "Provided.h"
#include "Map.h"
#include "GenericPlatform/GenericPlatform.h"

class AttractionMapperImpl
{
public:
    AttractionMapperImpl();
    ~AttractionMapperImpl();
    void init(const MapLoader& ml);
    bool getGeoCoord(FString attraction, GeoCoord& gc) const;
private:
    Map<class FString, GeoCoord> attractionMap;
};

AttractionMapperImpl::AttractionMapperImpl()
{
}

AttractionMapperImpl::~AttractionMapperImpl()
{
}

void AttractionMapperImpl::init(const MapLoader& ml)
{
    StreetSegment seg;
    for (int32 i = 0; i < ml.getNumSegments(); i++) // get every segment
    {
        ml.getSegment(i, seg);
        for (auto j : seg.attractions) // for every attraction on this segment, map to this segment
        {
            for (int32 c = 0; c < j.name.Len(); c++)
            {
                j.name[c] = tolower(j.name[c]);
            }
            attractionMap.associate(j.name, j.geocoordinates);
        }
    }
}

bool AttractionMapperImpl::getGeoCoord(FString attraction, GeoCoord& gc) const
{
    for (int32 c = 0; c < attraction.Len(); c++)
        attraction[c] = tolower(attraction[c]); // change attraction name to lower case for case-insensitivity
    const GeoCoord* at = attractionMap.find(attraction);

    if (at != nullptr) // attraction exists
    {
        gc = *at;
        return true;
    }
    return false;
}

//******************** AttractionMapper functions *****************************

// These functions simply delegate to AttractionMapperImpl's functions.
// You probably don't want to change any of this code.

AttractionMapper::AttractionMapper()
{
    m_impl = new AttractionMapperImpl;
}

AttractionMapper::~AttractionMapper()
{
    delete m_impl;
}

void AttractionMapper::init(const MapLoader& ml)
{
    m_impl->init(ml);
}

bool AttractionMapper::getGeoCoord(FString attraction, GeoCoord& gc) const
{
    return m_impl->getGeoCoord(attraction, gc);
}
