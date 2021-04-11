#include "TbTNav.h"
#include <queue>
#include <vector>
using namespace std;

TArray<FVector> UTbTNav::Navigate(FString mapFile, FString origin, FString destination) {

	TArray<FString> infile;
	const TCHAR* stream = *mapFile;
	FFileHelper::LoadANSITextFileToStrings(stream, NULL, infile);
    TArray<FVector> vectorMap;

	///
	///
    /// 
    /// 


    UMapLoader map;
    if (map.load(mapFile))
    {
        //cout << "Map loaded..." << endl;
        //cout << "Segments: " << map.getNumSegments() << endl;
    }
    else {
        //cout << "Map not found. Exiting..." << endl;
    }
    UAttractionMapper amap;
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
    USegmentMapper smap;
    smap.init(map);
    TArray<StreetSegment> v = smap.getSegments(geo);
    if (v.Num() >= 0)
    {
        //cout << "Found Origin Segment..." << endl;
        for (int i = 0; i < v.Num(); i++)
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
    UNavigator nav;
    nav.loadMapData(mapFile);
    TArray<NavSegment> directions;
    FVector startLocation;
    FVector endLocation;
    if (nav.navigate(origin, destination, directions) == NAV_SUCCESS)
    {
        for (auto i : directions) {
            //cout << "Segment start:" << endl << "Latitude: " << i.m_geoSegment.start.latitude << " Longitude: " << i.m_geoSegment.start.longitude << endl << "Segment end:" << endl << " Latitude: " << i.m_geoSegment.end.latitude << " Longitude: " << i.m_geoSegment.end.longitude << endl << "Direction: " << i.m_direction << " on " << i.m_streetName << endl;
            startLocation.Set(i.m_geoSegment.start.latitude, i.m_geoSegment.start.longitude , 0.0);
            endLocation.Set(i.m_geoSegment.end.latitude, i.m_geoSegment.end.longitude, 0.0);

            vectorMap.Add(startLocation);
        }
    }

	return vectorMap;
}


//***********************************************************************************************************************************************************************************************************************//
// Support
//***********************************************************************************************************************************************************************************************************************//


bool operator<=(const FGeoCoord& g1, const FGeoCoord& g2)
{
    if (g1.latitude < g2.latitude)
        return true;
    else if (g1.latitude == g2.latitude)
        return g1.longitude <= g2.longitude;
    else
        return false;
}

bool operator>(const FGeoCoord& g1, const FGeoCoord& g2)
{
    if (g1.latitude > g2.latitude)
        return true;
    else if (g1.latitude == g2.latitude)
        return g1.longitude > g2.longitude;
    else
        return false;
}

bool operator==(const FGeoCoord& g1, const FGeoCoord& g2)
{
    return (g1.latitude == g2.latitude) && (g1.longitude == g2.longitude);
}

bool operator==(const FStreetSegment& seg1, const FStreetSegment& seg2)
{
    return (seg1.segment.start == seg2.segment.start) && (seg1.segment.end == seg2.segment.end);
}

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
    bool getSegment(size_t segNum, FStreetSegment& seg) const;
private:
    TArray<FStreetSegment> segments;
    int32 numSeg;
    TArray<FString> processToStrings(FString line);
    FGeoSegment processToGeoSegment(FString line);
    FGeoCoord processToGeoCoord(FString line);
    FAttraction processToAttraction(FString line);
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

bool MapLoaderImpl::getSegment(size_t segNum, FStreetSegment& seg) const
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

FGeoSegment MapLoaderImpl::processToGeoSegment(FString line)
{
    TArray<FString> v = processToStrings(line);
    return FGeoSegment(FGeoCoord(v[0], v[1]), GeoCoord(v[2], v[3]));
}

FGeoCoord MapLoaderImpl::processToGeoCoord(FString line)
{
    TArray<FString> v = processToStrings(line);
    return FGeoCoord(v[0], v[1]);
}

FAttraction MapLoaderImpl::processToAttraction(FString line)
{
    int32 c = 0;
    FAttraction a;
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

UMapLoader::UMapLoader()
{
    m_impl = new MapLoaderImpl;
}

UMapLoader::~UMapLoader()
{
    delete m_impl;
}

bool UMapLoader::load(FString mapFile)
{
    return m_impl->load(mapFile);
}

size_t UMapLoader::getNumSegments() const
{
    return m_impl->getNumSegments();
}

bool UMapLoader::getSegment(size_t segNum, FStreetSegment& seg) const
{
    return m_impl->getSegment(segNum, seg);
}


//***********************************************************************************************************************************************************************************************************************//
// Navigator
//***********************************************************************************************************************************************************************************************************************//


class NavigatorImpl
{
public:
    NavigatorImpl();
    ~NavigatorImpl();
    bool loadMapData(FString mapFile);
    NavResult navigate(FString start, FString end, TArray<FNavSegment>& directions) const;
private:
    UMapLoader map;
    USegmentMapper smap;
    UAttractionMapper amap;
    struct Node {
        FGeoCoord coord;
        FStreetSegment seg;
        bool operator>(const Node& other) const;
        bool operator<=(const Node& other) const;
        bool operator==(const Node& other) const;
        double fScore;
    };
    void reconstructPath(UTbTNavigationSystem<Node, Node>& cameFrom, Node current, TArray<FNavSegment>& directions) const;
    FString getProceedDirection(double angle) const;
    FString getTurnDirection(double angle) const;
    double getActualAngle(const Node& current, const Node& from) const;
};

NavigatorImpl::NavigatorImpl()
{
}

NavigatorImpl::~NavigatorImpl()
{
}

bool NavigatorImpl::loadMapData(FString mapFile)
{
    if (!map.load(mapFile))
        return false;
    smap.init(map);
    amap.init(map);
    return true;
}

NavResult NavigatorImpl::navigate(FString start, FString end, TArray<FNavSegment>& directions) const
{
    // check if start location is valid
    Node source;
    if (!amap.getGeoCoord(start, source.coord))
        return NAV_BAD_SOURCE;

    // check if end location is valid
    Node goal;
    if (!amap.getGeoCoord(end, goal.coord))
        return NAV_BAD_DESTINATION;

    // get start's street segment
    source.seg = smap.getSegments(source.coord)[0];

    // get end's street segment
    goal.seg = smap.getSegments(goal.coord)[0];

    // f score = g score + heuristic score
    // start's fscore is heuristic score
    source.fScore = distanceEarthMiles(source.coord, goal.coord);

    // g score is cost to get to that node
    UTbTNavigationSystem<Node, double> gScore;
    gScore.associate(source, 0);

    priority_queue<Node, vector<Node>, greater<Node>> open;
    vector<Node> openV; // to traverse through open
    open.push(source);
    openV.push_back(source);

    vector<Node> closed;
    UTbTNavigationSystem<Node, Node> cameFrom; // to reconstruct path

    while (!open.empty())
    {
        Node current = open.top();

        // reach destination
        if (current.coord == goal.coord)
        {
            reconstructPath(cameFrom, current, directions);
            return NAV_SUCCESS;
        }

        // move Node from open to closed
        open.pop();
        vector<Node>::iterator it = openV.begin();
        while (it != openV.end())
        //for (auto& it : openV)
        {
            if (it->coord == current.coord)
                openV.erase(it);
            else
                it++;
        }
        closed.push_back(current);

        // get potential neighbors (start coordinate, end coordinate and coordinates of attractions)
        TArray<FStreetSegment> segments = smap.getSegments(current.coord);
        TArray<Node> neighbors;
        for (auto s : segments)
        {
            Node n;
            n.coord = s.segment.start;
            n.seg = s;
            neighbors.Push(n);
            n.coord = s.segment.end;
            n.seg = s;
            neighbors.Push(n);
            for (auto i : s.attractions)
            {
                n.coord = i.geocoordinates;
                n.seg = s;
                neighbors.Push(n);
            }

            // add potential neighbors to open if not already closed
            for (auto c : neighbors)
            {
                // check if has already been visited
                if (find(closed.begin(), closed.end(), c) != closed.end())
                    continue;

                double tentativeG = *(gScore.find(current)) + distanceEarthMiles(current.coord, c.coord); // g score = g score of current node + length between current node and neighbor
                c.fScore = tentativeG + distanceEarthMiles(c.coord, goal.coord);
                if (find(openV.begin(), openV.end(), c) == openV.end())
                {
                    open.push(c);
                    openV.push_back(c);
                }

                // check if better path
                else if (tentativeG >= *(gScore.find(*find(openV.begin(), openV.end(), c))))
                    continue;
                gScore.associate(c, tentativeG);
                cameFrom.associate(c, current);
            }
        }
    }

    return NAV_NO_ROUTE;
}

bool NavigatorImpl::Node::operator>(const NavigatorImpl::Node& other) const
{
    return fScore > other.fScore;
}

bool NavigatorImpl::Node::operator<=(const NavigatorImpl::Node& other) const
{
    return fScore <= other.fScore;
}

bool NavigatorImpl::Node::operator==(const NavigatorImpl::Node& other) const
{
    return coord == other.coord;
}

void NavigatorImpl::reconstructPath(UTbTNavigationSystem<Node, Node>& cameFrom, Node current, TArray<FNavSegment>& directions) const
{
    std::vector<FNavSegment> vec;
    TArray<FNavSegment> arr;
    arr.Append(directions);

    vec.resize(arr.Num());

    for (int i = 0; i < arr.Num(); i++) {
        vec[i] = arr[i];
    }

    Node from;
    Node after; // for turn nav segments
    while (cameFrom.find(current) != nullptr)
    {
        from = *cameFrom.find(current);
        FGeoSegment gs = FGeoSegment(from.coord, current.coord);
        if (from.seg.streetName == current.seg.streetName) // if nodes are on same street, then proceed
        {
            FNavSegment n = FNavSegment(getProceedDirection(angleOfLine(gs)), current.seg.streetName, distanceEarthMiles(from.coord, current.coord), FGeoSegment(from.coord, current.coord));
            vec.insert(vec.begin(), n);
        }
        else // if nodes are on different streets, then there is a turn nav segment
        {
            double angle = angleOfLine(gs);

            // add proceed nav segment leading up to turn
            FNavSegment p = FNavSegment(getProceedDirection(angle), current.seg.streetName, distanceEarthMiles(from.coord, current.coord), FGeoSegment(from.coord, current.coord));
            vec.insert(vec.begin(), p);
            Node beforeFrom = *cameFrom.find(from);
            FGeoSegment before = FGeoSegment(beforeFrom.coord, from.coord);

            // find the angle between the two streets
            double fromAngle = angleOfLine(before);
            angle -= fromAngle;
            if (angle < 0)
                angle += 360;

            // add turn segment
            p = FNavSegment(getTurnDirection(angle), current.seg.streetName);
            vec.insert(vec.begin(), p);
        }
        after = current;
        current = from;
    }
}

FString NavigatorImpl::getProceedDirection(double angle) const
{
    if (angle >= 0 && angle <= 22.5)
        return "east";
    else if (angle > 22.5 && angle <= 67.5)
        return "northeast";
    else if (angle > 67.5 && angle <= 112.5)
        return "north";
    else if (angle > 112.5 && angle <= 157.5)
        return "northwest";
    else if (angle > 157.5 && angle <= 202.5)
        return "west";
    else if (angle > 202.5 && angle <= 247.5)
        return "southwest";
    else if (angle > 247.5 && angle <= 292.5)
        return "south";
    else if (angle > 292.5 && angle <= 337.5)
        return "southeast";
    else
        return "east";
}

FString NavigatorImpl::getTurnDirection(double angle) const
{
    if (angle < 180)
        return "left";
    return "right";
}
//******************** Navigator functions ************************************

// These functions simply delegate to NavigatorImpl's functions.
// You probably don't want to change any of this code.

UNavigator::UNavigator()
{
    m_impl = new NavigatorImpl;
}

UNavigator::~UNavigator()
{
    delete m_impl;
}

bool UNavigator::loadMapData(FString mapFile)
{
    return m_impl->loadMapData(mapFile);
}

NavResult UNavigator::navigate(FString start, FString end, TArray<FNavSegment>& directions) const
{
    return m_impl->navigate(start, end, directions);
}


//***********************************************************************************************************************************************************************************************************************//
// Segment Mapper
//***********************************************************************************************************************************************************************************************************************//



class SegmentMapperImpl
{
public:
    SegmentMapperImpl();
    ~SegmentMapperImpl();
    void init(const UMapLoader& ml);
    TArray<FStreetSegment> getSegments(const FGeoCoord& gc) const;
private:
    UTbTNavigationSystem<FGeoCoord, TArray<FStreetSegment>> segmentMap;
};

SegmentMapperImpl::SegmentMapperImpl()
{
}

SegmentMapperImpl::~SegmentMapperImpl()
{
}

void SegmentMapperImpl::init(const UMapLoader& ml)
{
    FStreetSegment seg;
    for (int32 i = 0; i < ml.getNumSegments(); i++)
    {
        ml.getSegment(i, seg);

        FGeoCoord coord = seg.segment.start;

        // find all segments related to start of current segment
        TArray<FStreetSegment>* v = segmentMap.find(coord);
        if (v == nullptr)
        {
            TArray<FStreetSegment> sn;
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
            TArray<FStreetSegment> sn;
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
                TArray<FStreetSegment> sn;
                sn.Push(seg);
                segmentMap.associate(coord, sn);
            }
            else
                v->Push(seg);
        }
    }
}

TArray<FStreetSegment> SegmentMapperImpl::getSegments(const FGeoCoord& gc) const
{
    const TArray<FStreetSegment>* v;
    v = segmentMap.find(gc);
    if (v != nullptr)
        return *v;
    else // no segments found
    {
        TArray<FStreetSegment> segments;
        return segments;
    }
}

//******************** SegmentMapper functions ********************************

// These functions simply delegate to SegmentMapperImpl's functions.
// You probably don't want to change any of this code.

USegmentMapper::USegmentMapper()
{
    m_impl = new SegmentMapperImpl;
}

USegmentMapper::~USegmentMapper()
{
    delete m_impl;
}

void USegmentMapper::init(const UMapLoader& ml)
{
    m_impl->init(ml);
}

TArray<FStreetSegment> USegmentMapper::getSegments(const FGeoCoord& gc) const
{
    return m_impl->getSegments(gc);
}
