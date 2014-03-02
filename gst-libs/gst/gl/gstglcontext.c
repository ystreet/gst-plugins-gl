/*
 * GStreamer
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
 * SECTION:gstglcontext
 * @short_description: OpenGL context abstraction
 * @title: GstGLContext
 * @see_also: #GstGLDisplay, #GstGLWindow
 *
 * #GstGLContext wraps an OpenGL context object in a uniform API.  As a result
 * of the limitation on OpenGL context, this object is not thread safe unless
 * specified and must only be activated in a single thread.
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#if defined(ANDROID) || defined(__ANDROID__)
/* Avoid a linker error with _isoc99_sscanf() when building a shared library
 * for android
 */
#define _GNU_SOURCE
#endif

#include <gmodule.h>

#include "gl.h"
#include "gstglcontext.h"

#if GST_GL_HAVE_PLATFORM_GLX
#include "x11/gstglcontext_glx.h"
#endif
#if GST_GL_HAVE_PLATFORM_EGL
#include "egl/gstglcontext_egl.h"
#endif
#if GST_GL_HAVE_PLATFORM_COCOA
#include "cocoa/gstglcontext_cocoa.h"
#endif
#if GST_GL_HAVE_PLATFORM_WGL
#include "win32/gstglcontext_wgl.h"
#endif

#define USING_OPENGL(display) (display->gl_api & GST_GL_API_OPENGL)
#define USING_OPENGL3(display) (display->gl_api & GST_GL_API_OPENGL3)
#define USING_GLES(display) (display->gl_api & GST_GL_API_GLES)
#define USING_GLES2(display) (display->gl_api & GST_GL_API_GLES2)
#define USING_GLES3(display) (display->gl_api & GST_GL_API_GLES3)

#define GST_CAT_DEFAULT gst_gl_context_debug
GST_DEBUG_CATEGORY (GST_CAT_DEFAULT);

#define gst_gl_context_parent_class parent_class
G_DEFINE_ABSTRACT_TYPE (GstGLContext, gst_gl_context, G_TYPE_OBJECT);

#define GST_GL_CONTEXT_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE((o), GST_GL_TYPE_CONTEXT, GstGLContextPrivate))

static gpointer gst_gl_context_create_thread (GstGLContext * context);
static void gst_gl_context_finalize (GObject * object);

struct _GstGLContextPrivate
{
  GstGLDisplay *display;

  GThread *gl_thread;

  /* conditions */
  GMutex render_lock;
  GCond create_cond;
  GCond destroy_cond;

  gboolean created;
  gboolean alive;

  GstGLContext *other_context;
  GstGLAPI gl_api;
  GError **error;
};

typedef struct
{
  GstGLContext parent;

  guintptr handle;
  GstGLPlatform platform;
  GstGLAPI available_apis;
} GstGLWrappedContext;

typedef struct
{
  GstGLContextClass parent;
} GstGLWrappedContextClass;

#define GST_GL_TYPE_WRAPPED_CONTEXT (gst_gl_wrapped_context_get_type())
GType gst_gl_wrapped_context_get_type (void);
G_DEFINE_TYPE (GstGLWrappedContext, gst_gl_wrapped_context,
    GST_GL_TYPE_CONTEXT);

#define GST_GL_WRAPPED_CONTEXT(o)           (G_TYPE_CHECK_INSTANCE_CAST((o), GST_GL_TYPE_WRAPPED_CONTEXT, GstGLWrappedContext))
#define GST_GL_WRAPPED_CONTEXT_CLASS(k)     (G_TYPE_CHECK_CLASS((k), GST_GL_TYPE_CONTEXT, GstGLContextClass))
#define GST_GL_IS_WRAPPED_CONTEXT(o)        (G_TYPE_CHECK_INSTANCE_TYPE((o), GST_GL_TYPE_WRAPPED_CONTEXT))
#define GST_GL_IS_WRAPPED_CONTEXT_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), GST_GL_TYPE_WRAPPED_CONTEXT))
#define GST_GL_WRAPPED_CONTEXT_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS((o), GST_GL_TYPE_WRAPPED_CONTEXT, GstGLWrappedContextClass))

GQuark
gst_gl_context_error_quark (void)
{
  return g_quark_from_static_string ("gst-gl-context-error-quark");
}

static void
_ensure_window (GstGLContext * context)
{
  GstGLWindow *window;

  if (context->window)
    return;

  window = gst_gl_window_new (context->priv->display);

  gst_gl_context_set_window (context, window);

  gst_object_unref (window);
}

static void
gst_gl_context_init (GstGLContext * context)
{
  context->priv = GST_GL_CONTEXT_GET_PRIVATE (context);

  context->window = NULL;
  context->gl_vtable = g_slice_alloc0 (sizeof (GstGLFuncs));

  g_mutex_init (&context->priv->render_lock);

  g_cond_init (&context->priv->create_cond);
  g_cond_init (&context->priv->destroy_cond);
  context->priv->created = FALSE;
}

static void
gst_gl_context_class_init (GstGLContextClass * klass)
{
  g_type_class_add_private (klass, sizeof (GstGLContextPrivate));

  klass->get_proc_address =
      GST_DEBUG_FUNCPTR (gst_gl_context_default_get_proc_address);

  G_OBJECT_CLASS (klass)->finalize = gst_gl_context_finalize;
}

static void
_init_debug (void)
{
  static volatile gsize _init = 0;

  if (g_once_init_enter (&_init)) {
    GST_DEBUG_CATEGORY_INIT (gst_gl_context_debug, "glcontext", 0,
        "glcontext element");
    g_once_init_leave (&_init, 1);
  }
}

/**
 * gst_gl_context_new:
 * @display: a #GstGLDisplay
 *
 * Create a new #GstGLContext with the specified @display
 *
 * Returns: a new #GstGLContext
 */
GstGLContext *
gst_gl_context_new (GstGLDisplay * display)
{
  GstGLContext *context = NULL;
  const gchar *user_choice;

  _init_debug ();

  user_choice = g_getenv ("GST_GL_PLATFORM");
  GST_INFO ("creating a context, user choice:%s", user_choice);
#if GST_GL_HAVE_PLATFORM_EGL
  if (!context && (!user_choice || g_strstr_len (user_choice, 7, "egl")))
    context = GST_GL_CONTEXT (gst_gl_context_egl_new ());
#endif
#if GST_GL_HAVE_PLATFORM_COCOA
  if (!context && (!user_choice || g_strstr_len (user_choice, 5, "cocoa")))
    context = GST_GL_CONTEXT (gst_gl_context_cocoa_new ());
#endif
#if GST_GL_HAVE_PLATFORM_GLX
  if (!context && (!user_choice || g_strstr_len (user_choice, 3, "glx")))
    context = GST_GL_CONTEXT (gst_gl_context_glx_new ());
#endif
#if GST_GL_HAVE_PLATFORM_WGL
  if (!context && (!user_choice || g_strstr_len (user_choice, 3, "wgl"))) {
    context = GST_GL_CONTEXT (gst_gl_context_wgl_new ());
  }
#endif

  if (!context) {
    /* subclass returned a NULL context */
    GST_WARNING ("Could not create context. user specified %s",
        user_choice ? user_choice : "(null)");

    return NULL;
  }

  context->priv->display = gst_object_ref (display);

  return context;
}

/**
 * gst_gl_context_new_wrapped:
 * @display: a #GstGLDisplay
 * @handle: the OpenGL context to wrap
 * @context_type: a #GstGLPlatform specifying the type of context in @handle
 * @available_apis: a #GstGLAPI containing the available OpenGL apis in @handle
 *
 * Wraps an existing OpenGL context into a #GstGLContext.  
 *
 * Returns: a #GstGLContext wrapping @handle
 */
GstGLContext *
gst_gl_context_new_wrapped (GstGLDisplay * display, guintptr handle,
    GstGLPlatform context_type, GstGLAPI available_apis)
{
  GstGLContext *context;
  GstGLWrappedContext *context_wrap = NULL;

  _init_debug ();

  context_wrap = g_object_new (GST_GL_TYPE_WRAPPED_CONTEXT, NULL);

  if (!context_wrap) {
    /* subclass returned a NULL context */
    GST_ERROR ("Could not wrap existing context");

    return NULL;
  }

  context = (GstGLContext *) context_wrap;

  context->priv->display = gst_object_ref (display);
  context_wrap->handle = handle;
  context_wrap->platform = context_type;
  context_wrap->available_apis = available_apis;

  return context;
}

static void
gst_gl_context_finalize (GObject * object)
{
  GstGLContext *context = GST_GL_CONTEXT (object);

  if (context->window) {
    gst_gl_window_set_resize_callback (context->window, NULL, NULL, NULL);
    gst_gl_window_set_draw_callback (context->window, NULL, NULL, NULL);

    if (context->priv->alive) {
      g_mutex_lock (&context->priv->render_lock);
      GST_INFO ("send quit gl window loop");
      gst_gl_window_quit (context->window);
      while (context->priv->alive) {
        g_cond_wait (&context->priv->destroy_cond, &context->priv->render_lock);
      }
      g_mutex_unlock (&context->priv->render_lock);
    }

    gst_gl_window_set_close_callback (context->window, NULL, NULL, NULL);

    if (context->priv->gl_thread) {
      gpointer ret = g_thread_join (context->priv->gl_thread);
      GST_INFO ("gl thread joined");
      if (ret != NULL)
        GST_ERROR ("gl thread returned a non-null pointer");
      context->priv->gl_thread = NULL;
    }

    gst_object_unref (context->window);
  }

  gst_object_unref (context->priv->display);

  if (context->gl_vtable) {
    g_slice_free (GstGLFuncs, context->gl_vtable);
    context->gl_vtable = NULL;
  }

  g_mutex_clear (&context->priv->render_lock);

  g_cond_clear (&context->priv->destroy_cond);
  g_cond_clear (&context->priv->create_cond);

  G_OBJECT_CLASS (gst_gl_context_parent_class)->finalize (object);
}

/**
 * gst_gl_context_activate:
 * @context: a #GstGLContext
 * @activate: %TRUE to activate, %FALSE to deactivate
 *
 * (De)activate the OpenGL context represented by this @context.
 *
 * In OpenGL terms, calls eglMakeCurrent or similar with this context and the
 * currently set window.  See gst_gl_context_set_window() for details.
 *
 * Returns: Whether the activation succeeded
 */
gboolean
gst_gl_context_activate (GstGLContext * context, gboolean activate)
{
  GstGLContextClass *context_class;
  gboolean result;

  g_return_val_if_fail (GST_GL_IS_CONTEXT (context), FALSE);
  context_class = GST_GL_CONTEXT_GET_CLASS (context);
  g_return_val_if_fail (context_class->activate != NULL, FALSE);

  result = context_class->activate (context, activate);

  return result;
}

/**
 * gst_gl_context_get_gl_api:
 * @context: a #GstGLContext
 *
 * Get the currently enabled OpenGL api.
 *
 * The currently available API may be limited by the #GstGLDisplay in use and/or
 * the #GstGLWindow chosen.
 *
 * Returns: the currently available OpenGL api
 */
GstGLAPI
gst_gl_context_get_gl_api (GstGLContext * context)
{
  GstGLContextClass *context_class;

  g_return_val_if_fail (GST_GL_IS_CONTEXT (context), GST_GL_API_NONE);
  context_class = GST_GL_CONTEXT_GET_CLASS (context);
  g_return_val_if_fail (context_class->get_gl_api != NULL, GST_GL_API_NONE);

  return context_class->get_gl_api (context);
}

/**
 * gst_gl_context_get_proc_address:
 * @context: a #GstGLContext
 * @name: an opengl function name
 *
 * Get a function pointer to a specified opengl function, @name.  If the the
 * specific function does not exist, NULL is returned instead.
 *
 * Platform specfic functions (names starting 'egl', 'glX', 'wgl', etc) can also
 * be retreived using this method.
 *
 * Returns: a function pointer or NULL
 */
gpointer
gst_gl_context_get_proc_address (GstGLContext * context, const gchar * name)
{
  gpointer ret;
  GstGLContextClass *context_class;

  g_return_val_if_fail (GST_GL_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (!GST_GL_IS_WRAPPED_CONTEXT (context), NULL);
  context_class = GST_GL_CONTEXT_GET_CLASS (context);
  g_return_val_if_fail (context_class->get_proc_address != NULL, NULL);

  ret = context_class->get_proc_address (context, name);

  return ret;
}

gpointer
gst_gl_context_default_get_proc_address (GstGLContext * context,
    const gchar * name)
{
  gpointer ret = NULL;

#ifdef USE_EGL_RPI

  //FIXME: Can't understand why default does not work
  // so for now retrieve proc addressed manually

  static GModule *module_egl = NULL;
  static GModule *module_glesv2 = NULL;

  if (!module_egl)
    module_egl = g_module_open ("/opt/vc/lib/libEGL.so", G_MODULE_BIND_LAZY);

  if (module_egl) {
    if (!g_module_symbol (module_egl, name, &ret)) {
      ret = NULL;
    }
  }

  if (ret)
    return ret;

  if (!module_glesv2)
    module_glesv2 =
        g_module_open ("/opt/vc/lib/libGLESv2.so", G_MODULE_BIND_LAZY);

  if (module_glesv2) {
    if (!g_module_symbol (module_glesv2, name, &ret)) {
      return NULL;
    }
  }
#else
  static GModule *module = NULL;

  if (!module)
    module = g_module_open (NULL, G_MODULE_BIND_LAZY);

  if (module) {
    if (!g_module_symbol (module, name, &ret)) {
      return NULL;
    }
  }
#endif

  return ret;
}

/**
 * gst_gl_context_set_window:
 * @context: a #GstGLContext
 * @window: (transfer full): a #GstGLWindow
 *
 * Set's the current window on @context to @window.  The window can only be
 * changed before gst_gl_context_create() has been called and the @window is not
 * already running.
 *
 * Returns: Whether the window was successfully updated
 */
gboolean
gst_gl_context_set_window (GstGLContext * context, GstGLWindow * window)
{
  g_return_val_if_fail (!GST_GL_IS_WRAPPED_CONTEXT (context), FALSE);

  /* we can't change the window while we are running */
  if (context->priv->alive)
    return FALSE;

  if (window) {
    if (gst_gl_window_is_running (window))
      return FALSE;

    g_weak_ref_set (&window->context_ref, context);
  }

  if (context->window)
    gst_object_unref (context->window);

  context->window = window ? gst_object_ref (window) : NULL;

  return TRUE;
}

/**
 * gst_gl_context_get_window:
 * @context: a #GstGLContext
 *
 * Returns: the currently set window
 */
GstGLWindow *
gst_gl_context_get_window (GstGLContext * context)
{
  g_return_val_if_fail (GST_GL_IS_CONTEXT (context), NULL);

  if (GST_GL_IS_WRAPPED_CONTEXT (context))
    return NULL;

  _ensure_window (context);

  return gst_object_ref (context->window);
}

/**
 * gst_gl_context_create:
 * @context: a #GstGLContext:
 * @other_context: (allow-none): a #GstGLContext to share OpenGL objects with
 * @error: (allow-none): a #GError
 *
 * Creates an OpenGL context in the current thread with the specified
 * @other_context as a context to share shareable OpenGL objects with.  See the
 * OpenGL specification for what is shared between contexts.
 *
 * If an error occurs, and @error is not %NULL, then error will contain details
 * of the error and %FALSE will be returned.
 *
 * Should only be called once.
 *
 * Returns: whether the context could successfully be created
 */
gboolean
gst_gl_context_create (GstGLContext * context,
    GstGLContext * other_context, GError ** error)
{
  gboolean alive = FALSE;

  g_return_val_if_fail (GST_GL_IS_CONTEXT (context), FALSE);
  g_return_val_if_fail (!GST_GL_IS_WRAPPED_CONTEXT (context), FALSE);
  _ensure_window (context);

  g_mutex_lock (&context->priv->render_lock);

  if (!context->priv->created) {
    context->priv->other_context = other_context;
    context->priv->error = error;

    context->priv->gl_thread = g_thread_new ("gstglcontext",
        (GThreadFunc) gst_gl_context_create_thread, context);

    g_cond_wait (&context->priv->create_cond, &context->priv->render_lock);

    context->priv->created = TRUE;

    GST_INFO ("gl thread created");
  }

  alive = context->priv->alive;

  g_mutex_unlock (&context->priv->render_lock);

  return alive;
}

static gboolean
_create_context_gles2 (GstGLContext * context, gint * gl_major, gint * gl_minor,
    GError ** error)
{
  const GstGLFuncs *gl;
  GLenum gl_err = GL_NO_ERROR;

  gl = context->gl_vtable;

  GST_INFO ("GL_VERSION: %s", gl->GetString (GL_VERSION));
  GST_INFO ("GL_SHADING_LANGUAGE_VERSION: %s",
      gl->GetString (GL_SHADING_LANGUAGE_VERSION));
  GST_INFO ("GL_VENDOR: %s", gl->GetString (GL_VENDOR));
  GST_INFO ("GL_RENDERER: %s", gl->GetString (GL_RENDERER));

  gl_err = gl->GetError ();
  if (gl_err != GL_NO_ERROR) {
    g_set_error (error, GST_GL_CONTEXT_ERROR, GST_GL_CONTEXT_ERROR_FAILED,
        "glGetString error: 0x%x", gl_err);
    return FALSE;
  }
#if GST_GL_HAVE_GLES2
  if (!GL_ES_VERSION_2_0) {
    g_set_error (error, GST_GL_CONTEXT_ERROR, GST_GL_CONTEXT_ERROR_OLD_LIBS,
        "OpenGL|ES >= 2.0 is required");
    return FALSE;
  }
#endif

  if (gl_major)
    *gl_major = 2;
  if (gl_minor)
    *gl_minor = 0;

  return TRUE;
}

gboolean
_create_context_opengl (GstGLContext * context, gint * gl_major,
    gint * gl_minor, GError ** error)
{
  const GstGLFuncs *gl;
  guint maj, min;
  GLenum gl_err = GL_NO_ERROR;
  GString *opengl_version = NULL;

  gl = context->gl_vtable;

  GST_INFO ("GL_VERSION: %s", gl->GetString (GL_VERSION));
  GST_INFO ("GL_SHADING_LANGUAGE_VERSION: %s",
      gl->GetString (GL_SHADING_LANGUAGE_VERSION));
  GST_INFO ("GL_VENDOR: %s", gl->GetString (GL_VENDOR));
  GST_INFO ("GL_RENDERER: %s", gl->GetString (GL_RENDERER));

  gl_err = gl->GetError ();
  if (gl_err != GL_NO_ERROR) {
    g_set_error (error, GST_GL_CONTEXT_ERROR, GST_GL_CONTEXT_ERROR_FAILED,
        "glGetString error: 0x%x", gl_err);
    return FALSE;
  }
  opengl_version =
      g_string_truncate (g_string_new ((gchar *) gl->GetString (GL_VERSION)),
      3);

  sscanf (opengl_version->str, "%d.%d", &maj, &min);

  g_string_free (opengl_version, TRUE);

  /* OpenGL > 1.2.0 */
  if ((maj < 1) || (maj < 2 && maj >= 1 && min < 2)) {
    g_set_error (error, GST_GL_CONTEXT_ERROR, GST_GL_CONTEXT_ERROR_OLD_LIBS,
        "OpenGL >= 1.2.0 required, found %u.%u", maj, min);
    return FALSE;
  }

  if (gl_major)
    *gl_major = maj;
  if (gl_minor)
    *gl_minor = min;

  return TRUE;
}

GstGLAPI
_compiled_api (void)
{
  GstGLAPI ret = GST_GL_API_NONE;

#if GST_GL_HAVE_OPENGL
  ret |= GST_GL_API_OPENGL | GST_GL_API_OPENGL3;
#endif
#if GST_GL_HAVE_GLES2
  ret |= GST_GL_API_GLES2;
#endif

  return ret;
}

static void
_unlock_create_thread (GstGLContext * context)
{
  g_mutex_unlock (&context->priv->render_lock);
}

//gboolean
//gst_gl_context_create (GstGLContext * context, GstGLContext * other_context, GError ** error)
static gpointer
gst_gl_context_create_thread (GstGLContext * context)
{
  GstGLContextClass *context_class;
  GstGLWindowClass *window_class;
  GstGLDisplay *display;
  GstGLFuncs *gl;
  gint gl_major = 0, gl_minor = 0;
  gboolean ret = FALSE;
  GstGLAPI compiled_api, user_api;
  gchar *api_string;
  gchar *compiled_api_s;
  gchar *user_api_string;
  const gchar *user_choice;
  GError **error;
  GstGLContext *other_context;

  g_mutex_lock (&context->priv->render_lock);

  error = context->priv->error;
  other_context = context->priv->other_context;

  context_class = GST_GL_CONTEXT_GET_CLASS (context);
  window_class = GST_GL_WINDOW_GET_CLASS (context->window);

  if (window_class->open) {
    if (!window_class->open (context->window, error))
      goto failure;
  }

  display = context->priv->display;
  gl = context->gl_vtable;
  compiled_api = _compiled_api ();

  user_choice = g_getenv ("GST_GL_API");

  user_api = gst_gl_api_from_string (user_choice);
  user_api_string = gst_gl_api_to_string (user_api);

  compiled_api_s = gst_gl_api_to_string (compiled_api);

  if ((user_api & compiled_api) == GST_GL_API_NONE) {
    g_set_error (error, GST_GL_CONTEXT_ERROR, GST_GL_CONTEXT_ERROR_WRONG_API,
        "Cannot create context with the user requested api (%s).  "
        "We have support for (%s)", user_api_string, compiled_api_s);
    g_free (user_api_string);
    g_free (compiled_api_s);
    goto failure;
  }

  if (context_class->choose_format &&
      !context_class->choose_format (context, error)) {
    g_assert (error == NULL || *error != NULL);
    g_free (compiled_api_s);
    g_free (user_api_string);
    goto failure;
  }

  GST_INFO ("Attempting to create opengl context. user chosen api(s) (%s), "
      "compiled api support (%s)", user_api_string, compiled_api_s);

  if (!context_class->create_context (context, compiled_api & user_api,
          other_context, error)) {
    g_assert (error == NULL || *error != NULL);
    g_free (compiled_api_s);
    g_free (user_api_string);
    goto failure;
  }
  GST_INFO ("created context");

  if (!context_class->activate (context, TRUE)) {
    g_set_error (error, GST_GL_CONTEXT_ERROR,
        GST_GL_CONTEXT_ERROR_RESOURCE_UNAVAILABLE,
        "Failed to activate the GL Context");
    g_free (compiled_api_s);
    g_free (user_api_string);
    goto failure;
  }

  display->gl_api = gst_gl_context_get_gl_api (context);
  g_assert (display->gl_api != GST_GL_API_NONE
      && display->gl_api != GST_GL_API_ANY);

  api_string = gst_gl_api_to_string (display->gl_api);
  GST_INFO ("available GL APIs: %s", api_string);

  if (((compiled_api & display->gl_api) & user_api) == GST_GL_API_NONE) {
    g_set_error (error, GST_GL_CONTEXT_ERROR, GST_GL_CONTEXT_ERROR_WRONG_API,
        "failed to create context, context "
        "could not provide correct api. user (%s), compiled (%s), context (%s)",
        user_api_string, compiled_api_s, api_string);
    g_free (api_string);
    g_free (compiled_api_s);
    g_free (user_api_string);
    goto failure;
  }

  g_free (api_string);
  g_free (compiled_api_s);
  g_free (user_api_string);

  gl->GetError = gst_gl_context_get_proc_address (context, "glGetError");
  gl->GetString = gst_gl_context_get_proc_address (context, "glGetString");

  if (!gl->GetError || !gl->GetString) {
    g_set_error (error, GST_GL_CONTEXT_ERROR, GST_GL_CONTEXT_ERROR_FAILED,
        "could not GetProcAddress core opengl functions");
    goto failure;
  }

  /* gl api specific code */
  if (!ret && USING_OPENGL (display))
    ret = _create_context_opengl (context, &gl_major, &gl_minor, error);
  if (!ret && USING_GLES2 (display))
    ret = _create_context_gles2 (context, &gl_major, &gl_minor, error);

  if (!ret)
    goto failure;

  _gst_gl_feature_check_ext_functions (context, gl_major, gl_minor,
      (const gchar *) gl->GetString (GL_EXTENSIONS));

  context->priv->alive = TRUE;

  g_cond_signal (&context->priv->create_cond);

//  g_mutex_unlock (&context->priv->render_lock);
  gst_gl_window_send_message_async (context->window,
      (GstGLWindowCB) _unlock_create_thread, context, NULL);

  gst_gl_window_run (context->window);

  GST_INFO ("loop exited\n");

  g_mutex_lock (&context->priv->render_lock);

  context->priv->alive = FALSE;

  context_class->activate (context, FALSE);

  context_class->destroy_context (context);

  /* User supplied callback */
  if (context->window->close)
    context->window->close (context->window->close_data);

  /* window specific shutdown */
  if (window_class->close) {
    window_class->close (context->window);
  }

  g_cond_signal (&context->priv->destroy_cond);

  g_mutex_unlock (&context->priv->render_lock);

  return NULL;

failure:
  {
    g_cond_signal (&context->priv->create_cond);
    g_mutex_unlock (&context->priv->render_lock);
    return NULL;
  }
}

/**
 * gst_gl_context_get_gl_context:
 * @context: a #GstGLContext:
 *
 * Gets the backing OpenGL context used by @context.
 *
 * Returns: The platform specific backing OpenGL context
 */
guintptr
gst_gl_context_get_gl_context (GstGLContext * context)
{
  GstGLContextClass *context_class;
  guintptr result;

  g_return_val_if_fail (GST_GL_IS_CONTEXT (context), 0);
  context_class = GST_GL_CONTEXT_GET_CLASS (context);
  g_return_val_if_fail (context_class->get_gl_context != NULL, 0);

  result = context_class->get_gl_context (context);

  return result;
}

/**
 * gst_gl_context_get_gl_platform:
 * @context: a #GstGLContext:
 *
 * Gets the OpenGL platform that used by @context.
 *
 * Returns: The platform specific backing OpenGL context
 */
GstGLPlatform
gst_gl_context_get_gl_platform (GstGLContext * context)
{
  GstGLContextClass *context_class;

  g_return_val_if_fail (GST_GL_IS_CONTEXT (context), 0);
  context_class = GST_GL_CONTEXT_GET_CLASS (context);
  g_return_val_if_fail (context_class->get_gl_platform != NULL, 0);

  return context_class->get_gl_platform (context);
}

/**
 * gst_gl_context_get_display:
 * @context: a #GstGLContext:
 *
 * Returns: the #GstGLDisplay associated with this @context
 */
GstGLDisplay *
gst_gl_context_get_display (GstGLContext * context)
{
  g_return_val_if_fail (GST_GL_IS_CONTEXT (context), NULL);

  return gst_object_ref (context->priv->display);
}

typedef struct
{
  GstGLContext *context;
  GstGLContextThreadFunc func;
  gpointer data;
} RunGenericData;

static void
_gst_gl_context_thread_run_generic (RunGenericData * data)
{
  GST_TRACE ("running function:%p data:%p", data->func, data->data);

  data->func (data->context, data->data);
}

/**
 * gst_gl_context_thread_add:
 * @context: a #GstGLContext
 * @func: a #GstGLContextThreadFunc
 * @data: (closure): user data to call @func with
 *
 * Execute @func in the OpenGL thread of @context with @data
 *
 * MT-safe
 */
void
gst_gl_context_thread_add (GstGLContext * context,
    GstGLContextThreadFunc func, gpointer data)
{
  GstGLWindow *window;
  RunGenericData rdata;

  g_return_if_fail (GST_GL_IS_CONTEXT (context));
  g_return_if_fail (func != NULL);
  g_return_if_fail (!GST_GL_IS_WRAPPED_CONTEXT (context));

  rdata.context = context;
  rdata.data = data;
  rdata.func = func;

  window = gst_gl_context_get_window (context);

  gst_gl_window_send_message (window,
      GST_GL_WINDOW_CB (_gst_gl_context_thread_run_generic), &rdata);

  gst_object_unref (window);
}


static GstGLAPI
gst_gl_wrapped_context_get_gl_api (GstGLContext * context)
{
  GstGLWrappedContext *context_wrap = GST_GL_WRAPPED_CONTEXT (context);

  return context_wrap->available_apis;
}

static guintptr
gst_gl_wrapped_context_get_gl_context (GstGLContext * context)
{
  GstGLWrappedContext *context_wrap = GST_GL_WRAPPED_CONTEXT (context);

  return context_wrap->handle;
}

static GstGLPlatform
gst_gl_wrapped_context_get_gl_platform (GstGLContext * context)
{
  GstGLWrappedContext *context_wrap = GST_GL_WRAPPED_CONTEXT (context);

  return context_wrap->platform;
}

static gboolean
gst_gl_wrapped_context_activate (GstGLContext * context, gboolean activate)
{
  g_assert_not_reached ();

  return FALSE;
}

static void
gst_gl_wrapped_context_class_init (GstGLWrappedContextClass * klass)
{
  GstGLContextClass *context_class = (GstGLContextClass *) klass;

  context_class->get_gl_context =
      GST_DEBUG_FUNCPTR (gst_gl_wrapped_context_get_gl_context);
  context_class->get_gl_api =
      GST_DEBUG_FUNCPTR (gst_gl_wrapped_context_get_gl_api);
  context_class->get_gl_platform =
      GST_DEBUG_FUNCPTR (gst_gl_wrapped_context_get_gl_platform);
  context_class->activate = GST_DEBUG_FUNCPTR (gst_gl_wrapped_context_activate);
}

static void
gst_gl_wrapped_context_init (GstGLWrappedContext * context)
{
}

/* Must be called in the gl thread */
GstGLWrappedContext *
gst_gl_wrapped_context_new (void)
{
  GstGLWrappedContext *context =
      g_object_new (GST_GL_TYPE_WRAPPED_CONTEXT, NULL);

  return context;
}
