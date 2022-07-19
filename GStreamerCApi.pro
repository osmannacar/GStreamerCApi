QT -= gui
QT += network

CONFIG += c++17 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


PKGCONFIG += gstreamer-1.0 gstreamer-1.0-app

INCLUDEPATH +=  /usr/include/gstreamer-1.0 \
                /usr/include/glib-2.0 \
                /usr/lib/x86_64-linux-gnu/glib-2.0/include /

LIBS += -lgstreamer-1.0 -lglib-2.0 -lgobject-2.0 -lgstapp-1.0


HEADERS += \
    Enums.h \
    GstBase.h \
    gstdisplay/gstdisplay.h \
    gstdisplaycapture/gstdisplaycapture.h \
    gstmanager/gstmanager.h \
    gstrecord/gstrecord.h \
    gstsource/gstwebcamsource.h \
    gstudpstream/gstudpstream.h

SOURCES += \
        gstdisplay/gstdisplay.cpp \
        gstdisplaycapture/gstdisplaycapture.cpp \
        gstmanager/gstmanager.cpp \
        gstrecord/gstrecord.cpp \
        gstsource/gstwebcamsource.cpp \
        gstudpstream/gstudpstream.cpp \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

