noinst_PROGRAMS = fxtest 
noinst_PROGRAMS += pixbufdrop

fxtest_SOURCES = fxtest.c

fxtest_CFLAGS=$(GST_PLUGINS_GL_CFLAGS) $(GST_PLUGINS_BASE_CFLAGS) $(GST_CFLAGS) \
	$(GL_CFLAGS) $(GTK_CFLAGS)
fxtest_LDADD=../libgstgtkhelper.la \
	$(GST_PLUGINS_GL_LIBS) $(GST_PLUGINS_BASE_LIBS) $(GST_LIBS)  \
	$(GL_LIBS) $(GTK_LIBS) -lgstinterfaces-$(GST_MAJORMINOR)

pixbufdrop_SOURCES = pixbufdrop.c

pixbufdrop_CFLAGS=$(GST_PLUGINS_GL_CFLAGS) $(GST_PLUGINS_BASE_CFLAGS) $(GST_CFLAGS) \
	$(GL_CFLAGS) $(GTK_CFLAGS)
pixbufdrop_LDADD=../libgstgtkhelper.la \
	$(GST_PLUGINS_GL_LIBS) $(GST_PLUGINS_BASE_LIBS) $(GST_LIBS) \
	$(GL_LIBS) $(GTK_LIBS) -lgstinterfaces-$(GST_MAJORMINOR)
