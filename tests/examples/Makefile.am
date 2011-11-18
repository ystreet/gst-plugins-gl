
SUBDIRS = qt wx

if GL_IS_OPENGL
SUBDIRS += clutter
if HAVE_CLUTTER
SUBDIRS += clutter
endif
if HAVE_SDL
SUBDIRS += sdl
endif
endif

if HAVE_GTK
SUBDIRS += gtk
endif

if GL_BACKEND_COCOA
SUBDIRS += cocoa
endif 
