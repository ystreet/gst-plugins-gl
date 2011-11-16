noinst_PROGRAMS = filternoxoverlay

filternoxoverlay_SOURCES = main.cpp

filternoxoverlay_CXXFLAGS=$(GST_PLUGINS_GL_CFLAGS) $(GST_PLUGINS_BASE_CFLAGS) $(GST_CXXFLAGS) \
	$(GL_CFLAGS) $(GTK_CFLAGS)
filternoxoverlay_LDADD=$(GTK_LIBS) $(GST_PLUGINS_GL_LIBS) $(GST_PLUGINS_BASE_LIBS) $(GST_LIBS)  \
	$(GL_LIBS) -lgstinterfaces-$(GST_MAJORMINOR)

