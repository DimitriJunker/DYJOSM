#-------------------------------------------------
#
# Project created by QtCreator 2014-10-25T12:06:08
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QT += concurrent

TARGET = DyjTrack
TEMPLATE = app
TRANSLATIONS =dyjtrack_fr.ts\
    dyjtrack_en.ts\
    dyjtrack_xx.ts

SOURCES += main.cpp\
    cmakehtml.cpp \
    dialog.cpp \
    flugdialog.cpp \
    newairpdialog.cpp \
    cdyjtopt.cpp \
    cmakepix.cpp \
    cgpxproper.cpp \
    dyjmainw.cpp \
    cdyjtcopy.cpp

HEADERS  += \
    urldownload.h \
    cmakehtml.h \
    dialog.h \
    flugdialog.h \
    newairpdialog.h \
    cdyjtopt.h \
    cmakepix.h \
    cgpxproper.h \
    dyjmainw.h \
    cdyjtcopy.h

FORMS    += \
    flugdialog.ui \
    newairpdialog.ui \
    cdyjtopt.ui \
    cmakepix.ui\
    cmakehtml.ui \
    cgpxproper.ui \
    dyjmainw.ui \
    cdyjtcopy.ui


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../myLibsQT/release/ -lmyLibsQT -lvfw32
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../myLibsQT/debug/ -lmyLibsQT -lvfw32 -luser32

INCLUDEPATH += $$PWD/../myLibsQT
DEPENDPATH += $$PWD/../myLibsQT

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../myLibsQT/release/libmyLibsQT.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../myLibsQT/debug/libmyLibsQT.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../myLibsQT/release/myLibsQT.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../myLibsQT/debug/myLibsQT.lib
