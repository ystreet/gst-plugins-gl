noinst_PROGRAMS = gtkxoverlay

gtkxoverlay_SOURCES = main.cpp

gtkxoverlay_CXXFLAGS=$(GST_PLUGINS_GL_CFLAGS) $(GST_PLUGINS_BASE_CFLAGS) $(GST_CXXFLAGS) \
	$(GL_CFLAGS) $(GTK_CFLAGS)
gtkxoverlay_LDADD=../libgstgtkhelper.la \
	$(GST_PLUGINS_GL_LIBS) $(GST_PLUGINS_BASE_LIBS) $(GST_LIBS) \
	$(GL_LIBS) $(GTK_LIBS) -lgstinterfaces-$(GST_MAJORMINOR)

