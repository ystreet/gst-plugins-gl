
lib_LTLIBRARIES = libgstfreeglut-@GST_MAJORMINOR@.la

libgstfreeglut_@GST_MAJORMINOR@_la_CFLAGS = $(GL_CFLAGS) $(X_CFLAGS)
libgstfreeglut_@GST_MAJORMINOR@_la_LIBADD = $(GL_LIBS) $(X_LIBS)

libgstfreeglut_@GST_MAJORMINOR@_la_SOURCES = \
	freeglut_callbacks.c \
	freeglut_display.c \
	freeglut_init.c \
	freeglut_main.c \
	freeglut_state.c \
	freeglut_structure.c \
	freeglut_window.c

libgstfreeglut_@GST_MAJORMINOR@includedir = $(includedir)/gstreamer-@GST_MAJORMINOR@/gst/freeglut
libgstfreeglut_@GST_MAJORMINOR@include_HEADERS = \
	gstfreeglut.h \
	freeglut_ext.h \
	freeglut_std.h \
	freeglut_internal.h


