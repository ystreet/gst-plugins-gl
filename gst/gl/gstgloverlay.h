/*
 * GStreamer
 * Copyright (C) 2008 Filippo Argiolas <filippo.argiolas@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _GST_GL_OVERLAY_H_
#define _GST_GL_OVERLAY_H_

#include <gstglfilter.h>

#define GST_TYPE_GL_OVERLAY            (gst_gl_overlay_get_type())
#define GST_GL_OVERLAY(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_GL_OVERLAY,GstGLOverlay))
#define GST_IS_GL_OVERLAY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_GL_OVERLAY))
#define GST_GL_OVERLAY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass) , GST_TYPE_GL_OVERLAY,GstGLOverlayClass))
#define GST_IS_GL_OVERLAY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass) , GST_TYPE_GL_OVERLAY))
#define GST_GL_OVERLAY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj) , GST_TYPE_GL_OVERLAY,GstGLOverlayClass))

typedef struct _GstGLOverlay GstGLOverlay;
typedef struct _GstGLOverlayClass GstGLOverlayClass;

struct _GstGLOverlay
{
  GstGLFilter filter;
  gchar *location;
  gboolean pbuf_has_changed;
  gint8 pos_x_png;
  gint8 pos_y_png;
  guint8 size_png;
  gint8 pos_x_video;
  gint8 pos_y_video;
  guint8 size_video;
  gboolean video_top;
  guint8 rotate_png;
  guint8 rotate_video;
  gint8 angle_png;
  gint8 angle_video;
  guchar *pixbuf;
  gint width, height;
  GLuint pbuftexture;
  GLint internalFormat;
  GLenum format;
  gint type_file;               // 0 = No; 1 = PNG and 2 = JPEG
  gfloat width_window;
  gfloat height_window;
  gfloat posx;
  gfloat posy;
  gfloat ratio_window;
  gfloat ratio_texture;
  gfloat ratio_x;
  gfloat ratio_y;
  gfloat ratio_video;

/*  gboolean stretch; */
};

struct _GstGLOverlayClass
{
  GstGLFilterClass filter_class;
};

#endif /* _GST_GL_OVERLAY_H_ */
