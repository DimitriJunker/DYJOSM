TEMPLATE = subdirs
TRANSLATIONS =taho_fr.ts\
    taho_en.ts\
    taho_xx.ts

SUBDIRS += \
    Taho \
    myLibsQT

Taho.depends = myLibsQT
