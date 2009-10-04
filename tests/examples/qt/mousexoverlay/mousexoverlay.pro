TEMPLATE = app
TARGET = mousexoverlay
DESTDIR = ./debug
CONFIG += debug
DEFINES += UNICODE QT_THREAD_SUPPORT QT_CORE_LIB QT_GUI_LIB

win32 {
DEFINES += WIN32
INCLUDEPATH +=  ./GeneratedFiles \
    ./GeneratedFiles/Debug \
    C:/gstreamer/include/libxml2 \
    ./../../../../../libiconv/include \
    ./../../../../../glib/include/glib-2.0 \
    ./../../../../../glib/lib/glib-2.0/include \
    C:/gstreamer/include/gstreamer-0.10 \
    ./../../../../../glew/include
LIBS += -L"./../../../../../glib/lib" \
    -L"C:/gstreamer/lib" \
    -L"C:/gstreamer/bin" \
    -L"./../../../../../glew/lib" \
    -lgstreamer-0.10 \
    -lglib-2.0 \
    -lgmodule-2.0 \
    -lgobject-2.0 \
    -lgthread-2.0 \
    -llibgstinterfaces-0.10 \
    -lopengl32 \
    -lglu32 \
    -lglew32 \
    -lglew32s
}

unix {
DEFINES += UNIX
INCLUDEPATH += GeneratedFiles \
    GeneratedFiles/Debug \
    /usr/include/gstreamer-0.10 \
    /usr/local/include/gstreamer-0.10 \
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
include(mousexoverlay.pri)
