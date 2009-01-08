noinst_PROGRAMS = filternoxoverlay

filternoxoverlay_SOURCES = main.cpp

filternoxoverlay_CPPFLAGS=$(GST_PLUGINS_GL_CFLAGS) $(GST_CFLAGS) \
	$(GL_CFLAGS) $(GTK_CFLAGS)
filternoxoverlay_LDADD=$(GST_PLUGINS_GL_LIBS) $(GST_LIBS)  \
	$(GL_LIBS) $(GTK_LIBS) -lgstinterfaces-$(GST_MAJORMINOR)

