#-------------------------------------------------
#
# Project created by QtCreator 2018-12-20T18:17:17
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = wordSearcher
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    wordsearcher.cpp \
    mylistview.cpp \
    mytreeview.cpp \
    mymodel.cpp \
    errorevent.cpp \
    filehighlighterdialog.cpp \
    highlighter.cpp \
    trigramsearcher.cpp

HEADERS += \
        mainwindow.h \
    setunion.h \
    wordsearcher.h \
    setmanager.h \
    ui_mainwindow.h \
    mylistview.h \
    mytreeview.h \
    errorevent.h \
    directoryindexer.h \
    hashsearcher.h \
    filehighlighterdialog.h \
    ui_filehighlighterdialog.h \
    highlighter.h \
    trigramsearcher.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    img.qrc

FORMS +=
