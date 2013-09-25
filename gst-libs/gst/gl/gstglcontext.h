/*
 * GStreamer
 * Copyright (C) 2012 Matthew Waters <ystreet00@gmail.com>
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

#ifndef __GST_GL_CONTEXT_H__
#define __GST_GL_CONTEXT_H__

#include <gst/gst.h>

#include <gst/gl/gl.h>

G_BEGIN_DECLS

#define GST_GL_TYPE_CONTEXT         (gst_gl_context_get_type())
#define GST_GL_CONTEXT(o)           (G_TYPE_CHECK_INSTANCE_CAST((o), GST_GL_TYPE_CONTEXT, GstGLContext))
#define GST_GL_CONTEXT_CLASS(k)     (G_TYPE_CHECK_CLASS((k), GST_GL_TYPE_CONTEXT, GstGLContextClass))
#define GST_GL_IS_CONTEXT(o)        (G_TYPE_CHECK_INSTANCE_TYPE((o), GST_GL_TYPE_CONTEXT))
#define GST_GL_IS_CONTEXT_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), GST_GL_TYPE_CONTEXT))
#define GST_GL_CONTEXT_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS((o), GST_GL_TYPE_CONTEXT, GstGLContextClass))
GType gst_gl_context_get_type     (void);

#define GST_GL_CONTEXT_ERROR (gst_gl_context_error_quark ())
GQuark gst_gl_context_error_quark (void);

/**
 * GstGLContextThreadFunc:
 * @display: a #GstGLDisplay
 * @data: user data
 *
 * Represents a function to run in the GL thread
 */
typedef void (*GstGLContextThreadFunc) (GstGLContext * context, gpointer data);

typedef enum
{
  GST_GL_CONTEXT_ERROR_FAILED,
  GST_GL_CONTEXT_ERROR_WRONG_CONFIG,
  GST_GL_CONTEXT_ERROR_WRONG_API,
  GST_GL_CONTEXT_ERROR_OLD_LIBS,
  GST_GL_CONTEXT_ERROR_CREATE_CONTEXT,
  GST_GL_CONTEXT_ERROR_RESOURCE_UNAVAILABLE,
} GstGLContextError;

struct _GstGLContext {
  /*< private >*/
  GObject parent;

  /*< public >*/
  GstGLWindow  *window;

  GstGLFuncs *gl_vtable;

  /*< private >*/
  gpointer _reserved[GST_PADDING];

  GstGLContextPrivate *priv;
};

struct _GstGLContextClass {
  /*< private >*/
  GObjectClass parent_class;

  guintptr (*get_gl_context)     (GstGLContext *context);
  GstGLAPI (*get_gl_api)         (GstGLContext *context);
  gpointer (*get_proc_address)   (GstGLContext *context, const gchar *name);
  gboolean (*activate)           (GstGLContext *context, gboolean activate);
  gboolean (*choose_format)      (GstGLContext *context, GError **error);
  gboolean (*create_context)     (GstGLContext *context, GstGLAPI gl_api,
                                  GstGLContext *other_context, GError ** error);
  void     (*destroy_context)    (GstGLContext *context);
  void     (*swap_buffers)       (GstGLContext *context);

  /*< private >*/
  gpointer _reserved[GST_PADDING];
};

/* methods */

GstGLContext * gst_gl_context_new  (GstGLDisplay *display);

gboolean      gst_gl_context_activate         (GstGLContext *context, gboolean activate);

GstGLDisplay * gst_gl_context_get_display (GstGLContext *context);
gpointer      gst_gl_context_get_proc_address (GstGLContext *context, const gchar *name);
GstGLPlatform gst_gl_context_get_platform     (GstGLContext *context);
GstGLAPI      gst_gl_context_get_gl_api       (GstGLContext *context);
guintptr      gst_gl_context_get_gl_context   (GstGLContext *context);

gboolean      gst_gl_context_create           (GstGLContext *context, GstGLContext *other_context, GError ** error);

gpointer      gst_gl_context_default_get_proc_address (GstGLContext *context, const gchar *name);

gboolean      gst_gl_context_set_window (GstGLContext *context, GstGLWindow *window);
GstGLWindow * gst_gl_context_get_window (GstGLContext *context);

/* FIXME: remove */
void gst_gl_context_thread_add (GstGLContext * display,
    GstGLContextThreadFunc func, gpointer data);

G_END_DECLS

#endif /* __GST_GL_CONTEXT_H__ */
