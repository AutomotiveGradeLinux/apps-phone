TARGET = phone
QT = quickcontrols2 websockets

SOURCES = main.cpp phone.cpp fileplayer.cpp
HEADERS = phone.h numbertype.h fileplayer.h

CONFIG += link_pkgconfig
PKGCONFIG += qlibhomescreen qlibwindowmanager qtappfw libafbwsc gstreamer-1.0

RESOURCES += \
    phone.qrc \
    images/images.qrc

include(app.pri)
