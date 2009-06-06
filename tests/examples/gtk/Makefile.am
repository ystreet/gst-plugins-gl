
SUBDIRS = . gtkxoverlay filternoxoverlay filterxoverlay fxtest switchxoverlay


noinst_LTLIBRARIES = libgstgtkhelper.la

libgstgtkhelper_la_SOURCES = gstgtk.c gstgtk.h
libgstgtkhelper_la_CFLAGS = $(GST_PLUGINS_BASE_CFLAGS) $(GST_CFLAGS) $(GTK_CFLAGS)

if GL_BACKEND_COCOA
libgstgtkhelper_la_CFLAGS += -x objective-c
endif

