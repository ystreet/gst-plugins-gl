noinst_PROGRAMS = gtkxoverlay

gtkxoverlay_SOURCES = main.cpp

gtkxoverlay_CPPFLAGS=$(GST_PLUGINS_GL_CFLAGS) $(GST_CFLAGS) \
	$(GL_CFLAGS) $(GTK_CFLAGS)
gtkxoverlay_LDADD=$(GST_PLUGINS_GL_LIBS) $(GST_LIBS) -lGLU  \
	$(GL_LIBS) $(X_LIBS) $(GTK_LIBS) -lgstinterfaces-$(GST_MAJORMINOR)

