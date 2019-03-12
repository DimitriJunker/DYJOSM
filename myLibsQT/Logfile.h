#ifndef CLOGFILE_H
#define CLOGFILE_H
// Logfile.h: Schnittstelle für die Klasse CLogfile.
//
//////////////////////////////////////////////////////////////////////
#include <qstring.h>
#define writeFLS(a) write(__FILE__,__LINE__,a)

class CLogfile  
{
public:
    void write(QString file,int line,QString str="");
    void closeHtml();
    static QString m_path;
	CLogfile();
	virtual ~CLogfile();

};

#endif // CLOGFILE_H
