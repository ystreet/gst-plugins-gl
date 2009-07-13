AC_PREREQ(2.52)

dnl please read gstreamer/docs/random/autotools before changing this file

dnl initialize autoconf
dnl releases only do -Wall, cvs and prerelease does -Werror too
dnl use a three digit version number for releases, and four for cvs/prerelease
AC_INIT(GStreamer OpenGL Plug-ins, 0.10.1,
    http://bugzilla.gnome.org/enter_bug.cgi?product=GStreamer,
    gst-plugins-gl)

AG_GST_INIT

dnl initialize automake
AM_INIT_AUTOMAKE

dnl define PACKAGE_VERSION_* variables
AS_VERSION

dnl check if this is a release version
AS_NANO(GST_CVS="no", GST_CVS="yes")

dnl libtoolize complains about wrong macros
AC_CONFIG_MACRO_DIR([m4])

dnl can autoconf find the source ?
AC_CONFIG_SRCDIR([gst-libs/gst/gl/gstgldisplay.c])

dnl define the output header for config
AM_CONFIG_HEADER([config.h])

dnl AM_MAINTAINER_MODE only provides the option to configure to enable it
AM_MAINTAINER_MODE

dnl sets host_* variables
AC_CANONICAL_HOST

dnl our libraries and install dirs use major.minor as a version
GST_MAJORMINOR=$PACKAGE_VERSION_MAJOR.$PACKAGE_VERSION_MINOR
dnl we override it here if we need to for the release candidate of new series
GST_MAJORMINOR=0.10
AC_SUBST(GST_MAJORMINOR)

dnl CURRENT, REVISION, AGE
dnl - library source changed -> increment REVISION
dnl - interfaces added/removed/changed -> increment CURRENT, REVISION = 0
dnl - interfaces added -> increment AGE
dnl - interfaces removed -> AGE = 0
dnl sets GST_LT_LDFLAGS
AS_LIBTOOL(GST, 0, 0, 0)

dnl FIXME: this macro doesn't actually work;
dnl the generated libtool script has no support for the listed tags.
dnl So this needs to be fixed first if we want to use this
dnl AS_LIBTOOL_TAGS

AC_LIBTOOL_WIN32_DLL
AM_PROG_LIBTOOL
AS_PROG_OBJC

dnl *** required versions of GStreamer stuff ***
GST_REQ=0.10.15.1
GSTPB_REQ=0.10.15.1

dnl *** autotools stuff ****

dnl allow for different autotools
AS_AUTOTOOLS_ALTERNATE

dnl Add parameters for aclocal
AC_SUBST(ACLOCAL_AMFLAGS, "-I m4 -I common/m4")

dnl set up gettext
dnl the version check needs to stay here because autopoint greps for it
AM_GNU_GETTEXT_VERSION([0.11.5])
AM_GNU_GETTEXT([external])
AG_GST_GETTEXT([gst-plugins-gl-$GST_MAJORMINOR])

dnl *** check for arguments to configure ***

AG_GST_ARG_DEBUG
AG_GST_ARG_PROFILING
AG_GST_ARG_VALGRIND
AG_GST_ARG_GCOV

AG_GST_ARG_EXAMPLES

AG_GST_ARG_WITH_PKG_CONFIG_PATH
AG_GST_ARG_WITH_PACKAGE_NAME
AG_GST_ARG_WITH_PACKAGE_ORIGIN
AG_GST_ARG_WITH_PLUGINS

dnl *** checks for platform ***

dnl * hardware/architecture *
AC_CHECK_SIZEOF(void *)

dnl common/m4/gst-arch.m4
dnl check CPU type
AG_GST_ARCH

dnl check for large file support
dnl affected plugins must include config.h
AC_SYS_LARGEFILE

dnl *** checks for programs ***

dnl find a compiler
AC_PROG_CC
AC_PROG_CXX

dnl check if the compiler supports '-c' and '-o' options
AM_PROG_CC_C_O

AC_PATH_PROG(VALGRIND_PATH, valgrind, no)
AM_CONDITIONAL(HAVE_VALGRIND, test ! "x$VALGRIND_PATH" = "xno")

dnl check for documentation tools
GTK_DOC_CHECK([1.3])
AS_PATH_PYTHON([2.1])
AG_GST_PLUGIN_DOCS([1.3],[2.1])

dnl *** checks for libraries ***

dnl For interactive UNIX (a Sun thing)
dnl FIXME: this adds -lcposix to LIBS, but I doubt we use LIBS
AC_ISC_POSIX

dnl libm, for sin() etc.
AC_CHECK_LIBM
AC_SUBST(LIBM)

dnl *** checks for header files ***

dnl check if we have ANSI C header files
AC_HEADER_STDC

dnl *** checks for types/defines ***

dnl *** checks for structures ***

dnl *** checks for compiler characteristics ***

dnl make sure we can use "inline" from C code
AC_C_INLINE

dnl *** checks for library functions ***

dnl Check for a way to display the function name in debug output
AG_GST_CHECK_FUNCTION

dnl *** checks for dependency libraries ***

dnl GLib is required
AG_GST_GLIB_CHECK([2.6])

dnl liboil is required
PKG_CHECK_MODULES(LIBOIL, liboil-0.3 >= 0.3.8, HAVE_LIBOIL=yes, HAVE_LIBOIL=no)
if test "x$HAVE_LIBOIL" != "xyes"
then
  AC_ERROR([liboil-0.3.8 or later is required])
fi

#dnl Check for OpenGL, GLU and GLEW
echo host is $host
case $host in
  *-linux*)
    AG_GST_CHECK_X
    save_CPPFLAGS="$CPPFLAGS"
    save_LIBS="$LIBS"
    CPPFLAGS="$CPPFLAGS $X_CFLAGS"
    LIBS="$LIBS $X_LIBS"
    AG_GST_CHECK_LIBHEADER(GL, GL, glTexImage2D,, GL/gl.h,, AC_MSG_ERROR([OpenGL is required]))
    AG_GST_CHECK_LIBHEADER(GLU, GLU, gluSphere,, GL/glu.h,, AC_MSG_ERROR([GLU is required]))
    AG_GST_CHECK_LIBHEADER(GLEW, GLEW, glewInit,, GL/glew.h,, AC_MSG_ERROR([GLEW is required]))
    GL_LIBS="$LIBS $X_LIBS -lGL -lGLU -lGLEW"
    CPPFLAGS="$save_CPPFLAGS"
    LIBS="$save_LIBS"
    GL_BACKEND=x11
    ;;
  *-darwin*)
    save_CPPFLAGS="$CPPFLAGS"
    save_LIBS="$LIBS"
    AG_GST_CHECK_LIBHEADER(GLEW, GLEW, glewInit,, GL/glew.h,, AC_MSG_ERROR([GLEW is required]))
    GL_LIBS="$LIBS -lGLEW -framework OpenGL -framework Cocoa"
    CPPFLAGS="$save_CPPFLAGS -framework OpenGL"
    LIBS="$save_LIBS"
    GL_BACKEND=cocoa
    ;;
  *-mingw32*)
    save_CPPFLAGS="$CPPFLAGS"
    save_LIBS="$LIBS"
    LIBS="$LIBS -lgdi32"
    AG_GST_CHECK_LIBHEADER(GL, opengl32, glTexImage2D,, GL/gl.h)
    AG_GST_CHECK_LIBHEADER(GLU, glu32, gluSphere,, GL/glu.h)
    AG_GST_CHECK_LIBHEADER(GLEW, glew32, glewInit,, GL/glew.h)
    GL_LIBS="$LIBS -lgdi32 -lopengl32 -lglu32 -lglew32"
    CPPFLAGS="$save_CPPFLAGS"
    LIBS="$save_LIBS"
    GL_BACKEND=win32
    ;;
  *)
    exit 1
    ;;
esac
AC_SUBST(GL_LIBS)
AC_SUBST(GL_BACKEND)
AM_CONDITIONAL(GL_BACKEND_X11, test "x$GL_BACKEND" = "xx11")
AM_CONDITIONAL(GL_BACKEND_COCOA, test "x$GL_BACKEND" = "xcocoa")
AM_CONDITIONAL(GL_BACKEND_WIN32, test "x$GL_BACKEND" = "xwin32")

dnl checks for gstreamer
dnl uninstalled is selected preferentially -- see pkg-config(1)
AG_GST_CHECK_GST($GST_MAJORMINOR, [$GST_REQ], yes)
AG_GST_CHECK_GST_BASE($GST_MAJORMINOR, [$GST_REQ], yes)
AG_GST_CHECK_GST_GDP($GST_MAJORMINOR, [$GST_REQ], yes)
AG_GST_CHECK_GST_CONTROLLER($GST_MAJORMINOR, [$GST_REQ], yes)
AG_GST_CHECK_GST_CHECK($GST_MAJORMINOR, [$GST_REQ], no)
AG_GST_CHECK_GST_PLUGINS_BASE($GST_MAJORMINOR, [$GSTPB_REQ], yes)
AM_CONDITIONAL(HAVE_GST_CHECK, test "x$HAVE_GST_CHECK" = "xyes")

dnl Check for documentation xrefs
GLIB_PREFIX="`$PKG_CONFIG --variable=prefix glib-2.0`"
GST_PREFIX="`$PKG_CONFIG --variable=prefix gstreamer-$GST_MAJORMINOR`"
GSTPB_PREFIX="`$PKG_CONFIG --variable=prefix gstreamer-plugins-base-$GST_MAJORMINOR`"
AC_SUBST(GLIB_PREFIX)
AC_SUBST(GST_PREFIX)
AC_SUBST(GSTPB_PREFIX)


dnl libpng is required
PKG_CHECK_MODULES(LIBPNG, libpng12 >= 1.0, HAVE_LIBPNG=yes, HAVE_LIBPNG=no)
if test "x${HAVE_LIBPNG}" != xyes ; then
  AC_ERROR([libpng12 is required])
fi
AC_SUBST(LIBPNG_LIBS)
AC_SUBST(LIBPNG_CFLAGS)


dnl GTK is optional and used in examples
HAVE_GTK=NO
PKG_CHECK_MODULES(GTK2, gtk+-2.0 >= 2.2.0, HAVE_GTK_22=yes, HAVE_GTK_22=no)
if test "x$HAVE_GTK_22" = "xyes"; then
  HAVE_GTK=yes
  GTK_VERSION=`$PKG_CONFIG --variable=gtk_binary_version gtk+-2.0`
  AC_SUBST(GTK_VERSION)
  GTK_PREFIX=`$PKG_CONFIG --variable=prefix gdk-pixbuf-2.0`
  AC_SUBST(GTK_BASE_DIR)
else
  PKG_CHECK_MODULES(GTK2, gtk+-2.0, HAVE_GTK_20=yes, HAVE_GTK_20=no)
fi
if test "x$HAVE_GTK_20" = "xyes"; then
  HAVE_GTK=yes
fi
GTK_CFLAGS=$GTK2_CFLAGS
GTK_LIBS=$GTK2_LIBS
AC_SUBST(GTK_LIBS)
AC_SUBST(GTK_CFLAGS)
AC_SUBST(HAVE_GTK)
AM_CONDITIONAL(HAVE_GTK, test "x$HAVE_GTK" = "xyes")

dnl optional gdk-pixbuf (for examples)
#HAVE_GDKPIXBUF=NO
#PKG_CHECK_MODULES(GDKPIXBUF, gdk-pixbuf-2.0, HAVE_GDKPIXBUF=yes, HAVE_GDKPIXBUF=no)
#AC_SUBST(GDKPIXBUF_LIBS)
#AC_SUBST(GDKPIXBUF_CFLAGS)
#AC_SUBST(HAVE_GDKPIXBUF)
#AM_CONDITIONAL(HAVE_GDKPIXBUF, test "x$HAVE_GDKPIXBUF" = "xyes")
#if test "x$HAVE_GDKPIXBUF" = "xyes"; then
#  AC_DEFINE_UNQUOTED([HAVE_GDKPIXBUF], [], ["Have GdkPixbuf"])
#fi


dnl clutter is optional and used in examples
HAVE_CLUTTER=NO
PKG_CHECK_MODULES(CLUTTER, clutter-0.8 >= 0.8.0, HAVE_CLUTTER=yes, HAVE_CLUTTER=no)
AC_SUBST(CLUTTER_LIBS)
AC_SUBST(CLUTTER_CFLAGS)
AC_SUBST(HAVE_CLUTTER)
AM_CONDITIONAL(HAVE_CLUTTER, test "x$HAVE_CLUTTER" = "xyes")

HAVE_CLUTTER_WITH_TEXTURE_RECTANGLE=NO
PKG_CHECK_MODULES(CLUTTER, clutter-0.8 >= 0.8.6, HAVE_CLUTTER_WITH_TEXTURE_RECTANGLE=yes, HAVE_CLUTTER_WITH_TEXTURE_RECTANGLE=no)
AC_SUBST(HAVE_CLUTTER_WITH_TEXTURE_RECTANGLE)
AM_CONDITIONAL(HAVE_CLUTTER_WITH_TEXTURE_RECTANGLE, test "x$HAVE_CLUTTER_WITH_TEXTURE_RECTANGLE" = "xyes")

HAVE_CLUTTER_GLX=NO
PKG_CHECK_MODULES(CLUTTER_GLX, clutter-glx-0.8 >= 0.8.0, HAVE_CLUTTER_GLX=yes, HAVE_CLUTTER_GLX=no)
AC_SUBST(CLUTTER_GLX_LIBS)
AC_SUBST(CLUTTER_GLX_CFLAGS)
AC_SUBST(HAVE_CLUTTER_GLX)
AM_CONDITIONAL(HAVE_CLUTTER_GLX, test "x$HAVE_CLUTTER_GLX" = "xyes")

HAVE_CLUTTER_X11=NO
PKG_CHECK_MODULES(CLUTTER_X11, clutter-x11-0.8 >= 0.8.0, HAVE_CLUTTER_X11=yes, HAVE_CLUTTER_X11=no)
AC_SUBST(CLUTTER_X11_LIBS)
AC_SUBST(CLUTTER_X11_CFLAGS)
AC_SUBST(HAVE_CLUTTER_X11)
AM_CONDITIONAL(HAVE_CLUTTER_X11, test "x$HAVE_CLUTTER_X11" = "xyes")


dnl *** set variables based on configure arguments ***

dnl set license and copyright notice
GST_LICENSE="LGPL"
AC_DEFINE_UNQUOTED(GST_LICENSE, "$GST_LICENSE", [GStreamer license])
AC_SUBST(GST_LICENSE)

dnl set location of plugin directory
AG_GST_SET_PLUGINDIR

dnl define an ERROR_CFLAGS Makefile variable
AG_GST_SET_ERROR_CFLAGS($GST_CVS)
AG_GST_SET_ERROR_CXXFLAGS($GST_CVS)

dnl define correct level for debugging messages
AG_GST_SET_LEVEL_DEFAULT($GST_CVS)

dnl used in examples
AG_GST_DEFAULT_ELEMENTS

dnl *** plugins to includes ***

AG_GST_CHECK_PLUGIN(gl)

dnl *** sys plug-ins ***

echo
AC_MSG_NOTICE([Checking libraries for plugins in sys/])
echo

dnl *** ext plug-ins ***
dnl keep this list sorted alphabetically !

if test "x$BUILD_EXTERNAL" = "xyes"; then

echo
AC_MSG_NOTICE([Checking libraries for plugins in ext/])
echo

else

dnl not building plugins with external dependencies,
dnl but we still need to set the conditionals
true

fi dnl of EXT plugins

dnl *** finalize CFLAGS, LDFLAGS, LIBS

dnl Overview:
dnl GST_OPTION_CFLAGS:  common flags for profiling, debugging, errors, ...
dnl GST_*:              flags shared by built objects to link against GStreamer
dnl GST_ALL_LDFLAGS:    linker flags shared by all
dnl GST_LIB_LDFLAGS:    additional linker flags for all libaries
dnl GST_LT_LDFLAGS:     library versioning of our libraries
dnl GST_PLUGIN_LDFLAGS: flags to be used for all plugins

dnl GST_OPTION_CFLAGS
if test "x$USE_DEBUG" = xyes; then
   PROFILE_CFLAGS="-g"
fi
AC_SUBST(PROFILE_CFLAGS)

if test "x$GST_CVS" = "xyes"; then
  DEPRECATED_CFLAGS="-DGST_DISABLE_DEPRECATED"
else
  DEPRECATED_CFLAGS=""
fi
AC_SUBST(DEPRECATED_CFLAGS)

dnl every flag in GST_OPTION_CFLAGS can be overridden at make time
GST_OPTION_CFLAGS="\$(ERROR_CFLAGS) \$(DEBUG_CFLAGS) \$(PROFILE_CFLAGS) \$(GCOV_CFLAGS) \$(OPT_CFLAGS) \$(DEPRECATED_CFLAGS)"
GST_OPTION_CXXFLAGS="\$(ERROR_CXXFLAGS) \$(DEBUG_CFLAGS) \$(PROFILE_CFLAGS) \$(GCOV_CFLAGS) \$(OPT_CFLAGS) \$(DEPRECATED_CFLAGS)"
AC_SUBST(GST_OPTION_CFLAGS)
AC_SUBST(GST_OPTION_CXXFLAGS)

dnl our libraries need to be versioned correctly
AC_SUBST(GST_LT_LDFLAGS)

dnl GST_PLUGINS_GL_CFLAGS
dnl prefer internal headers to already installed ones
dnl also add builddir include for enumtypes and marshal
GST_PLUGINS_GL_CFLAGS="-I\$(top_srcdir)/gst-libs -I\$(top_builddir)/gst-libs"
AC_SUBST(GST_PLUGINS_GL_CFLAGS)

dnl FIXME: do we want to rename to GST_ALL_* ?
dnl add GST_OPTION_CFLAGS, but overridable
GST_CXXFLAGS="-I\$(top_srcdir)/gst-libs -I\$(top_builddir)/gst-libs $GST_CFLAGS $GST_CXXFLAGS \$(GST_OPTION_CXXFLAGS)"
GST_CFLAGS="-I\$(top_srcdir)/gst-libs -I\$(top_builddir)/gst-libs $GST_CFLAGS \$(GST_OPTION_CFLAGS)"
AC_SUBST(GST_CFLAGS)
AC_SUBST(GST_CXXFLAGS)

dnl add GCOV libs because libtool strips -fprofile-arcs -ftest-coverage
GST_LIBS="$GST_LIBS \$(GCOV_LIBS)"
AC_SUBST(GST_LIBS)

dnl LDFLAGS really should only contain flags, not libs - they get added before
dnl whatevertarget_LIBS and -L flags here affect the rest of the linking
GST_ALL_LDFLAGS="-no-undefined"
AC_SUBST(GST_ALL_LDFLAGS)

dnl GST_LIB_LDFLAGS
dnl linker flags shared by all libraries
dnl LDFLAGS modifier defining exported symbols from built libraries
dnl (export _gst_foo but not __gst_foo)
GST_LIB_LDFLAGS="-export-symbols-regex ^_?\(gst_\|Gst\|GST_\).*"
AC_SUBST(GST_LIB_LDFLAGS)

dnl this really should only contain flags, not libs - they get added before
dnl whatevertarget_LIBS and -L flags here affect the rest of the linking
GST_PLUGIN_LDFLAGS="-module -avoid-version -export-symbols-regex '^[_]*gst_plugin_desc\$\$' $GST_ALL_LDFLAGS"
AC_SUBST(GST_PLUGIN_LDFLAGS)

dnl *** output files ***

AC_CONFIG_FILES(
Makefile
gst-plugins-gl.spec
gst/Makefile
gst/gl/Makefile
gst-libs/Makefile
gst-libs/gst/Makefile
gst-libs/gst/gl/Makefile
win32/common/config.h
pkgconfig/Makefile
pkgconfig/gstreamer-gl.pc
pkgconfig/gstreamer-gl-uninstalled.pc
tests/Makefile
tests/check/Makefile
tests/examples/Makefile
tests/examples/generic/Makefile
tests/examples/generic/cube/Makefile
tests/examples/generic/cubeyuv/Makefile
tests/examples/generic/doublecube/Makefile
tests/examples/clutter/Makefile
tests/examples/gtk/Makefile
tests/examples/gtk/gtkxoverlay/Makefile
tests/examples/gtk/filternoxoverlay/Makefile
tests/examples/gtk/filterxoverlay/Makefile
tests/examples/gtk/fxtest/Makefile
tests/examples/gtk/switchxoverlay/Makefile
tests/examples/qt/Makefile
tests/examples/wx/Makefile
tests/icles/Makefile
docs/Makefile
docs/libs/Makefile
docs/plugins/Makefile
docs/version.entities
po/Makefile.in
common/Makefile
common/m4/Makefile
m4/Makefile
)
AC_OUTPUT

AG_GST_OUTPUT_PLUGINS
