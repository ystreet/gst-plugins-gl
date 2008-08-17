noinst_PROGRAMS = ##

if HAVE_CLUTTER_GLX
if HAVE_CLUTTER_X11

noinst_PROGRAMS += clutteractor

clutteractor_SOURCES = clutteractor.c

clutteractor_CPPFLAGS=$(GST_PLUGINS_GL_CFLAGS) $(GST_CFLAGS) \
	$(GL_CFLAGS) $(GTK_CFLAGS) \
	$(CLUTTER_CFLAGS) \
	$(CLUTTER_GLX_CFLAGS) $(CLUTTER_X11_CFLAGS)
clutteractor_LDADD=$(GST_PLUGINS_GL_LIBS) $(GST_LIBS) -lGLU  \
	$(GL_LIBS) $(X_LIBS) $(GTK_LIBS) -lgstinterfaces-$(GST_MAJORMINOR) \
	$(CLUTTER_LIBS) \
	$(CLUTTER_GLX_LIBS) $(CLUTTER_X11_LIBS)

endif
endif