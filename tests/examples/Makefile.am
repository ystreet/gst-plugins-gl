
SUBDIRS =

if GL_IS_OPENGL

if GL_BACKEND_COCOA
SUBDIRS += cocoa
else

SUBDIRS += generic qt

if HAVE_CLUTTER
SUBDIRS += clutter
endif

if HAVE_SDL
SUBDIRS += sdl
endif

if HAVE_GTK
SUBDIRS += gtk
endif

endif

endif
