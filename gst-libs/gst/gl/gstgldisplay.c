/*
 * GStreamer
 * Copyright (C) 2007 David A. Schleef <ds@schleef.org>
 * Copyright (C) 2008 Julien Isorce <julien.isorce@gmail.com>
 * Copyright (C) 2008 Filippo Argiolas <filippo.argiolas@gmail.com>
 * Copyright (C) 2013 Matthew Waters <ystreet00@gmail.com>
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
 * SECTION:gstgldisplay
 * @short_description: window system display connection abstraction
 * @title: GstGLDisplay
 * @see_also: #GstContext, #GstGLContext, #GstGLWindow
 *
 * #GstGLDisplay represents a connection to the underlying windowing system. 
 * Elements are required to make use of #GstContext to share and propogate
 * a #GstGLDisplay.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gl.h"
#include "gstgldisplay.h"

#if GST_GL_HAVE_WINDOW_X11
#include <gst/gl/x11/gstgldisplay_x11.h>
#endif

GST_DEBUG_CATEGORY_STATIC (gst_context);
GST_DEBUG_CATEGORY_STATIC (gst_gl_display_debug);
#define GST_CAT_DEFAULT gst_gl_display_debug

#define DEBUG_INIT \
  GST_DEBUG_CATEGORY_INIT (gst_gl_display_debug, "gldisplay", 0, "opengl display"); \
  GST_DEBUG_CATEGORY_GET (gst_context, "GST_CONTEXT");

G_DEFINE_TYPE_WITH_CODE (GstGLDisplay, gst_gl_display, G_TYPE_OBJECT,
    DEBUG_INIT);

#define GST_GL_DISPLAY_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE((o), GST_TYPE_GL_DISPLAY, GstGLDisplayPrivate))

static void gst_gl_display_finalize (GObject * object);
static guintptr gst_gl_display_default_get_handle (GstGLDisplay * display);

struct _GstGLDisplayPrivate
{
  gint dummy;
};

static void
gst_gl_display_class_init (GstGLDisplayClass * klass)
{
  g_type_class_add_private (klass, sizeof (GstGLDisplayPrivate));

  klass->get_handle = gst_gl_display_default_get_handle;

  G_OBJECT_CLASS (klass)->finalize = gst_gl_display_finalize;
}

static void
gst_gl_display_init (GstGLDisplay * display)
{
  display->priv = GST_GL_DISPLAY_GET_PRIVATE (display);

  display->gl_api = GST_GL_API_ANY;
  display->type = GST_GL_DISPLAY_TYPE_ANY;

  GST_TRACE ("init %p", display);

  gst_gl_memory_init ();
}

static void
gst_gl_display_finalize (GObject * object)
{
  GstGLDisplay *display = GST_GL_DISPLAY (object);

  if (display->context) {
    gst_object_unref (display->context);
    display->context = NULL;
  }

  GST_TRACE ("finalize %p", object);

  G_OBJECT_CLASS (gst_gl_display_parent_class)->finalize (object);
}

/**
 * gst_gl_display_new:
 *
 * Returns: (transfer full): a new #GstGLDisplay
 */
GstGLDisplay *
gst_gl_display_new (void)
{
  GstGLDisplay *display = NULL;
  const gchar *user_choice;
  static volatile gsize _init = 0;

  if (g_once_init_enter (&_init)) {
    GST_DEBUG_CATEGORY_INIT (gst_gl_display_debug, "gldisplay", 0,
        "gldisplay element");
    g_once_init_leave (&_init, 1);
  }

  user_choice = g_getenv ("GST_GL_WINDOW");
  GST_INFO ("creating a window, user choice:%s", user_choice);

#if GST_GL_HAVE_WINDOW_X11
  if (!display && (!user_choice || g_strstr_len (user_choice, 3, "x11")))
    display = GST_GL_DISPLAY (gst_gl_display_x11_new (NULL));
#endif
  if (!display) {
    /* subclass returned a NULL window */
    GST_WARNING ("Could not create display. user specified %s, creating dummy",
        user_choice ? user_choice : "(null)");

    return g_object_new (GST_TYPE_GL_DISPLAY, NULL);
  }

  return display;
}

GstGLAPI
gst_gl_display_get_gl_api (GstGLDisplay * display)
{
  g_return_val_if_fail (GST_IS_GL_DISPLAY (display), GST_GL_API_NONE);

  return display->gl_api;
}

guintptr
gst_gl_display_get_handle (GstGLDisplay * display)
{
  GstGLDisplayClass *klass;

  g_return_val_if_fail (GST_IS_GL_DISPLAY (display), 0);
  klass = GST_GL_DISPLAY_GET_CLASS (display);
  g_return_val_if_fail (klass->get_handle != NULL, 0);

  return klass->get_handle (display);
}

static guintptr
gst_gl_display_default_get_handle (GstGLDisplay * display)
{
  return 0;
}

/**
 * gst_context_set_gl_display:
 * @context: a #GstContext
 * @display: resulting #GstGLDisplay
 *
 * Sets @display on @context
 */
void
gst_context_set_gl_display (GstContext * context, GstGLDisplay * display)
{
  GstStructure *s;

  g_return_if_fail (context != NULL);

  GST_CAT_LOG (gst_context, "setting GstGLDisplay(%p) on context(%p)", display,
      context);

  s = gst_context_writable_structure (context);
  gst_structure_set (s, GST_GL_DISPLAY_CONTEXT_TYPE, GST_TYPE_GL_DISPLAY,
      display, NULL);
}

/**
 * gst_context_get_gl_display:
 * @context: a #GstContext
 * @display: resulting #GstGLDisplay
 *
 * Returns: Whether @display was in @context
 */
gboolean
gst_context_get_gl_display (GstContext * context, GstGLDisplay ** display)
{
  const GstStructure *s;
  gboolean ret;

  g_return_val_if_fail (display != NULL, FALSE);
  g_return_val_if_fail (context != NULL, FALSE);

  s = gst_context_get_structure (context);
  ret = gst_structure_get (s, GST_GL_DISPLAY_CONTEXT_TYPE,
      GST_TYPE_GL_DISPLAY, display, NULL);

  GST_CAT_LOG (gst_context, "got GstGLDisplay(%p) from context(%p)", *display,
      context);

  return ret;
}
