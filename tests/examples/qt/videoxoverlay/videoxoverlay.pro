TEMPLATE = app
TARGET = videoxoverlay
DESTDIR = ./Debug
CONFIG += debug
DEFINES += UNICODE QT_THREAD_SUPPORT QT_CORE_LIB QT_GUI_LIB
	
win32 {
DEFINES += WIN32
INCLUDEPATH += ./GeneratedFiles \
    ./GeneratedFiles/Debug \
	./../../../../libxml2-2.6.30+.win32/include \
    ./../../../../libiconv/include \
    ./../../../../glib/include \
    ./../../../../gstreamer/include
LIBS += -L"./../../../../glib/lib" \
    -L"./../../../../gstreamer/lib" \
    -llibgstreamer-0.10 \
    -lglib-2.0 \
    -lgmodule-2.0 \
    -lgobject-2.0 \
    -lgthread-2.0 \
    -llibgstinterfaces-0.10
}

unix {
DEFINES += UNIX
INCLUDEPATH += GeneratedFiles \
    GeneratedFiles/Debug \
	/usr/include/gstreamer-0.10 \
    /usr/include/glib-2.0 \
    /usr/lib/glib-2.0/include \
    /usr/include/libxml2
LIBS += -lgstreamer-0.10 \
    -lgstinterfaces-0.10 \
	-lglib-2.0 \
	-lgmodule-2.0 \
	-lgobject-2.0 \
	-lgthread-2.0 \
	-lGLU \
	-lGL \
	-lGLEW
}

DEPENDPATH += .
MOC_DIR += ./GeneratedFiles/debug
OBJECTS_DIR += debug
UI_DIR += ./GeneratedFiles
RCC_DIR += ./GeneratedFiles

#Include file(s)
include(videoxoverlay.pri)
