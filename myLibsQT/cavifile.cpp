#include "cavifile.h"
#include <QString>
#include <assert.h>


/*
Diese Klasse wurde für die Version 1.08a von DYJTrack erzeugt.


* 31.1.14 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ 1.08a'
    Die verwendeten Videoroutinen können fehlerfrei nur Files bis zu einer Länge von 2GB erzeugen. Deshalb wird jetzt ggf.
    in mehrere Files exportiert. Damit diese automatisch nacheinander angezeigt werden können erhalten ihre Namen einerseits
    Anhänge der Form _E01 und zusätzlich wird ein m3u File erzeugt.
*/

CAVIFile::CAVIFile(QString *pFileName, int fps, qint64 maxAviLen)
: m_basisFName("")
, m_fileLen(0)
, m_frameNr(0)
, m_maxFileLen(0)
, m_fileCount(0)
, m_fps(0)
, m_m3uFile(nullptr)
, m_pShowFile(nullptr)
{
    m_pafComp = nullptr;
    m_paviUnc = nullptr;
    m_paviComp = nullptr;
    m_maxFileLen = maxAviLen;
    m_fileCount = 0;
    m_basisFName = *pFileName;
    m_pShowFile = pFileName;
    m_fps = fps;
    m_m3uFile = nullptr;
    AVIFileInit();
}

CAVIFile::~CAVIFile()
{
    closeFile();
    AVIFileExit();
    if (m_m3uFile)
        fclose(m_m3uFile);

}


int CAVIFile::fileOpen()
{
    int err = 0;
    m_fileCount++;
    m_fileLen = 0;
    m_frameNr = -1;	//markiert, daß noch fileInit ausgeführt werden muß
    QString fileName = m_basisFName;
    int posBs = fileName.lastIndexOf('/')+1;
    if (m_fileCount>1)
    {
        int posPkt = fileName.lastIndexOf('.');
        if (posPkt > -1)
        {
            QString num;
            num.sprintf("_E%.2d", m_fileCount);
            fileName.insert(posPkt, num);
            if (m_fileCount == 2)
            {
                QString filename1 = m_basisFName;
                filename1.insert(posPkt, "_E01");
                remove(filename1.toStdString().c_str());					//sonst funktioniert rename ggf nicht
                if (rename(m_basisFName.toStdString().c_str(), filename1.toStdString().c_str()))	//konnte aus irgendeinem Grund nicht umbenannt werden
                    filename1 = m_basisFName;

                *m_pShowFile = m_basisFName.left(posPkt) + ".m3u";

                if (!fopen_s(&m_m3uFile, m_pShowFile->toStdString().c_str(), "w"))
                    fprintf(m_m3uFile, "%s\n", filename1.mid(posBs).toStdString().c_str());
            }
        }
        else
            err = 1005;
    }

    err=AVIFileOpen(&m_pafComp,fileName.toStdWString().c_str(), OF_CREATE | OF_WRITE, nullptr);
    if (!err && m_m3uFile)
        fprintf(m_m3uFile, "%s\n", fileName.mid(posBs).toStdString().c_str());

    return err;
}


    // Der Teil vom File Öffnen der ein QImage braucht
int CAVIFile::fileInit(QImage & image, LPBITMAPINFOHEADER imHead)
{
    m_frameNr = 0;
    int err = 0;
    memset(&m_asi, 0, sizeof(m_asi));
    m_asi.fccType = streamtypeVIDEO;
    m_asi.fccHandler = 0;
    m_asi.dwScale = 1;
    m_asi.dwRate = m_fps;
    m_asi.dwSuggestedBufferSize = DWORD(4*image.width()*image.height());
    SetRect(&m_asi.rcFrame, 0, 0,		    // rectangle for stream
            image.width(),image.height());
    err = AVIFileCreateStream(m_pafComp, &m_paviUnc, &m_asi);
    if (!err)
    {
//        static AVICOMPRESSOPTIONS lpOptionsSaved;
        AVICOMPRESSOPTIONS FAR * plpOptions[1] = { &m_lpOptions };
        INT_PTR ok=TRUE;
        if(m_fileCount==1)
        {
            memset(&m_lpOptions, 0, sizeof(m_lpOptions));
            ok = AVISaveOptions(nullptr, ICMF_CHOOSE_KEYFRAME, 1, &m_paviUnc, (LPAVICOMPRESSOPTIONS *)&plpOptions);
        }
        if (!ok)
            err = 1000;
        else
            err = AVIMakeCompressedStream(&m_paviComp, m_paviUnc, &m_lpOptions, nullptr);
    }
    if (!err)
        err = AVIStreamSetFormat(m_paviComp, ICMF_CHOOSE_DATARATE | ICMF_CHOOSE_PREVIEW, imHead, long(imHead->biSize));
    return err;
}


int CAVIFile::writeFrame(QImage &imFrame,LPBITMAPINFOHEADER imHead)
{
    long bytesW;
    if ((m_fileLen + 10000000) > m_maxFileLen)
    {
        closeFile();
        fileOpen();
    }
    int err = 0;
    if (m_frameNr < 0)
        err=fileInit(imFrame,imHead);
    if (!err)
    {
        err = AVIStreamWrite(m_paviComp, m_frameNr++, 1, imFrame.mirrored().bits(),
            long(imHead->biSizeImage), AVIIF_KEYFRAME, nullptr, &bytesW);
        m_fileLen += bytesW;
    }

    return err;
}



void CAVIFile::closeFile()
{
    if (m_paviComp)
        AVIStreamRelease(m_paviComp);
    if (m_paviUnc)
        AVIStreamRelease(m_paviUnc);
    if (m_pafComp)
        AVIFileRelease(m_pafComp);
}


