QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    about_dialog.cpp \
    main.cpp \
    mainwindow.cpp \
    manual_dialog.cpp \
    process.cpp \
    scheduler.cpp \
    scheduling_controller.cpp \
    storage_manager.cpp

HEADERS += \
    about_dialog.h \
    mainwindow.h \
    manual_dialog.h \
    process.h \
    scheduler.h \
    scheduling_controller.h \
    storage_manager.h

FORMS += \
    about_dialog.ui \
    mainwindow.ui \
    manual_dialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
