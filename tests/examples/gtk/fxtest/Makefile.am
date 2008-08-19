noinst_PROGRAMS = fxtest 
if HAVE_GDKPIXBUF
noinst_PROGRAMS += pixbufdrop
endif

fxtest_SOURCES = fxtest.c

fxtest_CPPFLAGS=$(GST_PLUGINS_GL_CFLAGS) $(GST_CFLAGS) \
	$(GL_CFLAGS) $(GTK_CFLAGS)
fxtest_LDADD=$(GST_PLUGINS_GL_LIBS) $(GST_LIBS) -lGLU  \
	$(GL_LIBS) $(X_LIBS) $(GTK_LIBS) -lgstinterfaces-$(GST_MAJORMINOR)

if HAVE_GDKPIXBUF
pixbufdrop_SOURCES = pixbufdrop.c

pixbufdrop_CPPFLAGS=$(GST_PLUGINS_GL_CFLAGS) $(GST_CFLAGS) \
	$(GL_CFLAGS) $(GTK_CFLAGS)
pixbufdrop_LDADD=$(GST_PLUGINS_GL_LIBS) $(GST_LIBS) -lGLU  \
	$(GL_LIBS) $(X_LIBS) $(GTK_LIBS) -lgstinterfaces-$(GST_MAJORMINOR)
endif
