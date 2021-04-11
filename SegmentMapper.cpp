#include "Provided.h"
#include "Map.h"
#include "Containers/Array.h"

//***********************************************************************************************************************************************************************************************************************//
// Segment Mapper
//***********************************************************************************************************************************************************************************************************************//



class SegmentMapperImpl
{
public:
    SegmentMapperImpl();
    ~SegmentMapperImpl();
    void init(const MapLoader& ml);
    TArray<StreetSegment> getSegments(const GeoCoord& gc) const;
private:
    Map<GeoCoord, TArray<StreetSegment>> segmentMap;
};

SegmentMapperImpl::SegmentMapperImpl()
{
}

SegmentMapperImpl::~SegmentMapperImpl()
{
}

void SegmentMapperImpl::init(const MapLoader& ml)
{
    StreetSegment seg;
    for (int32 i = 0; i < ml.getNumSegments(); i++)
    {
        ml.getSegment(i, seg);

        GeoCoord coord = seg.segment.start;

        // find all segments related to start of current segment
        TArray<StreetSegment>* v = segmentMap.find(coord);
        if (v == nullptr)
        {
            TArray<StreetSegment> sn;
            sn.Push(seg);
            segmentMap.associate(coord, sn);
        }

        else
            v->Push(seg);

        // find all segments related to end of current segment
        coord = seg.segment.end;
        v = segmentMap.find(coord);
        if (v == nullptr)
        {
            TArray<StreetSegment> sn;
            sn.Push(seg);
            segmentMap.associate(coord, sn);
        }
        else
            v->Push(seg);

        // find all segments related to each attraction
        for (auto j : seg.attractions)
        {
            coord = j.geocoordinates;
            v = segmentMap.find(coord);
            if (v == nullptr)
            {
                TArray<StreetSegment> sn;
                sn.Push(seg);
                segmentMap.associate(coord, sn);
            }
            else
                v->Push(seg);
        }
    }
}

TArray<StreetSegment> SegmentMapperImpl::getSegments(const GeoCoord& gc) const
{
    const TArray<StreetSegment>* v;
    v = segmentMap.find(gc);
    if (v != nullptr)
        return *v;
    else // no segments found
    {
        TArray<StreetSegment> segments;
        return segments;
    }
}

//******************** SegmentMapper functions ********************************

// These functions simply delegate to SegmentMapperImpl's functions.
// You probably don't want to change any of this code.

SegmentMapper::SegmentMapper()
{
    m_impl = new SegmentMapperImpl;
}

SegmentMapper::~SegmentMapper()
{
    delete m_impl;
}

void SegmentMapper::init(const MapLoader& ml)
{
    m_impl->init(ml);
}

TArray<StreetSegment> SegmentMapper::getSegments(const GeoCoord& gc) const
{
    return m_impl->getSegments(gc);
}
