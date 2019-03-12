#include "cgpxinfo.h"
//#include "GeoRect.h"
//#include <geopoint.h>
//#include <afxtempl.h>

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CGPXInfo::CGPXInfo()
{
    m_route=m_pfad="";
}

CGPXInfo::~CGPXInfo()
{
    while(!m_trkPts.isEmpty())
        m_trkPts.removeFirst();
}

void CGPXInfo::writeGPX(QString pfad)
{
    FILE *fOut;
    fopen_s(&fOut,pfad.toLatin1(),"w+");
    if(fOut)
    {
        fprintf(fOut,"<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
        fprintf(fOut,"<gpx version=\"1.0\" creator=\"dyjtrack.exe - http://www.dimitri-junker.de/html/openstreetmap.html\"\n");
        fprintf(fOut,"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n");
        fprintf(fOut,"xmlns=\"http://www.topografix.com/GPX/1/0\"\n");
        fprintf(fOut,"xsi:schemaLocation=\"http://www.topografix.com/GPX/1/0 http://www.topografix.com/GPX/1/0/gpx.xsd\">\n");
        fprintf(fOut,"<bounds minlat=\"%f\" minlon=\"%f\" maxlat=\"%f\" maxlon=\"%f\"/>\n",m_area.m_s,m_area.m_w,m_area.m_n,m_area.m_e);
        fprintf(fOut,"<trk>\n\t<trkseg>\n");

        QList<CGeoPoint>::Iterator aktPos;
        for(aktPos=m_trkPts.begin();aktPos!=m_trkPts.end();++aktPos)
            fprintf(fOut,"\t\t<trkpt lat=\"%f\" lon=\"%f\" />\n",aktPos->m_lat,aktPos->m_lon);

        fprintf(fOut,"\t</trkseg>\n</trk>\n</gpx>\n");
        fclose(fOut);

    }
}

