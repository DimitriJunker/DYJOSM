#include "trackcol.h"
QColor TrackCol::m_normal;
QColor TrackCol::m_flug;

TrackCol::TrackCol()
{
    /*DYJ DYJTrack 2.06c Start*/
    m_useTrColSp=TC_AUTO;
    /*DYJ  Ende; alt:
    m_useDef=true;
    */
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
    /*DYJ DYJTrack 2.06c Start*/
    m_useTrColSp=TC_RGB;
    /*DYJ  Ende; alt:
    m_useDef=false;
    */
    m_col=col;
}

void TrackCol::setCol(int col)
{
    /*DYJ DYJTrack 2.06c Start*/
    m_useTrColSp=TC_RGB;
    /*DYJ  Ende; alt:
    m_useDef=false;
    */
    m_col.setRgb(col);
}

/*DYJ DYJTrack 2.06c Start*/
void TrackCol::setCol(bool useStd,bool useFlug,QColor col)
{
    if(useStd)
        m_useTrColSp=TC_STD;
    else if(useFlug)
        m_useTrColSp=TC_FLUG;
    else
        m_useTrColSp=TC_RGB;
/*DYJ  Ende; alt:
void TrackCol::setCol(bool useDef,QColor col)
{
    m_useDef=useDef;
*/
m_col=col;
}
/*DYJ DYJTrack 2.06c Start*/
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

}/*DYJ  Ende; alt:
QColor TrackCol::get_col(bool isFlug)
{
    if(!m_useDef)
        return m_col;
    else if(isFlug)
        return m_flug;
    else
        return m_normal;
}*/




