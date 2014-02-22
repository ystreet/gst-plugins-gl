#!/bin/sh

FLAGS=

if "x$DISABLE_EGL" != "x"; then
  FLAGS="$FLAGS --disable-egl"
fi
if "x$DISABLE_GLX" != "x"; then
  FLAGS="$FLAGS --disable-glx"
fi
if "x$DISABLE_OPENGL" != "x"; then
  FLAGS="$FLAGS --disable-opengl"
fi
if "x$DISABLE_GLES2" != "x"; then
  FLAGS="$FLAGS --disable-gles2"
fi

./autogen.sh $FLAGS
