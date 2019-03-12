#-------------------------------------------------
#
# Project created by QtCreator 2014-10-25T12:06:08
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QT += concurrent

TARGET = Taho
TEMPLATE = app
TRANSLATIONS =taho_fr.ts\
    taho_en.ts\
    taho_xx.ts

SOURCES += main.cpp\
        tahomainw.cpp \
    ctahoopt.cpp \
    cdokmz.cpp \
    ctahocopy.cpp \
    tahostyle.cpp

HEADERS  += tahomainw.h \
    ctahoopt.h \
    cdokmz.h \
    ctahocopy.h \
    tahostyle.h

FORMS    += tahomainw.ui \
    ctahoopt.ui \
    cdokmz.ui \
    ctahocopy.ui

RESOURCES += \
    taho.qrc

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../myLibsQT/release/ -lmyLibsQT
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../myLibsQT/debug/ -lmyLibsQT

INCLUDEPATH += $$PWD/../myLibsQT
DEPENDPATH += $$PWD/../myLibsQT

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../myLibsQT/release/libmyLibsQT.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../myLibsQT/debug/libmyLibsQT.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../myLibsQT/release/myLibsQT.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../myLibsQT/debug/myLibsQT.lib
