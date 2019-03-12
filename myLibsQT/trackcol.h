#ifndef TRACKCOL_H
#define TRACKCOL_H
#include <QColor>

enum TrackColSp {TC_AUTO,TC_STD,TC_FLUG,TC_RGB};
class TrackCol
{
public:
    static QColor m_normal;
    static QColor m_flug;
    QColor m_col;
    /*DYJ DYJTrack 2.06c Start*/
    TrackColSp m_useTrColSp;
    /*DYJ  Ende; alt:
    bool m_useDef;
    */
    void setDef(QColor norm, QColor flug);
    void setCol(QColor col);
    void setCol(int col);   //SetRgb
    void setCol(bool useStd, bool useFlug, QColor col);
    QColor get_col(bool isFlug, TrackColSp colType);
    TrackCol();
};

#endif // TRACKCOL_H
