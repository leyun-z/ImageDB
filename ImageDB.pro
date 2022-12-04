QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Dialog.cpp \
    main.cpp \
    Window.cpp

HEADERS += \
    Dialog.h \
    Window.h

FORMS += \
    Dialog.ui \
    Window.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    ./res/res.qrc

COPY_INFO.files = ImageDB.sqlite
COPY_INFO.path = $$OUT_PWD
COPIES += COPY_INFO
