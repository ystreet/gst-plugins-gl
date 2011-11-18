GST_DOC_SCANOBJ = $(top_srcdir)/common/gstdoc-scangobj

## Process this file with automake to produce Makefile.in

# The name of the module, e.g. 'glib'.
#DOC_MODULE=gst-plugins-libs-@GST_MAJORMINOR@
MODULE=gst-plugins-gl
DOC_MODULE=$(MODULE)-plugins

# for upload-doc.mak
DOC=$(MODULE)-plugins
FORMATS=html
html: html-build.stamp
include $(top_srcdir)/common/upload-doc.mak

# The top-level SGML file. Change it if you want.
DOC_MAIN_SGML_FILE=$(DOC_MODULE)-docs.sgml

# The directory containing the source code. Relative to $(top_srcdir).
# gtk-doc will search all .c & .h files beneath here for inline comments
# documenting functions and macros.
DOC_SOURCE_DIR = $(top_srcdir)/gst

# Extra options to supply to gtkdoc-scan.
SCAN_OPTIONS=

# Extra options to supply to gtkdoc-mkdb.
MKDB_OPTIONS=--sgml-mode

# Extra options to supply to gtkdoc-fixref.
FIXXREF_OPTIONS=--extra-dir=$(GLIB_PREFIX)/share/gtk-doc/html \
	--extra-dir=$(GST_PREFIX)/share/gtk-doc/html \
        	--extra-dir=$(datadir)/gtk-doc/html

# Used for dependencies.
HFILE_GLOB=$(top_srcdir)/gst/*/*.h
CFILE_GLOB=$(top_srcdir)/gst/*/*.c

# Header files to ignore when scanning.
IGNORE_HFILES =
IGNORE_CFILES =

# we add all .h files of elements that have signals/args we want
# sadly this also pulls in the private methods - maybe we should
# move those around in the source ?
# also, we should add some stuff here conditionally based on whether
# or not the plugin will actually build
# but I'm not sure about that - it might be this Just Works given that
# the registry won't have the element

EXTRA_HFILES = \
	$(top_srcdir)/gst/gl/gstglbumper.h \
	$(top_srcdir)/gst/gl/gstglcolorscale.h \
	$(top_srcdir)/gst/gl/gstgldeinterlace.h \
	$(top_srcdir)/gst/gl/gstgldifferencematte.h \
	$(top_srcdir)/gst/gl/gstgldownload.h \
	$(top_srcdir)/gst/gl/gstgleffects.h \
	$(top_srcdir)/gst/gl/gstglfilterapp.h \
	$(top_srcdir)/gst/gl/gstglfilterblur.h \
	$(top_srcdir)/gst/gl/gstglfiltercube.h \
	$(top_srcdir)/gst/gl/gstglfilterglass.h \
	$(top_srcdir)/gst/gl/gstglfilterlaplacian.h \
	$(top_srcdir)/gst/gl/gstglfilterreflectedscreen.h \
	$(top_srcdir)/gst/gl/gstglfiltersobel.h \
	$(top_srcdir)/gst/gl/gstglfiltershader.h
	$(top_srcdir)/gst/gl/gstglimagesink.h \
	$(top_srcdir)/gst/gl/gstgloverlay.h \
	$(top_srcdir)/gst/gl/gstgltestsrc.h \
	$(top_srcdir)/gst/gl/gstglupload.h \
	$(top_srcdir)/gst/gl/gstglmosaic.h



# Images to copy into HTML directory.
HTML_IMAGES =

# Extra SGML files that are included by $(DOC_MAIN_SGML_FILE).
content_files =

# Other files to distribute.
extra_files =

# CFLAGS and LDFLAGS for compiling scan program. Only needed if your app/lib
# contains GtkObjects/GObjects and you want to document signals and properties.
GTKDOC_CFLAGS = $(GST_BASE_CFLAGS) -I$(top_builddir) -I$(top_builddir)/gst-libs
GTKDOC_LIBS = $(GST_BASE_LIBS)

GTKDOC_CC=$(LIBTOOL) --tag=CC --mode=compile $(CC)
GTKDOC_LD=$(LIBTOOL) --tag=CC --mode=link $(CC)

# If you need to override some of the declarations, place them in this file
# and uncomment this line.
#DOC_OVERRIDES = $(DOC_MODULE)-overrides.txt
DOC_OVERRIDES =

include $(top_srcdir)/common/gtk-doc-plugins.mak
