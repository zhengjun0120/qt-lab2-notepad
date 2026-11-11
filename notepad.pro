QT       += core gui widgets svg

CONFIG   += c++17
TARGET    = notepad
TEMPLATE  = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    codeeditor.cpp \
    finddialog.cpp \
    replacedialog.cpp \
    aboutdialog.cpp \
    windowchrome.cpp

HEADERS += \
    mainwindow.h \
    codeeditor.h \
    finddialog.h \
    replacedialog.h \
    aboutdialog.h \
    windowchrome.h

FORMS += \
    mainwindow.ui \
    finddialog.ui \
    replacedialog.ui \
    aboutdialog.ui

LIBS += -ldwmapi

RESOURCES += resources.qrc
