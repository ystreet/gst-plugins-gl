/*
 * GStreamer
 * Copyright (C) 2008-2010 Filippo Argiolas <filippo.argiolas@gmail.com>
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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:element-glfiltersobel.
 *
 * Sobel Edge Detection.
 *
 * <refsect2>
 * <title>Examples</title>
 * |[
 * gst-launch videotestsrc ! glupload ! glfiltersobel ! glimagesink
 * ]|
 * FBO (Frame Buffer Object) and GLSL (OpenGL Shading Language) are required.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstglfiltersobel.h"
#include <gstgleffectssources.h>

enum
{
  PROP_0,
  PROP_INVERT
};

#define GST_CAT_DEFAULT gst_gl_filtersobel_debug
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);

#define DEBUG_INIT \
  GST_DEBUG_CATEGORY_INIT (gst_gl_filtersobel_debug, "glfiltersobel", 0, "glfiltersobel element");

G_DEFINE_TYPE_WITH_CODE (GstGLFilterSobel, gst_gl_filtersobel,
    GST_TYPE_GL_FILTER, DEBUG_INIT);

static void gst_gl_filtersobel_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_gl_filtersobel_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);
static void gst_gl_filter_filtersobel_reset (GstGLFilter * filter);

static gboolean gst_gl_filtersobel_init_shader (GstGLFilter * filter);
static gboolean gst_gl_filtersobel_filter (GstGLFilter * filter,
    GstGLBuffer * inbuf, GstGLBuffer * outbuf);

static void gst_gl_filtersobel_length (gint width, gint height, guint texture,
    gpointer stuff);

static void
gst_gl_filtersobel_init_resources (GstGLFilter * filter)
{
  GstGLFilterSobel *filtersobel = GST_GL_FILTERSOBEL (filter);
  int i;

  for (i = 0; i < 2; i++) {
    glGenTextures (1, &filtersobel->midtexture[i]);
    glBindTexture (GL_TEXTURE_RECTANGLE_ARB, filtersobel->midtexture[i]);
    glTexImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA8,
        filter->width, filter->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER,
        GL_LINEAR);
    glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER,
        GL_LINEAR);
    glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S,
        GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T,
        GL_CLAMP_TO_EDGE);
  }
}

static void
gst_gl_filtersobel_reset_resources (GstGLFilter * filter)
{
  GstGLFilterSobel *filtersobel = GST_GL_FILTERSOBEL (filter);
  int i;

  for (i = 0; i < 2; i++) {
    glDeleteTextures (1, &filtersobel->midtexture[i]);
  }
}

static void
gst_gl_filtersobel_class_init (GstGLFilterSobelClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *element_class;

  gobject_class = (GObjectClass *) klass;
  element_class = GST_ELEMENT_CLASS (klass);

  gobject_class->set_property = gst_gl_filtersobel_set_property;
  gobject_class->get_property = gst_gl_filtersobel_get_property;

  GST_GL_FILTER_CLASS (klass)->filter = gst_gl_filtersobel_filter;
  GST_GL_FILTER_CLASS (klass)->display_init_cb =
      gst_gl_filtersobel_init_resources;
  GST_GL_FILTER_CLASS (klass)->display_reset_cb =
      gst_gl_filtersobel_reset_resources;
  GST_GL_FILTER_CLASS (klass)->onInitFBO = gst_gl_filtersobel_init_shader;
  GST_GL_FILTER_CLASS (klass)->onReset = gst_gl_filter_filtersobel_reset;

  g_object_class_install_property (gobject_class,
      PROP_INVERT,
      g_param_spec_boolean ("invert",
          "Invert the colors",
          "Invert colors to get dark edges on bright background",
          FALSE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gst_element_class_set_details_simple (element_class,
      "Gstreamer OpenGL Sobel", "Filter/Effect", "Sobel edge detection",
      "Filippo Argiolas <filippo.argiolas@gmail.com>");
}

static void
gst_gl_filtersobel_init (GstGLFilterSobel * filtersobel)
{
  int i;
  filtersobel->hconv = NULL;
  filtersobel->vconv = NULL;
  filtersobel->invert = FALSE;
  for (i = 0; i < 2; i++) {
    filtersobel->midtexture[i] = 0;
  }
}

static void
gst_gl_filter_filtersobel_reset (GstGLFilter * filter)
{
  GstGLFilterSobel *filtersobel = GST_GL_FILTERSOBEL (filter);

  //blocking call, wait the opengl thread has destroyed the shader
  gst_gl_display_del_shader (filter->display, filtersobel->desat);
  gst_gl_display_del_shader (filter->display, filtersobel->hconv);
  gst_gl_display_del_shader (filter->display, filtersobel->vconv);
  gst_gl_display_del_shader (filter->display, filtersobel->len);
}

static void
gst_gl_filtersobel_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstGLFilterSobel *filtersobel = GST_GL_FILTERSOBEL (object);

  switch (prop_id) {
    case PROP_INVERT:
      filtersobel->invert = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_gl_filtersobel_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstGLFilterSobel *filtersobel = GST_GL_FILTERSOBEL (object);

  switch (prop_id) {
    case PROP_INVERT:
      g_value_set_boolean (value, filtersobel->invert);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static gboolean
gst_gl_filtersobel_init_shader (GstGLFilter * filter)
{
  GstGLFilterSobel *filtersobel = GST_GL_FILTERSOBEL (filter);
  gboolean ret = TRUE;

  //blocking call, wait the opengl thread has compiled the shader
  ret =
      gst_gl_display_gen_shader (filter->display, 0, desaturate_fragment_source,
      &filtersobel->desat);
  ret &=
      gst_gl_display_gen_shader (filter->display, 0,
      sep_sobel_hconv3_fragment_source, &filtersobel->hconv);
  ret &=
      gst_gl_display_gen_shader (filter->display, 0,
      sep_sobel_vconv3_fragment_source, &filtersobel->vconv);
  ret &=
      gst_gl_display_gen_shader (filter->display, 0,
      sep_sobel_length_fragment_source, &filtersobel->len);

  return ret;
}

static gboolean
gst_gl_filtersobel_filter (GstGLFilter * filter, GstGLBuffer * inbuf,
    GstGLBuffer * outbuf)
{
  GstGLFilterSobel *filtersobel = GST_GL_FILTERSOBEL (filter);

  gst_gl_filter_render_to_target_with_shader (filter, inbuf->texture,
      filtersobel->midtexture[0], filtersobel->desat);
  gst_gl_filter_render_to_target_with_shader (filter,
      filtersobel->midtexture[0], filtersobel->midtexture[1],
      filtersobel->hconv);
  gst_gl_filter_render_to_target_with_shader (filter,
      filtersobel->midtexture[1], filtersobel->midtexture[0],
      filtersobel->vconv);
  gst_gl_filter_render_to_target (filter, filtersobel->midtexture[0],
      outbuf->texture, gst_gl_filtersobel_length, filtersobel);
  return TRUE;
}

static void
gst_gl_filtersobel_length (gint width, gint height, guint texture,
    gpointer stuff)
{
  GstGLFilter *filter = GST_GL_FILTER (stuff);
  GstGLFilterSobel *filtersobel = GST_GL_FILTERSOBEL (filter);

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();

  gst_gl_shader_use (filtersobel->len);

  glActiveTexture (GL_TEXTURE1);
  glEnable (GL_TEXTURE_RECTANGLE_ARB);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
  glDisable (GL_TEXTURE_RECTANGLE_ARB);

  gst_gl_shader_set_uniform_1i (filtersobel->len, "tex", 1);
  gst_gl_shader_set_uniform_1i (filtersobel->len, "invert",
      filtersobel->invert);

  gst_gl_filter_draw_texture (filter, texture);
}
