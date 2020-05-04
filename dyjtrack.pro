TEMPLATE = subdirs
TRANSLATIONS =dyjtrack_fr.ts\
    dyjtrack_en.ts\
    dyjtrack_xx.ts

SUBDIRS += \
    dyjtrack \
    myLibsQT

dyjtrack.depends = myLibsQT
