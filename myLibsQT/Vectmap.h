// Vectmap.h: Schnittstelle für die Klasse CVectmap.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VECTMAP_H__330635E0_C573_45B2_9AB8_A4E3294910BD__INCLUDED_)
#define AFX_VECTMAP_H__330635E0_C573_45B2_9AB8_A4E3294910BD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <cgeorect.h>
#include <cpixmap.h>
#include <cmapsrc.h>

class CVectmap  
{
public:
	CGeoRect m_rect;
	CMapSrc* m_map;
    void LoadTile(SDLM_DATA *data,QString fName);
    unsigned int m_maxCacheDays;
	int m_tileNr;
    QString m_vectExt;
    QString m_filename;
    QString m_pfadG;
    QString m_pfadI;
	CVectmap(CGeoRect &rect, SDLM_DATA *data, CMapSrc *map);
	CVectmap();
	virtual ~CVectmap();

};

#endif // !defined(AFX_VECTMAP_H__330635E0_C573_45B2_9AB8_A4E3294910BD__INCLUDED_)
