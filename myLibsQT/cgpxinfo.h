#ifndef CGPXINFO_H
#define CGPXINFO_H
#include "cgeopoint.h"
#include "cgeorect.h"
#include <qstring.h>
#include <qdatetime.h>
#include <qcolor.h>
#include <trackcol.h>

#define COLDEF QColor(0,0,0,1)


class CGPXInfo
{
public:
    CGPXInfo();
    void writeGPX(QString pfad);
    QString m_route;
    QList<CGeoPoint> m_trkPts;
    CGeoRect m_area;
    TrackCol m_col;
    QDateTime m_dateTime;
    bool m_rev;
    QString m_pfad;
    QString m_name;
    virtual ~CGPXInfo();
};

#endif // CGPXINFO_H
