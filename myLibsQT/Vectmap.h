#if !defined(AFX_VECTMAP_H__330635E0_C573_45B2_9AB8_A4E3294910BD__INCLUDED_)
#define AFX_VECTMAP_H__330635E0_C573_45B2_9AB8_A4E3294910BD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <cgeorect.h>
#include <QString>

class CMapSrc;
struct SDLM_DATA;

class CVectmap  
{
public:
    CVectmap() = default;
    CVectmap(const CGeoRect &rect, const SDLM_DATA & data, CMapSrc *map);

    CGeoRect m_rect;
    CMapSrc* m_map = nullptr;
    unsigned int m_maxCacheDays = 0;
    int m_tileNr = -1;
    QString m_vectExt;
    QString m_filename;
    QString m_pfadG;
    QString m_pfadI;

    void LoadTile(SDLM_DATA &data, QString fName);

private:
    void noOffsetUrl(QString &url) const;
    void unpack(const SDLM_DATA & data, const QString &tile1Path) const;
    void renameFile(const SDLM_DATA & data, const QString & tile1Path, const QString & zfName) const;
};

#endif // !defined(AFX_VECTMAP_H__330635E0_C573_45B2_9AB8_A4E3294910BD__INCLUDED_)
