TARGET = phone
QT = quickcontrols2 websockets

SOURCES = main.cpp phone.cpp fileplayer.cpp
HEADERS = phone.h numbertype.h fileplayer.h

CONFIG += link_pkgconfig
PKGCONFIG += libhomescreen qlibwindowmanager qtappfw-phone qtappfw-bt-pbap gstreamer-1.0

CONFIG(release, debug|release) {
    QMAKE_POST_LINK = $(STRIP) --strip-unneeded $(TARGET)
}

RESOURCES += \
    phone.qrc \
    images/images.qrc

include(app.pri)
