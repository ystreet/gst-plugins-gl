noinst_PROGRAMS = gtkvideooverlay

gtkvideooverlay_SOURCES = main.cpp

gtkvideooverlay_CXXFLAGS=$(GST_PLUGINS_GL_CFLAGS) $(GST_PLUGINS_BASE_CFLAGS) $(GST_CXXFLAGS) \
	$(GL_CFLAGS) $(GTK_CFLAGS)
gtkvideooverlay_LDADD=../libgstgtkhelper.la \
	$(GTK_LIBS) $(GST_PLUGINS_GL_LIBS) $(GST_PLUGINS_BASE_LIBS) $(GST_LIBS) \
	$(GL_LIBS) -lgstvideo-$(GST_API_VERSION)

