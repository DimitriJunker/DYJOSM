#include "trackcol.h"
QColor TrackCol::m_normal;
QColor TrackCol::m_flug;

TrackCol::TrackCol()
{
    m_useTrColSp=TC_AUTO;
    m_normal=QColor(255,0,0);
    m_flug=QColor(0,255,0);
}
void TrackCol::setDef(QColor norm, QColor flug)
{
    m_normal=norm;
    m_flug=flug;
}

void TrackCol::setCol(QColor col)
{
    m_useTrColSp=TC_RGB;
    m_col=col;
}

void TrackCol::setCol(int col)
{
    m_useTrColSp=TC_RGB;
    m_col.setRgb(col);
}

void TrackCol::setCol(bool useStd,bool useFlug,QColor col)
{
    if(useStd)
        m_useTrColSp=TC_STD;
    else if(useFlug)
        m_useTrColSp=TC_FLUG;
    else
        m_useTrColSp=TC_RGB;
m_col=col;
}
QColor TrackCol::get_col(bool isFlug,TrackColSp colType)
{
    QColor ret;
    switch(colType)
    {
    case TC_AUTO:
        if(isFlug)
            ret=m_flug;
        else
            ret=m_normal;
        break;
    case TC_STD:
        ret=m_normal;
        break;
    case TC_FLUG:
        ret=m_flug;
        break;
    default:
        ret=m_col;
    }
    return ret;

}




