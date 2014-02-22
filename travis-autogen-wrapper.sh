#!/bin/sh

FLAGS=

if test "x$DISABLE_EGL" != "x"; then
  FLAGS="$FLAGS --disable-egl"
fi
if test "x$DISABLE_GLX" != "x"; then
  FLAGS="$FLAGS --disable-glx"
fi
if test "x$DISABLE_OPENGL" != "x"; then
  FLAGS="$FLAGS --disable-opengl"
fi
if test "x$DISABLE_GLES2" != "x"; then
  FLAGS="$FLAGS --disable-gles2"
fi

echo "Running ./autogen.sh $FLAGS"

./autogen.sh $FLAGS
