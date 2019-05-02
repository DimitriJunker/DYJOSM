#include "Vectmap.h"

#include <cosm.h>
#include <cmapsrc.h>
#include <urldownload.h>
#include <cpixmap.h>
#include <cmapsrc.h>

//#include <QProcess>
//#include <QMessageBox>

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CVectmap::CVectmap(const CGeoRect &rect, const SDLM_DATA & data, CMapSrc *map) :
    m_rect(rect), m_map(map)
{
	m_map=map;
	m_rect=rect;
    m_pfadI=QString("%1map/").arg(data.m_outBas);
    if (map->m_offset == -1)	//*.osm
	{
		m_pfadG="";
        m_filename=QString("%1_N%2_S%3_W%4_E%5").arg(map->m_pref).arg(rect.m_n).arg(rect.m_s).arg(rect.m_w).arg(rect.m_e);
        m_filename.replace('.','_');
		m_tileNr=-1;
		m_vectExt=".osm";
	}
	else
	{
        m_pfadG=QString("%1%2/").arg(data.m_outBas, map->m_name);
        const auto xloop=rect.m_w+180;
        const auto yloop=rect.m_s+90;
	//Number = ((int)((lat + 90) / ksize) + (int)((lon + 180) / ksize) * (int)(180 / ksize)) + 63240001
		m_tileNr=int(yloop/map->m_kSize)+int(xloop/map->m_kSize)*int(180/map->m_kSize)+map->m_offset;

        if(data.m_nameBy==NB_COORD)
		{
            m_filename=QString("%1_lat%2_lon%3").arg(map->m_pref).arg(rect.m_s).arg(rect.m_w);
            m_filename.replace('.','_');
		}
		else
            m_filename=QString("%1_%2").arg(map->m_pref).arg(m_tileNr);
		
        m_vectExt=data.m_pictType;
	}
    m_maxCacheDays=data.m_maxCacheDays;
}

void CVectmap::LoadTile(SDLM_DATA &data, QString zfName)
{
    auto url = m_map->m_url;
	if(m_map->m_offset==-1)
	{
        noOffsetUrl(url);
        const auto file=QString("%1%2.osm").arg(m_pfadI, zfName);
        if(urlDownload::downloadFile(url, file)!=0)
		{
            data.m_errTxts.append(zfName+="<br>\r\n");
            data.errs|= ERR_PIXM_TILE;
		}
        return;
	}

    if (m_map->m_mapsPF)
        url=QString("%1/%2").arg(m_map->m_url).arg(m_tileNr/m_map->m_mapsPF);

    COsm osm(0,url,m_map->m_ext,m_pfadG,m_maxCacheDays);
    QString tile1Path,errTxt;
    switch(osm.getsaveVTileWithCache(m_tileNr,tile1Path,&errTxt))
    {
    case 1:	//Laden erfolglos
        data.m_errTxts.append(tile1Path.mid(1)+="<br>\r\n");
        data.errs|= ERR_PIXM_TILE;
        break;
    case 2: //Laden erfolglos aber altes File vorhanden
        data.m_errTxts.append(tile1Path.mid(1)+"<br>\r\n");
        data.errs|= ERR_PIXM_TILE_OLD;
    [[clang::fallthrough]]; case 0:	//OK
        unpack(data, tile1Path);
        renameFile(data, tile1Path, zfName);
    }
}

void CVectmap::noOffsetUrl(QString& url) const
{
    QString txt;
    txt=QString("%1").arg(m_rect.m_w);
    url.replace("$W",txt);
    txt=QString("%1").arg(m_rect.m_e);
    url.replace("$E",txt);
    txt=QString("%1").arg(m_rect.m_s);
    url.replace("$S",txt);
    txt=QString("%1").arg(m_rect.m_n);
    url.replace("$N",txt);
}

void CVectmap::unpack(const SDLM_DATA & data, const QString & tile1Path) const
{
    if(!data.m_packer.isEmpty() && !data.m_unGz.isEmpty())
    {
        QString param=data.m_unGz;
        for(int i=0; i < param.size(); ++i)
        {
            if(param[i]!='$')
                continue;

            if(param[i+1]=='$')
            {
                param.remove(i);
            }
            else if(param[i+1]=='Z')
            {
                param.remove(i,2);
                param.insert(i,"\"\"");
                param.insert(i+1,m_pfadI);
            }
            else if(param[i+1]=='Q')
            {
                param.remove(i,2);
                param.insert(i,"\"\"");
                param.insert(i+1,tile1Path);
            }
        }
        QString cmd="\""+data.m_packer+"\" "+param;
        /*QProcess *mp = new QProcess();
        if(!mp->startDetached(cmd))
        {
            QMessageBox::warning(nullptr, QObject::tr("Vector-Tile"), QObject::tr("Fehler in unGz"),
                                          QMessageBox::Ok);
            return;
        }*/
    }
}

void CVectmap::renameFile(const SDLM_DATA & data, const QString & tile1Path, const QString & zfName) const
{
    auto pos=tile1Path.lastIndexOf('/');
    if(pos < 0)
        return;

    auto qfName=tile1Path.mid(pos+1);
    pos=qfName.lastIndexOf('.');
    if(pos>-1)
        qfName=qfName.left(pos);
    const auto quell=m_pfadI+qfName;
    const auto ziel2=m_pfadI+zfName+data.m_pictType;
    QFile fi(quell);
    fi.rename(ziel2);
}

