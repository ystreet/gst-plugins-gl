
pcverfiles = gstreamer-gl-@GST_API_VERSION@.pc

pcverfiles_uninstalled = gstreamer-gl-@GST_API_VERSION@-uninstalled.pc

all-local: $(pcverfiles) $(pcverfiles_uninstalled)

### how to generate versioned .pc files from .pc files in this dir
%-@GST_API_VERSION@.pc: %.pc
	cp $< $@
%-@GST_API_VERSION@-uninstalled.pc: %-uninstalled.pc
	cp $< $@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = $(pcverfiles)

CLEANFILES = $(pcverfiles) $(pcverfiles_uninstalled)
pcinfiles = \
	gstreamer-gl.pc.in gstreamer-gl-uninstalled.pc.in

DISTCLEANFILES = $(pcinfiles:.in=)
EXTRA_DIST = $(pcinfiles)

