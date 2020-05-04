#-------------------------------------------------
#
# Project created by QtCreator 2019-01-15T21:43:56
#
#-------------------------------------------------

QT       += gui widgets network

TARGET = myLibsQT
TEMPLATE = lib
CONFIG += staticlib

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        cavifile.cpp \
        cgeopoint.cpp \
        cgeorect.cpp  \
        cgpxinfo.cpp  \
	cmapsrc.cpp   \
	cosm.cpp      \
	cosmopt.cpp   \
	cpath.cpp     \
	cpixmap.cpp   \
	cxmlfile.cpp  \
        Logfile.cpp   \
        trackcol.cpp   \
        urldownload.cpp \
	Vectmap.cpp   
        
HEADERS += \
        cavifile.h \
        cgeopoint.h \
       	cgeorect.h \ 
        cgpxinfo.h  \
        cmapsrc.h \
	cosm.h \ 
	cosmopt.h \ 
	cpath.h \ 
	cpixmap.h \ 
	cxmlfile.h \ 
	Logfile.h \ 
        trackcol.h   \
        urldownload.h \
	Vectmap.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

