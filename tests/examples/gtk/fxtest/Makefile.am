noinst_PROGRAMS = fxtest

fxtest_SOURCES = fxtest.c

fxtest_CPPFLAGS=$(GST_PLUGINS_GL_CFLAGS) $(GST_CFLAGS) \
	$(GL_CFLAGS) $(GTK_CFLAGS)
fxtest_LDADD=$(GST_PLUGINS_GL_LIBS) $(GST_LIBS) -lGLU  \
	$(GL_LIBS) $(X_LIBS) $(GTK_LIBS) -lgstinterfaces-$(GST_MAJORMINOR)

