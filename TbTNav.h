#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Math/UnrealMathUtility.h"
#include "TbTNav.generated.h"

UCLASS()
class MYPROJECT3_API UTbTNav : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Turn By Turn Navigation")
		static TArray<FVector> Navigate(FString mapFile, FString origin, FString destination);

};


//***********************************************************************************************************************************************************************************************************************//
// UTbTNavigationSystem
//***********************************************************************************************************************************************************************************************************************//


template<typename KeyType, typename ValueType>

UCLASS()
class MYPROJECT3_API UTbTNavigationSystem : public UObject {

    GENERATED_BODY()
public:
    UTbTNavigationSystem() {
        root = nullptr;
        numAss = 0;
    }

    ~UTbTNavigationSystem() {
        clear();
    }


    void clear()
    {
        remove(root);
        root = nullptr;
        numAss = 0;
        return;
    }

    int32 size() const
    {
        return numAss;
    }

    void associate(const KeyType& key, const ValueType& value)
    {
        if (insert(key, value))
            numAss++;
    }

    // for a map that can't be modified, return a pointer to const ValueType
    const ValueType* find(const KeyType& key) const
    {
        if (root == nullptr)
            return nullptr;
        Node* cur = root;
        while (cur != nullptr)
        {
            if (key == cur->m_key)
                return &(cur->m_value);
            if (key <= cur->m_key)
                cur = cur->left;
            else if (key > cur->m_key)
                cur = cur->right;
        }
        return nullptr;
    }

    // for a modifiable map, return a pointer to modifiable ValueType
    ValueType* find(const KeyType& key)
    {
        return const_cast<ValueType*>(const_cast<const UTbTNavigationSystem*>(this)->find(key));
    }

    // C++11 syntax for preventing copying and assignment
    UTbTNavigationSystem(const UTbTNavigationSystem&) = delete;
    UTbTNavigationSystem& operator=(const UTbTNavigationSystem&) = delete;


private:
    USTRUCT()
    struct Node
    {
        GENERATED_BODY()
        Node(const KeyType& key, const ValueType& value)
        {
            m_key = key;
            m_value = value;
            left = right = nullptr;
        }
        KeyType m_key;
        ValueType m_value;
        Node* left;
        Node* right;
    };

    Node* root;
    int32 numAss;

    bool insert(const KeyType& key, const ValueType& value)
    {
        if (root == nullptr) // empty BST
        {
            root = new Node(key, value);
            return true;
        }
        if (find(key) != nullptr) // already exists in BST
        {
            ValueType* f = find(key);
            *f = value;
            return false;
        }
        Node* cur = root;
        for (;;)
        {
            if (key <= cur->m_key) // move/add left if key is less than current key
            {
                if (cur->left != nullptr)
                    cur = cur->left;
                else
                {
                    cur->left = new Node(key, value);
                    return true;
                }
            }
            else if (key > cur->m_key) // move/add right if key greater than current key
            {
                if (cur->right != nullptr)
                    cur = cur->right;
                else
                {
                    cur->right = new Node(key, value);
                    return true;
                }
            }
        }
    }

    void remove(Node* cur)
    {
        // move all the way down the tree and delete leaf nodes
        if (cur == nullptr)
            return;
        else
        {
            if (cur->left != nullptr)
                remove(cur->left);
            if (cur->right != nullptr)
                remove(cur->right);
            delete cur;
        }
    }
};


//***********************************************************************************************************************************************************************************************************************//
// Provided
//***********************************************************************************************************************************************************************************************************************//

USTRUCT()
struct FGeoCoord
{
    GENERATED_BODY()
    FGeoCoord(FString lat, FString lon)
        : latitudeText(lat), longitudeText(lon), latitude(FCString::Atod(*lat)), longitude(FCString::Atod(*lon))
    {}

    FGeoCoord()
        : latitudeText("0"), longitudeText("0"), latitude(0), longitude(0)
    {}

    FString latitudeText;
    FString longitudeText;
    double      latitude;
    double      longitude;
};

USTRUCT()
struct FGeoSegment
{
    GENERATED_BODY()
    FGeoSegment(const FGeoCoord& s, const FGeoCoord& e)
        : start(s), end(e)
    {}

    FGeoSegment()
    {}

    FGeoCoord start;
    FGeoCoord end;
};

USTRUCT()
struct FAttraction
{
    GENERATED_BODY()
    FString name;
    FGeoCoord	geocoordinates;
};

USTRUCT()
struct FStreetSegment
{
    GENERATED_BODY()
    FString			streetName;
    FGeoSegment				segment;
    TArray<FAttraction>	attractions;
};

class MapLoaderImpl;

UCLASS()
class MYPROJECT3_API UMapLoader : public UObject
{
    GENERATED_BODY()
public:
    UMapLoader();
    ~UMapLoader();
    bool load(FString mapFile);
    size_t getNumSegments() const;
    bool getSegment(size_t segNum, FStreetSegment& seg) const;
    // We prevent a MapLoader object from being copied or assigned.
  //UMapLoader(const UMapLoader&) = delete;
  //UMapLoader& operator=(const UMapLoader&) = delete;
private:
    MapLoaderImpl* m_impl;
};

class AttractionMapperImpl;

UCLASS()
class MYPROJECT3_API UAttractionMapper : public UObject
{
    GENERATED_BODY()
public:
    UAttractionMapper();
    ~UAttractionMapper();
    void init(const UMapLoader& ml);
    bool getGeoCoord(FString attraction, FGeoCoord& gc) const;
    // We prevent an AttractionMapper object from being copied or assigned.
  //UAttractionMapper(const UAttractionMapper&) = delete;
  //UAttractionMapper& operator=(const UAttractionMapper&) = delete;
private:
    AttractionMapperImpl* m_impl;
};

class SegmentMapperImpl;

UCLASS()
class MYPROJECT3_API USegmentMapper : public UObject
{
    GENERATED_BODY()
public:
    USegmentMapper();
    ~USegmentMapper();
    void init(const UMapLoader& ml);
    TArray<FStreetSegment> getSegments(const FGeoCoord& gc) const;
    // We prevent a SegmentMapper object from being copied or assigned.
  //USegmentMapper(const USegmentMapper&) = delete;
  //USegmentMapper& operator=(const USegmentMapper&) = delete;
private:
    SegmentMapperImpl* m_impl;
};

USTRUCT()
struct FNavSegment
{
    GENERATED_BODY()
public:
    UENUM()
    enum NavCommand { PROCEED, TURN };

    FNavSegment()
        : m_command(PROCEED), m_distance(0)
    {}

    // constructor for a Proceed NavSegment
    FNavSegment(FString direction, FString streetName, double distance, const FGeoSegment& gs)
        : m_command(PROCEED), m_direction(direction), m_streetName(streetName), m_distance(distance), m_geoSegment(gs)
    {}

    // constructor for a Turn NavSegment
    FNavSegment(FString direction, FString streetName)
        : m_command(TURN), m_direction(direction), m_streetName(streetName)
    {}

    NavCommand	m_command;	    // PROCEED or TURN
    FString	m_direction;	// e.g., "north" for proceed or "left" for turn
    FString	m_streetName;	// e.g., Westwood Blvd
    double		m_distance;		// for proceed, distance in kilometers
    FGeoSegment	m_geoSegment;
};

UENUM()
enum NavResult {
    NAV_SUCCESS, NAV_BAD_SOURCE, NAV_BAD_DESTINATION, NAV_NO_ROUTE
};

class NavigatorImpl;

UCLASS()
class MYPROJECT3_API UNavigator : public UObject
{
    GENERATED_BODY()
public:
    UNavigator();
    ~UNavigator();
    bool loadMapData(FString mapFile);
    NavResult navigate(FString start, FString end, TArray<FNavSegment>& directions) const;
    // We prevent a Navigator object from being copied or assigned.
//    UNavigator(const UNavigator&) = delete;
//    UNavigator& operator=(const UNavigator&) = delete;
private:
    NavigatorImpl* m_impl;
};


// Tools for computing distance between GeoCoords, angle of a GeoSegment,
// and angle between two GeoSegments 

//#include <cmath> 

  // decimal degrees to radians
inline double deg2rad(double deg) {
    return deg * PI / 180;
}


// radians to decimal degrees
inline double rad2deg(double rad) {
    return rad * 180 / PI;
}

/**
* Returns the distance between two points on the Earth.
* Direct translation from http://en.wikipedia.org/wiki/Haversine_formula
* @param lat1d Latitude of the first point in degrees
* @param lon1d Longitude of the first point in degrees
* @param lat2d Latitude of the second point in degrees
* @param lon2d Longitude of the second point in degrees
* @return The distance between the two points in kilometers
*/
inline double distanceEarthKM(const FGeoCoord& g1, const FGeoCoord& g2) {
    static const double earthRadiusKm = 6371.0;
    double lat1r = deg2rad(g1.latitude);
    double lon1r = deg2rad(g1.longitude);
    double lat2r = deg2rad(g2.latitude);
    double lon2r = deg2rad(g2.longitude);
    double u = FMath::Sin((lat2r - lat1r) / 2);
    double v = FMath::Sin((lon2r - lon1r) / 2);
    return 2.0 * earthRadiusKm * FMath::Asin(FMath::Sqrt(u * u + FMath::Sin(lat1r) * FMath::Sin(lat2r) * v * v));
}

inline double distanceEarthMiles(const FGeoCoord& g1, const FGeoCoord& g2) {
    const double milesPerKm = 0.621371;
    return distanceEarthKM(g1, g2) * milesPerKm;
}

inline double angleBetween2Lines(const FGeoSegment& line1, const FGeoSegment& line2)
{
    double angle1 = atan2(line1.end.latitude - line1.start.latitude, line1.end.longitude - line1.start.longitude);
    double angle2 = atan2(line2.end.latitude - line2.start.latitude, line2.end.longitude - line2.start.longitude);

    double result = rad2deg(angle2 - angle1);
    if (result < 0)
        result += 360;

    return result;
}

inline double angleOfLine(const FGeoSegment& line1)
{
    double angle = atan2(line1.end.latitude - line1.start.latitude, line1.end.longitude - line1.start.longitude);
    double result = rad2deg(angle);
    if (result < 0)
        result += 360;

    return result;
}


//***********************************************************************************************************************************************************************************************************************//
// Support
//***********************************************************************************************************************************************************************************************************************//


bool operator<=(const FGeoCoord& g1, const FGeoCoord& g2);

bool operator>(const FGeoCoord& g1, const FGeoCoord& g2);

bool operator==(const FGeoCoord& g1, const FGeoCoord& g2);

bool operator==(const FStreetSegment& seg1, const FStreetSegment& seg2);

bool operator==(const FGeoSegment& seg1, const FGeoSegment& seg2);


//***********************************************************************************************************************************************************************************************************************//
// Priority Queue
//***********************************************************************************************************************************************************************************************************************//

