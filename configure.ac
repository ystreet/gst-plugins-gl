AC_PREREQ(2.60)

dnl please read gstreamer/docs/random/autotools before changing this file

dnl initialize autoconf
dnl releases only do -Wall, git and prerelease does -Werror too
dnl use a three digit version number for releases, and four for git/prerelease
AC_INIT(GStreamer OpenGL Plug-ins, 0.11.0.1,
    http://bugzilla.gnome.org/enter_bug.cgi?product=GStreamer,
    gst-plugins-gl)

AG_GST_INIT

dnl initialize automake
AM_INIT_AUTOMAKE([-Wno-portability 1.10 subdir-objects])

dnl define PACKAGE_VERSION_* variables
AS_VERSION

dnl check if this is a release version
AS_NANO(GST_GIT="no", GST_GIT="yes")

dnl libtoolize complains about wrong macros
AC_CONFIG_MACRO_DIR([m4])

dnl can autoconf find the source ?
AC_CONFIG_SRCDIR([gst-libs/gst/gl/gstgldisplay.c])

dnl define the output header for config
AC_CONFIG_HEADERS([config.h])

dnl AM_MAINTAINER_MODE only provides the option to configure to enable it
AM_MAINTAINER_MODE

dnl sets host_* variables
AC_CANONICAL_HOST

dnl use pretty build output with automake >= 1.11
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])],
  [AM_DEFAULT_VERBOSITY=1
   AC_SUBST(AM_DEFAULT_VERBOSITY)])

dnl our libraries and install dirs use GST_API_VERSION in the filename
dnl to allow side-by-side installation of different API versions
GST_API_VERSION=1.0
AC_SUBST(GST_API_VERSION)
AC_DEFINE_UNQUOTED(GST_API_VERSION, "$GST_API_VERSION",
  [GStreamer API Version])

dnl CURRENT, REVISION, AGE
dnl - library source changed -> increment REVISION
dnl - interfaces added/removed/changed -> increment CURRENT, REVISION = 0
dnl - interfaces added -> increment AGE
dnl - interfaces removed -> AGE = 0
dnl sets GST_LT_LDFLAGS
AS_LIBTOOL(GST, 1, 0, 0)

dnl FIXME: this macro doesn't actually work;
dnl the generated libtool script has no support for the listed tags.
dnl So this needs to be fixed first if we want to use this
dnl AS_LIBTOOL_TAGS

AC_LIBTOOL_WIN32_DLL
AM_PROG_LIBTOOL

dnl *** required versions of GStreamer stuff ***
GST_REQ=1.0.0
GSTPB_REQ=1.0.0

dnl *** autotools stuff ****

dnl allow for different autotools
AS_AUTOTOOLS_ALTERNATE

dnl Add parameters for aclocal
AC_SUBST(ACLOCAL_AMFLAGS, "-I m4 -I common/m4")

dnl set up gettext
dnl the version check needs to stay here because autopoint greps for it
AM_GNU_GETTEXT_VERSION([0.17])
AM_GNU_GETTEXT([external])
AG_GST_GETTEXT([gst-plugins-gl-$GST_API_VERSION])

dnl *** check for arguments to configure ***

AG_GST_ARG_DISABLE_FATAL_WARNINGS

AG_GST_ARG_DEBUG
AG_GST_ARG_PROFILING
AG_GST_ARG_VALGRIND
AG_GST_ARG_GCOV

AG_GST_ARG_EXAMPLES

AG_GST_ARG_WITH_PKG_CONFIG_PATH
AG_GST_ARG_WITH_PACKAGE_NAME
AG_GST_ARG_WITH_PACKAGE_ORIGIN
AG_GST_ARG_WITH_PLUGINS

AG_GST_ARG_ENABLE_EXTERNAL

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
AC_PROG_CC_STDC
AC_PROG_CXX
AC_PROG_OBJC

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

dnl build static plugins or not
AC_MSG_CHECKING([whether to build static plugins or not])
AC_ARG_ENABLE(
  static-plugins,
  AC_HELP_STRING(
    [--enable-static-plugins],
    [build static plugins @<:@default=no@:>@]),
  [AS_CASE(
    [$enableval], [no], [], [yes], [],
    [AC_MSG_ERROR([bad value "$enableval" for --enable-static-plugins])])],
  [enable_static_plugins=no])
AC_MSG_RESULT([$enable_static_plugins])
if test "x$enable_static_plugins" = xyes; then
  AC_DEFINE(GST_PLUGIN_BUILD_STATIC, 1,
    [Define if static plugins should be built])
  GST_PLUGIN_LIBTOOLFLAGS=""
else
  GST_PLUGIN_LIBTOOLFLAGS="--tag=disable-static"
fi
AC_SUBST(GST_PLUGIN_LIBTOOLFLAGS)
AM_CONDITIONAL(GST_PLUGIN_BUILD_STATIC, test "x$enable_static_plugins" = "xyes")

dnl *** checks for dependency libraries ***

dnl GLib is required
AG_GST_GLIB_CHECK([2.22])
AG_GST_PKG_CHECK_MODULES(GMODULE, gmodule-no-export-2.0 >= 2.22)

AC_ARG_ENABLE([opengl],
     [  --enable-opengl         Enable Desktop OpenGL support @<:@default=auto@:>@],
     [case "${enableval}" in
       yes)  NEED_GL=yes ;;
       no)   NEED_GL=no ;;
       auto) NEED_GL=auto ;;
       *) AC_MSG_ERROR([bad value ${enableval} for --enable-opengl]) ;;
     esac],[NEED_GL=auto])

AC_ARG_ENABLE([gles2],
     [  --enable-gles2          Enable OpenGL|ES 2.0 support @<:@default=auto@:>@],
     [case "${enableval}" in
       yes)  NEED_GLES2=yes ;;
       no)   NEED_GLES2=no ;;
       auto) NEED_GLES2=auto ;;
       *) AC_MSG_ERROR([bad value ${enableval} for --enable-gles2]) ;;
     esac],[NEED_GLES2=auto])

AC_ARG_ENABLE([egl],
     [  --enable-egl            Enable EGL support @<:@default=auto@:>@],
     [case "${enableval}" in
       yes)  NEED_EGL=yes ;;
       no)   NEED_EGL=no ;;
       auto) NEED_EGL=auto ;;
       *) AC_MSG_ERROR([bad value ${enableval} for --enable-egl]) ;;
     esac],[NEED_EGL=auto])

AC_ARG_ENABLE([wgl],
     [  --enable-wgl            Enable WGL support @<:@default=auto@:>@],
     [case "${enableval}" in
       yes)  NEED_WGL=yes ;;
       no)   NEED_WGL=no ;;
       auto) NEED_WGL=auto ;;
       *) AC_MSG_ERROR([bad value ${enableval} for --enable-wgl]) ;;
     esac],[NEED_WGL=auto])

AC_ARG_ENABLE([glx],
     [  --enable-glx            Enable GLX support @<:@default=auto@:>@],
     [case "${enableval}" in
       yes)  NEED_GLX=yes ;;
       no)   NEED_GLX=no ;;
       auto) NEED_GLX=auto ;;
       *) AC_MSG_ERROR([bad value ${enableval} for --enable-glx]) ;;
     esac],[NEED_GLX=auto])

AC_ARG_ENABLE([cocoa],
     [  --enable-cocoa          Enable Cocoa support (using GNUstep on non OS X platforms) @<:@default=auto@:>@],
     [case "${enableval}" in
       yes)  NEED_COCOA=yes ;;
       no)   NEED_COCOA=no ;;
       auto) NEED_COCOA=auto ;;
       *) AC_MSG_ERROR([bad value ${enableval} for --enable-cocoa]) ;;
     esac],[NEED_COCOA=auto])

AC_ARG_ENABLE([x11],
     [  --enable-x11            Enable x11 support @<:@default=auto@:>@],
     [case "${enableval}" in
       yes)  NEED_X11=yes ;;
       no)   NEED_X11=no ;;
       auto) NEED_X11=auto ;;
       *) AC_MSG_ERROR([bad value ${enableval} for --enable-x11]) ;;
     esac],[NEED_X11=auto])

AC_ARG_ENABLE([wayland],
     [  --enable-wayland        Enable Wayland support (requires EGL) @<:@default=auto@:>@],
     [case "${enableval}" in
       yes)  NEED_WAYLAND_EGL=yes ;;
       no)   NEED_WAYLAND_EGL=no ;;
       auto) NEED_WAYLAND_EGL=auto ;;
       *) AC_MSG_ERROR([bad value ${enableval} for --enable-wayland]) ;;
     esac],[NEED_WAYLAND_EGL=auto])

AG_GST_CHECK_X
save_CPPFLAGS="$CPPFLAGS"
save_LIBS="$LIBS"

HAVE_GL=no
HAVE_GLES2=no
HAVE_EGL=no
HAVE_GLU=no
HAVE_GNUSTEP_COCOA=no
HAVE_WAYLAND_EGL=no

case $host in
  *-mingw32* )
    LIBS="$LIBS -lgdi32"
    AG_GST_CHECK_LIBHEADER(GL, opengl32, glTexImage2D,, GL/gl.h)
    AG_GST_CHECK_LIBHEADER(GLU, glu32, gluSphere,, GL/glu.h)
    HAVE_GL=yes
    HAVE_GLU=yes
  ;;
  *)
    AG_GST_CHECK_LIBHEADER(GL, GL, glTexImage2D,, GL/gl.h)
    AG_GST_CHECK_LIBHEADER(GLU, GLU, gluSphere,, GL/glu.h)
    AG_GST_CHECK_LIBHEADER(EGL, EGL, eglGetError,, EGL/egl.h)
    AG_GST_CHECK_LIBHEADER(GLES2, GLESv2, glTexImage2D,, GLES2/gl2.h)
    PKG_CHECK_MODULES(WAYLAND_EGL, wayland-client >= 1.0 wayland-cursor >= 1.0 wayland-egl >= 9.0, HAVE_WAYLAND_EGL=yes, HAVE_WAYLAND_EGL=no)
  ;;
esac

case $host in
  *-darwin* ) ;;
  *)
    AC_PATH_PROG([GNUSTEP_CONFIG],[gnustep-config])
    if test "x$GNUSTEP_CONFIG" != "x"; then
      GNUSTEP_HOST="`$GNUSTEP_CONFIG --variable=GNUSTEP_HOST`"
      case $host in
        *-mingw* )
          case $GNUSTEP_HOST in
            *-mingw* )
              HAVE_GNUSTEP_COCOA=yes
            ;;
          esac
          ;;
        * )
          HAVE_GNUSTEP_COCOA=yes
        ;;
      esac
    fi
  ;;
esac

AC_MSG_CHECKING([for GNUstep])
if test "x$HAVE_GNUSTEP_COCOA" = "xyes" ; then
  AC_MSG_RESULT([yes])
else
  AC_MSG_RESULT([no])
fi

CPPFLAGS="$save_CPPFLAGS"
LIBS="$save_LIBS"

AC_SUBST(HAVE_GL)
AC_SUBST(HAVE_GLU)
AC_SUBST(HAVE_EGL)
AC_SUBST(HAVE_GLES2)

USE_OPENGL=no
USE_GLES2=no
USE_EGL=no
USE_GLX=no
USE_COCOA=no
USE_WGL=no
USE_X11=no
GL_LIBS=
GL_CFLAGS=
GL_OBJCFLAGS=

dnl Check for what the user asked for and what we could find
if test "x$HAVE_EGL" = "xno"; then
  if test "x$NEED_EGL" = "xyes"; then
    AC_MSG_ERROR([Could not find the required EGL libraries])
  fi
fi

if test "x$HAVE_GL" = "xno"; then
  if test "x$NEED_GL" = "xyes"; then
    AC_MSG_ERROR([Could not find the required OpenGL libraries])
  fi
fi

if test "x$NEED_GLES2" = "xno"; then
  if test "x$NEED_GL" = "xno"; then
    AC_MSG_ERROR([Either OpenGL or OpenGL|ES is required])
  fi
fi

if test "x$HAVE_GLES2" = "xno"; then
  if test "x$NEED_GLES2" = "xyes"; then
    AC_MSG_ERROR([Could not find the required OpenGL|ES 2.0 libraries])
  fi
fi

dnl X, GLX and OpenGL
if test "x$HAVE_X" = "xno"; then
  if test "x$NEED_GLX" = "xyes"; then
    AC_MSG_ERROR([Building the GLX backend without X11 is unsupported])
  fi
  if test "x$NEED_X11" = "xyes"; then
    AC_MSG_ERROR([Could not find X11 development libraries])
  fi
else
  if test "x$NEED_GL" != "xno"; then
    if test "x$HAVE_GL" = "xno"; then
      if test "x$NEED_GLX" = "xyes"; then
        AC_MSG_ERROR([Building the GLX backend without the OpenGL backend is unsupported])
      fi
    else dnl HAVE_GL=yes
      if test "x$HAVE_GLU" = "xno"; then
        if test "x$NEED_GL" = "xyes"; then
          AC_MSG_ERROR([GLU is required with OpenGL support])
        else
          AC_MSG_WARN([GLU is required with OpenGL support])
        fi
      else dnl HAVE_GLU=yes
        USE_OPENGL=yes
        if test "x$NEED_GLX" != "xno"; then
          USE_GLX=yes
        fi
      fi
    fi
  fi
fi

#dnl Check for OpenGL, GLU
echo host is $host
case $host in
  *-android*)
    if test "x$NEED_WGL" = "xyes"; then
      AC_MSG_ERROR([WGL is not available on Android])
    fi
    if test "x$NEED_GLX" = "xyes"; then
      AC_MSG_ERROR([GLX is not available on Android])
    fi
    if test "x$NEED_GL" = "xyes"; then
      AC_MSG_ERROR([GL is not available on Android])
    fi
    if test "x$NEED_X11" = "xyes"; then
      AC_MSG_ERROR([X11 is not available on Android])
    fi
    if test "x$NEED_COCOA" = "xyes"; then
      AC_MSG_ERROR([Cocoa is not available on Android])
    fi

    dnl OpenGL|ES 2.0
    if test "x$HAVE_GLES2" = "xyes"; then
      if test "x$NEED_GLES2" != "xno"; then
        GL_LIBS="$GL_LIBS -lGLESv2"
        USE_GLES2=yes
      fi
    fi

    dnl EGL
    if test "x$HAVE_EGL" = "xyes"; then
      if test "x$NEED_EGL" != "xno"; then
        GL_LIBS="$GL_LIBS -lEGL"
        USE_EGL=yes
      fi
    fi

    if test "x$USE_EGL" != "xyes"; then
      AC_MSG_ERROR([Need EGL on Android])
    fi

    if test "x$USE_GLES2" != "xyes"; then
      AC_MSG_ERROR([Need OpenGL|ES 2.0 on Android])
    fi

    HAVE_WINDOW_ANDROID=yes
    ;;
  *-linux* | *-cygwin* | *-solaris* | *-netbsd* | *-freebsd* | *-openbsd* | *-kfreebsd* | *-dragonflybsd* | *-gnu* )
    if test "x$NEED_WGL" = "xyes"; then
      AC_MSG_ERROR([WGL is not available on unix])
    fi

    if test "x$HAVE_X" = "xno"; then
      if test "x$HAVE_WAYLAND_EGL" = "xno"; then
        AC_MSG_ERROR([X or Wayland is required])
      fi
    fi

    dnl check Desktop OpenGL
    if test "x$HAVE_GL" = "xyes"; then
      if test "x$NEED_GL" != "xno"; then
        GL_LIBS="$GL_LIBS -lGL -lGLU"
      fi
    fi

    dnl OpenGL|ES 2.0
    if test "x$HAVE_GLES2" = "xyes"; then
      if test "x$NEED_GLES2" != "xno"; then
        GL_LIBS="$GL_LIBS -lGLESv2"
        USE_GLES2=yes
      fi
    fi

    if test "x$HAVE_X" = "xyes"; then
      if test "x$NEED_X11" != "xno"; then
        GL_LIBS="$GL_LIBS $X_LIBS"
        GL_CFLAGS="$GL_CFLAGS $X_CFLAGS"
        HAVE_WINDOW_X11=yes
      fi
    fi

    if test "x$HAVE_WAYLAND_EGL" = "xyes"; then
      if test "x$NEED_EGL" = "xno" -o "x$HAVE_EGL" = "xno"; then
        AC_MSG_WARN([EGL is required by the Wayland backend])
      else
        if test "x$NEED_WAYLAND_EGL" != "xno"; then
          HAVE_WINDOW_WAYLAND=yes
          GL_LIBS="$GL_LIBS $WAYLAND_EGL_LIBS"
          GL_CFLAGS="$GL_CFLAGS $WAYLAND_EGL_CFLAGS"
        fi
      fi
    fi

    dnl EGL
    if test "x$HAVE_EGL" = "xno"; then
      if test "x$HAVE_GL" = "xno"; then
        AC_MSG_ERROR([Building requires either EGL or GLX])
      fi
    else
      if test "x$NEED_EGL" != "xno"; then
        if test "x$HAVE_WINDOW_WAYLAND" = "xyes" -o "x$HAVE_WINDOW_X11" = "xyes"; then
          GL_LIBS="$GL_LIBS -lEGL"
          USE_EGL=yes
        fi
      fi
    fi

    dnl GNUstep provides the Cocoa API on unix
    if test "x$NEED_COCOA" != "xno"; then
      if test "x$HAVE_GNUSTEP_COCOA" = "xyes" ; then
        GNUSTEP_OBJCFLAGS="`$GNUSTEP_CONFIG --objc-flags`"
        GNUSTEP_LIBS="`$GNUSTEP_CONFIG --gui-libs`"
        GL_LIBS="$GL_LIBS $GNUSTEP_LIBS -lgnustep-gui -lgnustep-base"
        GL_OBJCFLAGS="$GL_OBJCFLAGS $GNUSTEP_OBJCFLAGS"

        USE_COCOA=yes
        HAVE_WINDOW_COCOA=yes
      fi
    fi
    ;;
  *-darwin*)
    if test "x$NEED_WGL" = "xyes"; then
      AC_MSG_ERROR([WGL is not available on Mac OS X])
    fi
    if test "x$NEED_EGL" = "xyes"; then
      AC_MSG_ERROR([EGL is not available on Mac OS X])
    fi
    if test "x$NEED_GLES2" = "xyes"; then
      AC_MSG_ERROR([OpenGL|ES 2.0 is not supported on your platform yet])
    fi

    GL_LIBS="$LIBS -framework OpenGL -framework Cocoa"
    GL_CFLAGS="$GL_CFLAGS -framework OpenGL"
    USE_COCOA=yes
    HAVE_WINDOW_COCOA=yes
    USE_OPENGL=yes
    ;;
  *-mingw32*)
    if test "x$NEED_GLX" = "xyes"; then
      AC_MSG_ERROR([GLX is not available on Windows])
    fi
    if test "x$NEED_GLES2" = "xyes"; then
      AC_MSG_ERROR([OpenGL|ES 2.0 is not supported on your platform yet])
    fi

    GL_LIBS="$GL_LIBS -lgdi32 -lopengl32 -lglu32"
    HAVE_WINDOW_WIN32=yes
    USE_OPENGL=yes
    USE_WGL=yes

    dnl GNUstep provides the Cocoa API on win32
    if test "x$HAVE_GNUSTEP_COCOA" = "xyes" ; then
      GNUSTEP_CFLAGS="`$GNUSTEP_CONFIG --objc-flags`"
      GNUSTEP_LIBS="`$GNUSTEP_CONFIG --gui-libs`"
      GL_LIBS="$GL_LIBS $GNUSTEP_LIBS -lgnustep-gui -lgnustep-base"
      GL_OBJCFLAGS="$OBJCFLAGS $GNUSTEP_CFLAGS"

      USE_COCOA=yes
      HAVE_WINDOW_COCOA=yes
    fi
    ;;
  *)
    AC_MSG_ERROR([Don't know how to check for OpenGL on your platform.])
    exit 1
    ;;
esac

GL_PLATFORMS=
GL_WINDOWS=
GL_APIS=
GL_CONFIG_DEFINES=

dnl API's
if test "x$USE_OPENGL" = "xyes"; then
  GL_APIS="gl $GL_APIS"
  GL_CONFIG_DEFINES="$GL_CONFIG_DEFINES
#define GST_GL_HAVE_OPENGL 1"
fi
if test "x$USE_GLES2" = "xyes"; then
  GL_APIS="gles2 $GL_APIS"
  GL_CONFIG_DEFINES="$GL_CONFIG_DEFINES
#define GST_GL_HAVE_GLES2 1"
fi

dnl WINDOW's
if test "x$HAVE_WINDOW_X11" = "xyes"; then
  GL_WINDOWS="x11 $GL_WINDOWS"
  GL_CONFIG_DEFINES="$GL_CONFIG_DEFINES
#define GST_GL_HAVE_WINDOW_X11 1"
fi
if test "x$HAVE_WINDOW_COCOA" = "xyes"; then
  GL_WINDOWS="cocoa $GL_WINDOWS"
  GL_CONFIG_DEFINES="$GL_CONFIG_DEFINES
#define GST_GL_HAVE_WINDOW_COCOA 1"
fi
if test "x$HAVE_WINDOW_WIN32" = "xyes"; then
  GL_WINDOWS="win32 $GL_WINDOWS"
  GL_CONFIG_DEFINES="$GL_CONFIG_DEFINES
#define GST_GL_HAVE_WINDOW_WIN32 1"
fi
if test "x$HAVE_WINDOW_WAYLAND" = "xyes"; then
  GL_WINDOWS="wayland $GL_WINDOWS"
  GL_CONFIG_DEFINES="$GL_CONFIG_DEFINES
#define GST_GL_HAVE_WINDOW_WAYLAND 1"
fi
if test "x$HAVE_WINDOW_ANDROID" = "xyes"; then
  GL_WINDOWS="android $GL_WINDOWS"
  GL_CONFIG_DEFINES="$GL_CONFIG_DEFINES
#define GST_GL_HAVE_WINDOW_ANDROID 1"
fi

dnl PLATFORM's
if test "x$USE_EGL" = "xyes"; then
  GL_PLATFORMS="egl $GL_PLATFORMS"
  GL_CONFIG_DEFINES="$GL_CONFIG_DEFINES
#define GST_GL_HAVE_PLATFORM_EGL 1"
fi
if test "x$USE_GLX" = "xyes"; then
  GL_PLATFORMS="glx $GL_PLATFORMS"
  GL_CONFIG_DEFINES="$GL_CONFIG_DEFINES
#define GST_GL_HAVE_PLATFORM_GLX 1"
fi
if test "x$USE_WGL" = "xyes"; then
  GL_PLATFORMS="wgl $GL_PLATFORMS"
  GL_CONFIG_DEFINES="$GL_CONFIG_DEFINES
#define GST_GL_HAVE_PLATFORM_WGL 1"
fi
if test "x$USE_COCOA" = "xyes"; then
  GL_PLATFORMS="cocoa $GL_PLATFORMS"
  GL_CONFIG_DEFINES="$GL_CONFIG_DEFINES
#define GST_GL_HAVE_PLATFORM_COCOA 1"
fi

dnl Check for no platforms/window systems
if test "x$GL_APIS" = "x"; then
  AC_MSG_ERROR([Either OpenGL or OpenGL|ES is required])
fi
if test "x$GL_PLATFORMS" = "x"; then
  AC_MSG_ERROR([Could not find any OpenGL platforms to use such as Cocoa, WGL or GLX])
fi
if test "x$GL_WINDOWS" = "x"; then
  AC_MSG_ERROR([Could not find any window systems to use such as Cocoa, Win32API or X11])
fi

AC_SUBST(GL_LIBS)
AC_SUBST(GL_CFLAGS)
AC_SUBST(GL_OBJCFLAGS)

AM_CONDITIONAL(HAVE_WINDOW_X11, test "x$HAVE_WINDOW_X11" = "xyes")
AM_CONDITIONAL(HAVE_WINDOW_X11ES2, test "x$HAVE_WINDOW_X11ES2" = "xyes")
AM_CONDITIONAL(HAVE_WINDOW_COCOA, test "x$HAVE_WINDOW_COCOA" = "xyes")
AM_CONDITIONAL(HAVE_WINDOW_WIN32, test "x$HAVE_WINDOW_WIN32" = "xyes")
AM_CONDITIONAL(HAVE_WINDOW_WAYLAND, test "x$HAVE_WINDOW_WAYLAND" = "xyes")
AM_CONDITIONAL(HAVE_WINDOW_ANDROID, test "x$HAVE_WINDOW_ANDROID" = "xyes")

AM_CONDITIONAL(USE_OPENGL, test "x$USE_OPENGL" = "xyes")
AM_CONDITIONAL(USE_GLES2, test "x$USE_GLES2" = "xyes")
AM_CONDITIONAL(USE_EGL, test "x$USE_EGL" = "xyes")
AM_CONDITIONAL(USE_GLX, test "x$USE_GLX" = "xyes")
AM_CONDITIONAL(USE_WGL, test "x$USE_WGL" = "xyes")
AM_CONDITIONAL(USE_COCOA, test "x$USE_COCOA" = "xyes")

dnl Check for some types that are not always present
GL_INCLUDES=""
if test "x$USE_GLES2" = "xyes"; then
  GL_INCLUDES="$GL_INCLUDES
# include <GLES2/gl2.h>
# include <GLES2/gl2ext.h>
"
fi

if test "x$USE_OPENGL" = "xyes"; then
  GL_INCLUDES="$GL_INCLUDES
# if __APPLE__
#  include <OpenGL/OpenGL.h>
#  include <OpenGL/gl.h>
# else
#  include <GL/glu.h>
#  include <GL/gl.h>
#  if __WIN32__ || _WIN32
#   include <GL/glext.h>
#  endif
# endif
"
fi

AC_CHECK_TYPES(GLeglImageOES, [], [], [[$GL_INCLUDES]])
if test "x$ac_cv_type_GLeglImageOES" = "xyes"; then
  GL_CONFIG_DEFINES="$GL_CONFIG_DEFINES
#define GST_GL_HAVE_GLEGLIMAGEOES 1"
fi

AC_CHECK_TYPES(GLchar, [], [], [[$GL_INCLUDES]])
if test "x$ac_cv_type_GLchar" = "xyes"; then
  GL_CONFIG_DEFINES="$GL_CONFIG_DEFINES
#define GST_GL_HAVE_GLCHAR 1"
fi

AC_CHECK_TYPES(GLsizeiptr, [], [], [[$GL_INCLUDES]])
if test "x$ac_cv_type_GLsizeiptr" = "xyes"; then
  GL_CONFIG_DEFINES="$GL_CONFIG_DEFINES
#define GST_GL_HAVE_GLSIZEIPTR 1"
fi

AC_CHECK_TYPES(GLintptr, [], [], [[$GL_INCLUDES]])
if test "x$ac_cv_type_GLintptr" = "xyes"; then
  GL_CONFIG_DEFINES="$GL_CONFIG_DEFINES
#define GST_GL_HAVE_GLINTPTR 1"
fi

AC_CONFIG_COMMANDS([gst-libs/gst/gl/gstglconfig.h], [
	outfile=gstglconfig.h-tmp
	cat > $outfile <<\_______EOF
/* gstglconfig.h
 *
 * This is a generated file.  Please modify `configure.ac'
 */

#ifndef __GST_GL_CONFIG_H__
#define __GST_GL_CONFIG_H__

#include <gst/gst.h>

G_BEGIN_DECLS

_______EOF

	cat >>$outfile <<_______EOF
$gl_config_defines
_______EOF

	cat >>$outfile <<_______EOF

G_END_DECLS

#endif  /* __GST_GL_CONFIG_H__ */
_______EOF


	if cmp -s $outfile gst-libs/gst/gl/gstglconfig.h; then
          AC_MSG_NOTICE([gst-libs/gst/gl/gstglconfig.h is unchanged])
	  rm -f $outfile
	else
	  mv $outfile gst-libs/gst/gl/gstglconfig.h
	fi
],[
gl_config_defines='$GL_CONFIG_DEFINES'
])

dnl checks for gstreamer
dnl uninstalled is selected preferentially -- see pkg-config(1)
AG_GST_CHECK_GST($GST_API_VERSION, [$GST_REQ], yes)
AG_GST_CHECK_GST_BASE($GST_API_VERSION, [$GST_REQ], yes)
AG_GST_CHECK_GST_CONTROLLER($GST_API_VERSION, [$GST_REQ], yes)
AG_GST_CHECK_GST_CHECK($GST_API_VERSION, [$GST_REQ], no)
AG_GST_CHECK_GST_PLUGINS_BASE($GST_API_VERSION, [$GSTPB_REQ], yes)
AM_CONDITIONAL(HAVE_GST_CHECK, test "x$HAVE_GST_CHECK" = "xyes")

dnl Check for documentation xrefs
GLIB_PREFIX="`$PKG_CONFIG --variable=prefix glib-2.0`"
GST_PREFIX="`$PKG_CONFIG --variable=prefix gstreamer-$GST_API_VERSION`"
GSTPB_PREFIX="`$PKG_CONFIG --variable=prefix gstreamer-plugins-base-$GST_API_VERSION`"
AC_SUBST(GLIB_PREFIX)
AC_SUBST(GST_PREFIX)
AC_SUBST(GSTPB_PREFIX)


dnl libpng is optional
PKG_CHECK_MODULES(LIBPNG, libpng >= 1.0, HAVE_PNG=yes, HAVE_PNG=no)
if test "x$HAVE_PNG" = "xyes"; then
  AC_DEFINE(HAVE_PNG, [1] , [Use libpng])
fi
AC_SUBST(HAVE_PNG)
AC_SUBST(LIBPNG_LIBS)
AC_SUBST(LIBPNG_CFLAGS)
AM_CONDITIONAL(HAVE_PNG, test "x$HAVE_PNG" = "xyes")

dnl libjpeg is optional
AC_ARG_WITH(jpeg-mmx, [  --with-jpeg-mmx, path to MMX'ified JPEG library])
OLD_LIBS="$LIBS"
if test x$with_jpeg_mmx != x; then
  LIBS="$LIBS -L$with_jpeg_mmx"
fi
AC_CHECK_LIB(jpeg-mmx, jpeg_set_defaults, HAVE_JPEG="yes", HAVE_JPEG="no")
JPEG_LIBS="$LIBS -ljpeg-mmx"
LIBS="$OLD_LIBS"
if test x$HAVE_JPEG != xyes; then
  AC_CHECK_LIB(jpeg, jpeg_set_defaults, HAVE_JPEG="yes", HAVE_JPEG="no")
  JPEG_LIBS="-ljpeg"
  AC_DEFINE(HAVE_JPEG, [1] , [Use libjpeg])
fi
AC_SUBST(HAVE_JPEG)
AC_SUBST(JPEG_LIBS)
AM_CONDITIONAL(HAVE_JPEG, test "x$HAVE_JPEG" = "xyes")


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
PKG_CHECK_MODULES(CLUTTER, clutter-1.0 >= 1.8, HAVE_CLUTTER=yes, HAVE_CLUTTER=no)
AC_SUBST(CLUTTER_LIBS)
AC_SUBST(CLUTTER_CFLAGS)
AC_SUBST(HAVE_CLUTTER)
AM_CONDITIONAL(HAVE_CLUTTER, test "x$HAVE_CLUTTER" = "xyes")

HAVE_CLUTTER_WITH_TEXTURE_RECTANGLE=NO
PKG_CHECK_MODULES(CLUTTER_WITH_TEXTURE_RECTANGLE, clutter-1.0 >= 1.8, HAVE_CLUTTER_WITH_TEXTURE_RECTANGLE=yes, HAVE_CLUTTER_WITH_TEXTURE_RECTANGLE=no)
AC_SUBST(HAVE_CLUTTER_WITH_TEXTURE_RECTANGLE)
AM_CONDITIONAL(HAVE_CLUTTER_WITH_TEXTURE_RECTANGLE, test "x$HAVE_CLUTTER_WITH_TEXTURE_RECTANGLE" = "xyes")

HAVE_CLUTTER_GLX=NO
PKG_CHECK_MODULES(CLUTTER_GLX, clutter-glx-1.0 >= 1.8, HAVE_CLUTTER_GLX=yes, HAVE_CLUTTER_GLX=no)
AC_SUBST(CLUTTER_GLX_LIBS)
AC_SUBST(CLUTTER_GLX_CFLAGS)
AC_SUBST(HAVE_CLUTTER_GLX)
AM_CONDITIONAL(HAVE_CLUTTER_GLX, test "x$HAVE_CLUTTER_GLX" = "xyes")

HAVE_CLUTTER_X11=NO
PKG_CHECK_MODULES(CLUTTER_X11, clutter-x11-1.0 >= 1.8, HAVE_CLUTTER_X11=yes, HAVE_CLUTTER_X11=no)
AC_SUBST(CLUTTER_X11_LIBS)
AC_SUBST(CLUTTER_X11_CFLAGS)
AC_SUBST(HAVE_CLUTTER_X11)
AM_CONDITIONAL(HAVE_CLUTTER_X11, test "x$HAVE_CLUTTER_X11" = "xyes")

dnl used in the clutteractor example
PKG_CHECK_MODULES(XCOMPOSITE, xcomposite, HAVE_XCOMPOSITE=yes, HAVE_XCOMPOSITE=no)
AC_SUBST(XCOMPOSITE_CFLAGS)
AC_SUBST(XCOMPOSITE_LIBS)
AM_CONDITIONAL(HAVE_XCOMPOSITE, test "x$HAVE_XCOMPOSITE" = "xyes")

dnl sdl is optional and used in examples
HAVE_SDL=NO
PKG_CHECK_MODULES(SDL, sdl >= 1.2.0, HAVE_SDL=yes, HAVE_SDL=no)
AC_SUBST(SDL_LIBS)
AC_SUBST(SDL_CFLAGS)
AC_SUBST(SDL_CLUTTER)
AM_CONDITIONAL(HAVE_SDL, test "x$HAVE_SDL" = "xyes")

dnl projectM is optional and used in libvisual-gl
HAVE_PROJECTM_TAKING_CARE_OF_EXTERNAL_FBO=NO
PKG_CHECK_MODULES(PROJECTM, libprojectM >= 2.0.1, HAVE_PROJECTM_TAKING_CARE_OF_EXTERNAL_FBO=yes, HAVE_PROJECTM_TAKING_CARE_OF_EXTERNAL_FBO=no)
AC_SUBST(HAVE_PROJECTM_TAKING_CARE_OF_EXTERNAL_FBO)
AM_CONDITIONAL(HAVE_PROJECTM_TAKING_CARE_OF_EXTERNAL_FBO, test "x$HAVE_PROJECTM_TAKING_CARE_OF_EXTERNAL_FBO" = "xyes")
if test "x$HAVE_PROJECTM_TAKING_CARE_OF_EXTERNAL_FBO" = "xyes"; then
  AC_DEFINE(HAVE_PROJECTM_TAKING_CARE_OF_EXTERNAL_FBO, [1] , [Full compatible with projectM])
else
  AC_DEFINE(HAVE_PROJECTM_TAKING_CARE_OF_EXTERNAL_FBO, [0] , [Minimum comptible with projectM])
fi

dnl *** set variables based on configure arguments ***

dnl set license and copyright notice
GST_LICENSE="LGPL"
AC_DEFINE_UNQUOTED(GST_LICENSE, "$GST_LICENSE", [GStreamer license])
AC_SUBST(GST_LICENSE)

dnl set location of plugin directory
AG_GST_SET_PLUGINDIR

dnl set release date/time
AG_GST_SET_PACKAGE_RELEASE_DATETIME_WITH_NANO([$PACKAGE_VERSION_NANO],
  ["${srcdir}/gst-plugins-gl.doap"],
  [$PACKAGE_VERSION_MAJOR.$PACKAGE_VERSION_MINOR.$PACKAGE_VERSION_MICRO])

dnl define an ERROR_CFLAGS Makefile variable
AG_GST_SET_ERROR_CFLAGS($FATAL_WARNINGS)
AG_GST_SET_ERROR_CXXFLAGS($FATAL_WARNINGS)
dnl define an ERROR_OBJCFLAGS Makefile variable
AG_GST_SET_ERROR_OBJCFLAGS($FATAL_WARNINGS, [
    -Wmissing-declarations -Wmissing-prototypes -Wredundant-decls
    -Wwrite-strings -Wold-style-definition
    -Winit-self -Wmissing-include-dirs -Wno-multichar
    -Wnested-externs $NO_WARNINGS])

dnl define correct level for debugging messages
AG_GST_SET_LEVEL_DEFAULT($GST_GIT)

dnl used in examples
AG_GST_DEFAULT_ELEMENTS

dnl *** plugins to includes ***
GST_PLUGINS_NONPORTED="libvisual "
AC_SUBST(GST_PLUGINS_NONPORTED)
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

dnl *** libvisual ***
translit(dnm, m, l) AM_CONDITIONAL(USE_LIBVISUAL, true)
AG_GST_CHECK_FEATURE(LIBVISUAL, [libvisual visualization library], libvisual, [
  AG_GST_PKG_CHECK_MODULES(LIBVISUAL, libvisual-0.4 >= 0.4.0)
  if test x$HAVE_LIBVISUAL = xyes; then
    LIBVIS_PLUGINSDIR="`$PKG_CONFIG --variable=pluginsbasedir libvisual-0.4`"
  fi
  AC_MSG_NOTICE([libvisual pluginsdir: $LIBVIS_PLUGINSDIR])
  if test x$LIBVIS_PLUGINSDIR != x; then
    AC_DEFINE_UNQUOTED(LIBVISUAL_PLUGINSBASEDIR,
        "$LIBVIS_PLUGINSDIR",
        [directory in which the detected libvisual's plugins are located])
  fi
])

else

dnl not building plugins with external dependencies,
dnl but we still need to set the conditionals
AM_CONDITIONAL(USE_LIBVISUAL, false)

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

if test "x$PACKAGE_VERSION_NANO" = "x1"; then
  DEPRECATED_CFLAGS="-DGST_DISABLE_DEPRECATED"
else
  DEPRECATED_CFLAGS=""
fi
AC_SUBST(DEPRECATED_CFLAGS)

dnl every flag in GST_OPTION_CFLAGS can be overridden at make time
GST_OPTION_CFLAGS=" \$(WARNING_CFLAGS) \$(ERROR_CFLAGS) \$(DEBUG_CFLAGS) \$(PROFILE_CFLAGS) \$(GCOV_CFLAGS) \$(OPT_CFLAGS) \$(DEPRECATED_CFLAGS)"
GST_OPTION_CXXFLAGS=" \$(WARNING_CXXFLAGS) \$(ERROR_CXXFLAGS) \$(DEBUG_CFLAGS) \$(PROFILE_CFLAGS) \$(GCOV_CFLAGS) \$(OPT_CFLAGS) \$(DEPRECATED_CFLAGS)"
GST_OPTION_OBJCFLAGS=" \$(WARNING_OBJCFLAGS) \$(ERROR_OBJCFLAGS) \$(DEBUG_CFLAGS) \$(PROFILE_CFLAGS) \$(GCOV_CFLAGS) \$(OPT_CFLAGS) \$(DEPRECATED_CFLAGS)"
AC_SUBST(GST_OPTION_CFLAGS)
AC_SUBST(GST_OPTION_CXXFLAGS)
AC_SUBST(GST_OPTION_OBJCFLAGS)

dnl our libraries need to be versioned correctly
AC_SUBST(GST_LT_LDFLAGS)

dnl GST_PLUGINS_GL_CFLAGS
dnl prefer internal headers to already installed ones
dnl also add builddir include for enumtypes and marshal
GST_PLUGINS_GL_CFLAGS=" -I\$(top_srcdir)/gst-libs -I\$(top_builddir)/gst-libs"
AC_SUBST(GST_PLUGINS_GL_CFLAGS)

dnl FIXME: do we want to rename to GST_ALL_* ?
dnl add GST_OPTION_CFLAGS, but overridable
GST_OBJCFLAGS=" -I\$(top_srcdir)/gst-libs $GST_CFLAGS $GLIB_EXTRA_CFLAGS \$(GST_OPTION_OBJCFLAGS)"
GST_CXXFLAGS=" -I\$(top_srcdir)/gst-libs -I\$(top_builddir)/gst-libs $GST_CFLAGS $GST_CXXFLAGS $GLIB_EXTRA_CFLAGS \$(GST_OPTION_CXXFLAGS)"
GST_CFLAGS=" -I\$(top_srcdir)/gst-libs -I\$(top_builddir)/gst-libs $GST_CFLAGS $GLIB_EXTRA_CFLAGS \$(GST_OPTION_CFLAGS)"
AC_SUBST(GST_CFLAGS)
AC_SUBST(GST_CXXFLAGS)
AC_SUBST(GST_OBJCFLAGS)

dnl add GCOV libs because libtool strips -fprofile-arcs -ftest-coverage
GST_LIBS="$GST_LIBS $GMODULE_LIBS \$(GCOV_LIBS)"
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
GST_PLUGIN_LDFLAGS="-module -avoid-version -export-symbols-regex '^[_]*gst_plugin_.*' $GST_ALL_LDFLAGS"
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
gst-libs/gst/gl/x11/Makefile
gst-libs/gst/gl/android/Makefile
gst-libs/gst/gl/win32/Makefile
gst-libs/gst/gl/cocoa/Makefile
gst-libs/gst/gl/wayland/Makefile
gst-libs/gst/gl/glprototypes/Makefile
ext/Makefile
ext/libvisual/Makefile
win32/common/config.h
pkgconfig/Makefile
pkgconfig/gstreamer-gl.pc
pkgconfig/gstreamer-gl-uninstalled.pc
tests/Makefile
tests/check/Makefile
tests/examples/Makefile
tests/examples/cocoa/Makefile
tests/examples/cocoa/videooverlay/Makefile
tests/examples/generic/Makefile
tests/examples/generic/cube/Makefile
tests/examples/generic/cubeyuv/Makefile
tests/examples/generic/doublecube/Makefile
tests/examples/generic/recordgraphic/Makefile
tests/examples/clutter/Makefile
tests/examples/gtk/Makefile
tests/examples/gtk/gtkvideooverlay/Makefile
tests/examples/gtk/filternovideooverlay/Makefile
tests/examples/gtk/filtervideooverlay/Makefile
tests/examples/gtk/fxtest/Makefile
tests/examples/gtk/switchvideooverlay/Makefile
tests/examples/qt/Makefile
tests/examples/sdl/Makefile
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

cat << EOF

        OpenGL API's: ${GL_APIS}
    OpenGL Platforms: ${GL_PLATFORMS}
      Window Systems: ${GL_WINDOWS}

EOF
