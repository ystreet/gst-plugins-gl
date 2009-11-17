TEMPLATE = app
TARGET = qglwtextureshare
QT += opengl
# Add console to the CONFIG to see debug messages printed in 
# the console on Windows
#CONFIG += console
DEFINES += UNICODE QT_THREAD_SUPPORT QT_CORE_LIB QT_GUI_LIB

win32 {
DEFINES += WIN32
INCLUDEPATH += \
    C:/gstreamer/include/libxml2 \
    ./../../../../../libiconv/include \
    ./../../../../../glib/include/glib-2.0 \
    ./../../../../../glib/lib/glib-2.0/include \
    ./../../../../../glew/include \
    C:/gstreamer/include/gstreamer-0.10 \
    /usr/src/ossbuild/Shared/Build/Windows/Win32/include/glib-2.0 \
    /usr/src/ossbuild/Shared/Build/Windows/Win32/lib/glib-2.0/include \
    /usr/src/ossbuild/Build/Windows/Win32/Release/include/gstreamer-0.10 \
    /usr/src/ossbuild/Shared/Build/Windows/Win32/include/libxml2 \
    /usr/src/ossbuild/Shared/SDKs/MSVCRT/include/gl
LIBS += -L"./../../../../../glib/lib" \
    -L"C:/gstreamer/lib" \
    -L"C:/gstreamer/bin" \
    -L"./../../../../../glew/lib" \
    -L\usr\src\ossbuild\Build\Windows\Win32\Release\lib\gstreamer \
    -L\usr\src\ossbuild\Shared\Build\Windows\Win32\lib \
    -lgstreamer \
    -lglib-2.0 \
    -lgmodule-2.0 \
    -lgobject-2.0 \
    -lgthread-2.0 \
    -lgstinterfaces \
    -lopengl32 \
    -lglu32 \
    -lglew32 \
    -lglew32s
}

unix {
DEFINES += UNIX
INCLUDEPATH += \
    /usr/include/gstreamer-0.10 \
    /usr/local/include/gstreamer-0.10 \
    /usr/include/glib-2.0 \
    /usr/lib/glib-2.0/include \
    /usr/include/libxml2
LIBS += \
    -lgstreamer-0.10 \
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

#Header files
HEADERS += \
    gstthread.h \
    pipeline.h \
    qglrenderer.h \
    AsyncQueue.h \
    GstGLBufferDef.h \
    glcontextid.h

#Source files
SOURCES += \
    gstthread.cpp \
    main.cpp \
    pipeline.cpp \
    qglrenderer.cpp
