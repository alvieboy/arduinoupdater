#-------------------------------------------------
#
# Project created by QtCreator 2014-03-20T18:36:48
#
#-------------------------------------------------

QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ArduinoUpdateManager
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp\
        updatelistmodel.cpp\
        scanner.cpp\
        manager.cpp \
    releasedialog.cpp\
    compressor.cpp

HEADERS  += mainwindow.h  updatelistmodel.h scanner.h manager.h \
    releasedialog.h compressor.h

FORMS    += mainwindow.ui \
    releasedialog.ui

LIBS += -lz
