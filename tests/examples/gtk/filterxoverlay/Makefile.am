noinst_PROGRAMS = filterxoverlay

filterxoverlay_SOURCES = main.cpp

filterxoverlay_CPPFLAGS=$(GST_PLUGINS_GL_CFLAGS) $(GST_CFLAGS) \
	$(GL_CFLAGS) $(GTK_CFLAGS)
filterxoverlay_LDADD=$(GST_PLUGINS_GL_LIBS) $(GST_LIBS) \
	$(GL_LIBS) $(GTK_LIBS) -lgstinterfaces-$(GST_MAJORMINOR)

