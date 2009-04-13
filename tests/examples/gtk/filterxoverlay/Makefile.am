noinst_PROGRAMS = filterxoverlay

filterxoverlay_SOURCES = main.cpp

filterxoverlay_CXXFLAGS=$(GST_PLUGINS_GL_CFLAGS) $(GST_PLUGINS_BASE_CFLAGS) $(GST_CXXFLAGS) \
	$(GL_CFLAGS) $(GTK_CFLAGS)
filterxoverlay_LDADD=../libgstgtkhelper.la \
	$(GST_PLUGINS_GL_LIBS) $(GST_PLUGINS_BASE_LIBS) $(GST_LIBS) \
	$(GL_LIBS) $(GTK_LIBS) -lgstinterfaces-$(GST_MAJORMINOR)

