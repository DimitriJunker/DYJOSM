#ifndef CGEORECT_H
#define CGEORECT_H
#include "cgeopoint.h"

class CGeoRect
{
public:
    void invalidate();
    void add(CGeoPoint pt);
    void add(CGeoRect gr);
    void add(double lat,double lon);
    void init(double north, double south, double west, double east,double maxLat=90);
    bool isValid();
    CGeoRect(double nord, double south, double west, double east,double maxLat=90);
    CGeoRect(const char * nord, const char * south, const char * west, const char * east,double maxLat=90);
    double m_w;
    double m_e;
    double m_s;
    double m_n;
    CGeoRect();
    virtual ~CGeoRect();
};

#endif // CGEORECT_H
