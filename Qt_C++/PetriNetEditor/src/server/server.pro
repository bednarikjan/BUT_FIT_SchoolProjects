#-------------------------------------------------
#
# Project created by QtCreator 2012-04-26T15:23:30
#
#-------------------------------------------------

QT       += network

TEMPLATE = app
TARGET = server2012
CONFIG   += console
CONFIG -= app_bundle
SOURCES += main.cpp \
    myserver.cpp \
    ../shared/myXML.cpp \
    ../shared/petrinetplacecore.cpp \
    ../shared/lists.cpp \
    parser.cpp \
    simulation.cpp

HEADERS += \
    myserver.h \
    ../shared/myXML.h \
    ../shared/petrinetplacecore.h \
    ../shared/lists.h \
    parser.h \
    simulation.h

