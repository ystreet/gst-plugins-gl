if GL_BACKEND_COCOA

noinst_PROGRAMS = videoxoverlay

videoxoverlay_SOURCES = main.m

videoxoverlay_OBJCFLAGS=$(GST_PLUGINS_GL_CFLAGS) $(GST_PLUGINS_BASE_CFLAGS) $(GST_CFLAGS) \
	$(GL_CFLAGS) -I/usr/local/include/gstreamer-${GST_API_VERSION}
videoxoverlay_LDADD=$(GST_PLUGINS_GL_LIBS) $(GST_PLUGINS_BASE_LIBS) $(GST_LIBS) \
	$(GL_LIBS)
	
videoxoverlay_LIBTOOLFLAGS = --tag=OBJC

endif
