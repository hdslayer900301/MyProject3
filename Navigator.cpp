#include "Provided.h"
#include "Map.h"
#include "Support.h"
#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "GenericPlatform/GenericPlatform.h"
#include <queue>
#include <vector>
using namespace std;

//***********************************************************************************************************************************************************************************************************************//
// Navigator
//***********************************************************************************************************************************************************************************************************************//


class NavigatorImpl
{
public:
    NavigatorImpl();
    ~NavigatorImpl();
    bool loadMapData(FString mapFile);
    NavResult navigate(FString start, FString end, TArray<NavSegment>& directions) const;
private:
    MapLoader map;
    SegmentMapper smap;
    AttractionMapper amap;
    struct Node {
        GeoCoord coord;
        StreetSegment seg;
        bool operator>(const Node& other) const;
        bool operator<=(const Node& other) const;
        bool operator==(const Node& other) const;
        double fScore;
    };
    void reconstructPath(Map<Node, Node>& cameFrom, Node current, TArray<NavSegment>& directions) const;
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

NavResult NavigatorImpl::navigate(FString start, FString end, TArray<NavSegment>& directions) const
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
    Map<Node, double> gScore;
    gScore.associate(source, 0);

    priority_queue<Node, vector<Node>, greater<Node>> open;
    vector<Node> openV; // to traverse through open
    open.push(source);
    openV.push_back(source);

    vector<Node> closed;
    Map<Node, Node> cameFrom; // to reconstruct path

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
        TArray<StreetSegment> segments = smap.getSegments(current.coord);
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

void NavigatorImpl::reconstructPath(Map<Node, Node>& cameFrom, Node current, TArray<NavSegment>& directions) const
{
    std::vector<NavSegment> vec;
    TArray<NavSegment> arr;
    arr.Append(directions);

    vec.resize(arr.Num());

    for (int32 i = 0; i < arr.Num(); i++) {
        vec[i] = arr[i];
    }

    Node from;
    Node after; // for turn nav segments
    while (cameFrom.find(current) != nullptr)
    {
        from = *cameFrom.find(current);
        GeoSegment gs = GeoSegment(from.coord, current.coord);
        if (from.seg.streetName == current.seg.streetName) // if nodes are on same street, then proceed
        {
            NavSegment n = NavSegment(getProceedDirection(angleOfLine(gs)), current.seg.streetName, distanceEarthMiles(from.coord, current.coord), GeoSegment(from.coord, current.coord));
            vec.insert(vec.begin(), n);
        }
        else // if nodes are on different streets, then there is a turn nav segment
        {
            double angle = angleOfLine(gs);

            // add proceed nav segment leading up to turn
            NavSegment p = NavSegment(getProceedDirection(angle), current.seg.streetName, distanceEarthMiles(from.coord, current.coord), GeoSegment(from.coord, current.coord));
            vec.insert(vec.begin(), p);
            Node beforeFrom = *cameFrom.find(from);
            GeoSegment before = GeoSegment(beforeFrom.coord, from.coord);

            // find the angle between the two streets
            double fromAngle = angleOfLine(before);
            angle -= fromAngle;
            if (angle < 0)
                angle += 360;

            // add turn segment
            p = NavSegment(getTurnDirection(angle), current.seg.streetName);
            vec.insert(vec.begin(), p);
        }
        after = current;
        current = from;
    }
    arr.SetNumUninitialized(vec.size());
    for (int i = 0; i < vec.size(); i++) {
        arr[i] = vec[i];
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

Navigator::Navigator()
{
    m_impl = new NavigatorImpl;
}

Navigator::~Navigator()
{
    delete m_impl;
}

bool Navigator::loadMapData(FString mapFile)
{
    return m_impl->loadMapData(mapFile);
}

NavResult Navigator::navigate(FString start, FString end, TArray<NavSegment>& directions) const
{
    return m_impl->navigate(start, end, directions);
}

