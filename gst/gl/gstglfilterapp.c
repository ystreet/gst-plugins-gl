/* 
 * GStreamer
 * Copyright (C) 2008 Julien Isorce <julien.isorce@gmail.com>
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

/**
 * SECTION:element-glfilterapp
 *
 * The resize and redraw callbacks can be set from a client code.
 *
 * <refsect2>
 * <title>CLient callbacks</title>
 * <para>
 * The graphic scene can be written from a client code through the 
 * two glfilterapp properties.
 * </para>
 * </refsect2>
 * <refsect2>
 * <title>Examples</title>
 * see gst-plugins-gl/tests/examples/generic/recordgraphic
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstglfilterapp.h"

#define GST_CAT_DEFAULT gst_gl_filter_app_debug
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);

enum
{
  PROP_0,
  PROP_CLIENT_RESHAPE_CALLBACK,
  PROP_CLIENT_DRAW_CALLBACK,
  PROP_CLIENT_DATA
};

#define DEBUG_INIT \
  GST_DEBUG_CATEGORY_INIT (gst_gl_filter_app_debug, "glfilterapp", 0, "glfilterapp element");

G_DEFINE_TYPE_WITH_CODE (GstGLFilterApp, gst_gl_filter_app,
    GST_TYPE_GL_FILTER, DEBUG_INIT);

static void gst_gl_filter_app_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_gl_filter_app_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static gboolean gst_gl_filter_app_set_caps (GstGLFilter * filter,
    GstCaps * incaps, GstCaps * outcaps);
static gboolean gst_gl_filter_app_filter_texture (GstGLFilter * filter,
    guint in_tex, guint out_tex);
static void gst_gl_filter_app_callback (gint width, gint height, guint texture,
    gpointer stuff);


static void
gst_gl_filter_app_class_init (GstGLFilterAppClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *element_class;

  gobject_class = (GObjectClass *) klass;
  element_class = GST_ELEMENT_CLASS (klass);

  gobject_class->set_property = gst_gl_filter_app_set_property;
  gobject_class->get_property = gst_gl_filter_app_get_property;

  GST_GL_FILTER_CLASS (klass)->set_caps = gst_gl_filter_app_set_caps;
  GST_GL_FILTER_CLASS (klass)->filter_texture =
      gst_gl_filter_app_filter_texture;

  g_object_class_install_property (gobject_class, PROP_CLIENT_RESHAPE_CALLBACK,
      g_param_spec_pointer ("client-reshape-callback",
          "Client reshape callback",
          "Define a custom reshape callback in a client code",
          G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_CLIENT_DRAW_CALLBACK,
      g_param_spec_pointer ("client-draw-callback", "Client draw callback",
          "Define a custom draw callback in a client code",
          G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_CLIENT_DATA,
      g_param_spec_pointer ("client-data", "Client data",
          "Pass data to the draw and reshape callbacks",
          G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS));

  gst_element_class_set_metadata (element_class,
      "OpenGL application filter", "Filter/Effect",
      "Use client callbacks to define the scene",
      "Julien Isorce <julien.isorce@gmail.com>");
}

static void
gst_gl_filter_app_init (GstGLFilterApp * filter)
{
  filter->clientReshapeCallback = NULL;
  filter->clientDrawCallback = NULL;
  filter->client_data = NULL;
}

static void
gst_gl_filter_app_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstGLFilterApp *filter = GST_GL_FILTER_APP (object);

  switch (prop_id) {
    case PROP_CLIENT_RESHAPE_CALLBACK:
    {
      filter->clientReshapeCallback = g_value_get_pointer (value);
      break;
    }
    case PROP_CLIENT_DRAW_CALLBACK:
    {
      filter->clientDrawCallback = g_value_get_pointer (value);
      break;
    }
    case PROP_CLIENT_DATA:
    {
      filter->client_data = g_value_get_pointer (value);
      break;
    }
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_gl_filter_app_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  //GstGLFilterApp* filter = GST_GL_FILTER_APP (object);

  switch (prop_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static gboolean
gst_gl_filter_app_set_caps (GstGLFilter * filter, GstCaps * incaps,
    GstCaps * outcaps)
{
  //GstGLFilterApp* app_filter = GST_GL_FILTER_APP(filter);

  return TRUE;
}

static gboolean
gst_gl_filter_app_filter_texture (GstGLFilter * filter, guint in_tex,
    guint out_tex)
{
  GstGLFilterApp *app_filter = GST_GL_FILTER_APP (filter);

  if (app_filter->clientDrawCallback) {
    //blocking call, use a FBO
    gst_gl_context_use_fbo (filter->context,
        GST_VIDEO_INFO_WIDTH (&filter->out_info),
        GST_VIDEO_INFO_HEIGHT (&filter->out_info),
        filter->fbo, filter->depthbuffer, out_tex,
        app_filter->clientDrawCallback,
        GST_VIDEO_INFO_WIDTH (&filter->in_info),
        GST_VIDEO_INFO_HEIGHT (&filter->in_info),
        in_tex, 45,
        (gfloat) GST_VIDEO_INFO_WIDTH (&filter->out_info) /
        (gfloat) GST_VIDEO_INFO_HEIGHT (&filter->out_info),
        0.1, 100, GST_GL_DISPLAY_PROJECTION_PERSPECTIVE,
        app_filter->client_data);
  }
  //default
  else {
    //blocking call, use a FBO
    gst_gl_filter_render_to_target (filter, TRUE, in_tex, out_tex,
        gst_gl_filter_app_callback, filter);
  }

  return TRUE;
}

//opengl scene, params: input texture (not the output filter->texture)
static void
gst_gl_filter_app_callback (gint width, gint height, guint texture,
    gpointer stuff)
{
  GstGLFilter *filter = GST_GL_FILTER (stuff);
  GstGLFuncs *gl = filter->context->gl_vtable;

  gl->MatrixMode (GL_PROJECTION);
  gl->LoadIdentity ();

  gst_gl_filter_draw_texture (filter, texture, width, height);
}
