TEMPLATE = subdirs

SUBDIRS += \
    dyjtrack \
    myLibsQT

dyjtrack.depends = myLibsQT
