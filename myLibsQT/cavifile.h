#ifndef CAVIFILE_H
#define CAVIFILE_H
#include <windows.h>
#include <vfw.h>
#include <QString>
#include <QImage>
class CAVIFile
{
public:
    CAVIFile(QString *fileName, int fps,qint64 maxAviLen);
    ~CAVIFile();
private:
    PAVIFILE m_pafComp;
    PAVISTREAM m_paviUnc;
public:
    PAVISTREAM m_paviComp;
    int fileOpen();
private:
    // Filename an den ggf. noch _1 usw angehängt wird
    QString m_basisFName;
    qint64 m_fileLen;
    BITMAPINFOHEADER m_bih;
    PGETFRAME m_pFrame;
public:
    AVISTREAMINFO m_asi;
    int writeFrame(QImage & imFrame, LPBITMAPINFOHEADER imHead);
    int readFrame(QImage &imFrame,LPBITMAPINFOHEADER imHead);
    int m_frameNr;
    int m_frameAnz;
    int m_frame1st;
    qint64 m_maxFileLen;
    void closeFile();
    int m_fileCount;
    int m_fps;
    AVICOMPRESSOPTIONS m_lpOptions;
    // Der Teil vom File Öffnen der ein CDibDrow braucht
    int fileInit(QImage & image, LPBITMAPINFOHEADER imHead);
    FILE *m_m3uFile;
    QString *m_pShowFile ;
};


#endif // CAVIFILE_H
