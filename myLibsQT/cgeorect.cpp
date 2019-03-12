#include "cgeorect.h"
#include <cmath>
 #include <QtGlobal>

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CGeoRect::CGeoRect()
{
    invalidate();
}

CGeoRect::~CGeoRect()
{

}

CGeoRect::CGeoRect(double north, double south, double west, double east,double maxLat/*=90*/)
{
    init(north,south,west,east,maxLat);
}
CGeoRect::CGeoRect(const char * north, const char * south, const char * west, const char * east,double maxLat/*=90*/)
{
    init(atof(north),atof(south),atof(west),atof(east),maxLat);
}

bool CGeoRect::isValid()
{
    if(m_w<-180 ||m_w>180 || m_n<-90 ||m_n>90||(m_e-m_w)<0 ||(m_e-m_w)>360 || m_s<-90 ||m_s>90)	//CHG: DYJTracker 1.01a DYJ
        return false;
    else
        return true;
}

void CGeoRect::init(double north, double south, double west, double east, double maxLat)
{
    if(north<south)
    {
        double tmp=north;
        north=south;
        south=tmp;
    }
    if(north>maxLat)
        m_n=maxLat;
    else
        m_n=north;
    if(south<(-maxLat))
        m_s=-maxLat;
    else
        m_s=south;
    m_w=fmod(west,360);
    if(m_w>180)
        m_w-=360.;
    if(m_w<=-180)
        m_w+=360.;
    double width=east-west;	//CHG: DYJTracker 1.01a DYJ
    if(width>360)
        width=360;
    m_e=m_w+width;
/*	m_e=fmod(east,360);
    if(m_e>180)
        m_e-=360.;
    if(m_e<=-180||m_e<=m_w)
        m_e+=360.;
*/
}

void CGeoRect::add(double lat, double lon)
{
    if(lon>=-180 && lon<=180 && lat>=-90 && lat<=90)
    {
        if(isValid())
        {
            double w1=qMin(m_w,lon);
            double e1=qMax(m_e,lon);
            double w2=qMin(m_w,lon+360);
            double e2=qMax(m_e,lon+360);
            double w3=qMin(m_w+360,lon);
            double e3=qMax(m_e+360,lon);
            if((e1-w1)<(e2-w2) && (e1-w1)<(e3-w3))
            {
                m_e=e1;
                m_w=w1;
            }
            else if((e2-w2)<(e3-w3))
            {
                m_e=e2;
                m_w=w2;
            }
            else
            {
                m_e=e3;
                m_w=w3;
            }
            m_n=qMax(m_n,lat);
            m_s=qMin(m_s,lat);
        }
        else
        {
            m_n=m_s=lat;
            m_e=m_w=lon;
        }
    }

}

void CGeoRect::add(CGeoRect gr)
{
    if(isValid())
    {
        m_n=qMax(m_n,gr.m_n);
        m_s=qMin(m_s,gr.m_s);
        m_w=qMin(m_w,gr.m_w);
        m_e=qMax(m_e,gr.m_e);
    }
    else
        *this=gr;
}

void CGeoRect::add(CGeoPoint pt)
{
    add(pt.m_lat,pt.m_lon);
}

void CGeoRect::invalidate()
{
    m_n=100;
}
