
pcverfiles = gstreamer-gl-@GST_MAJORMINOR@.pc

pcverfiles_uninstalled = gstreamer-gl-@GST_MAJORMINOR@-uninstalled.pc

all-local: $(pcverfiles) $(pcverfiles_uninstalled)

### how to generate versioned .pc files from .pc files in this dir
%-@GST_MAJORMINOR@.pc: %.pc
	cp $< $@
%-@GST_MAJORMINOR@-uninstalled.pc: %-uninstalled.pc
	cp $< $@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = $(pcverfiles)

CLEANFILES = $(pcverfiles) $(pcverfiles_uninstalled)
pcinfiles = \
	gstreamer-gl.pc.in gstreamer-gl-uninstalled.pc.in

DISTCLEANFILES = $(pcinfiles:.in=)
EXTRA_DIST = $(pcinfiles)

