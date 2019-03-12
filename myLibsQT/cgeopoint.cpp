#include "cgeopoint.h"
#include <cmath>

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CGeoPoint::CGeoPoint()
{
    invalidate();
}

CGeoPoint::CGeoPoint(double lat, double lon, double maxLat)
{
    init(lat, lon, maxLat);
}

void CGeoPoint::init(double lat, double lon, double maxLat)
{
    if(lat>maxLat)
        m_lat=maxLat;
    else if(lat<(-maxLat))
        m_lat=-maxLat;
    else
        m_lat=lat;
    m_lon=fmod(lon,360);
    if(m_lon>180)
        m_lon-=360.;
    if(m_lon<=-180)
        m_lon+=360.;

}

void CGeoPoint::invalidate()
{
    m_lat=100;	//ungueltig

}
