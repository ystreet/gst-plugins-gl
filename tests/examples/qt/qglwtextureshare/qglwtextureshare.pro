TEMPLATE = app
TARGET = qglwtextureshare
QT += opengl

# Add console to the CONFIG to see debug messages printed in 
# the console on Windows
# CONFIG += console
DEFINES += UNICODE QT_THREAD_SUPPORT QT_CORE_LIB QT_GUI_LIB

win32 {
DEFINES += WIN32
INCLUDEPATH += \
    C:/gstreamer/include \
    C:/gstreamer/include/libxml2 \
    C:/gstreamer/include/glib-2.0 \
    C:/gstreamer/lib/glib-2.0/include \
    C:/gstreamer/include/gstreamer-0.10
LIBS += -L"C:/gstreamer/lib" \
    -L"C:/gstreamer/bin" \
    -lgstreamer-0.10 \
    -lglib-2.0 \
    -lgmodule-2.0 \
    -lgobject-2.0 \
    -lgthread-2.0 \
    -lgstinterfaces-0.10 \
    -lopengl32 \
    -lglu32 \
    -lglew32
}
unix:!mac {
    DEFINES += UNIX
    INCLUDEPATH += /usr/include/gstreamer-0.10 \
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
mac {
    DEFINES += MACOSX
    INCLUDEPATH += /opt/local/include/ \
        /opt/local/include/gstreamer-0.10/ \
        /opt/local/include/glib-2.0/ \
        /opt/local/lib/glib-2.0/include \
        /opt/local/include/libxml2
    LIBS += -L/opt/local/lib \
        -lGLEW \
        -lgstreamer-0.10 \
        -lgstapp-0.10 \
        -lgstvideo-0.10 \
        -lglib-2.0 \
        -lgobject-2.0 \
        -lcxcore \
        -lcvaux \
        -lcv
    OBJECTIVE_SOURCES +=  cocoa_utils.mm
    LIBS += -framework AppKit
}
DEPENDPATH += .

# Header files
HEADERS += gstthread.h \
    pipeline.h \
    qglrenderer.h \
    AsyncQueue.h \
    GstGLBufferDef.h \
    glcontextid.h

# Source files
SOURCES += gstthread.cpp \
    main.cpp \
    pipeline.cpp \
    qglrenderer.cpp
