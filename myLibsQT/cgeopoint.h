#ifndef CGEOPOINT_H
#define CGEOPOINT_H

class CGeoPoint
{
public:
    CGeoPoint();
    CGeoPoint(double lat, double lon, double maxLat=90.0);
    void init(double lat, double lon, double maxLat=90.0);
    void invalidate();
    double m_lon;
    double m_lat;
};

#endif // CGEOPOINT_H
