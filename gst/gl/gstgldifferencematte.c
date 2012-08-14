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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:element-gldifferencematte.
 *
 * Saves a background frame and replace it with a pixbuf.
 *
 * <refsect2>
 * <title>Examples</title>
 * |[
 * gst-launch videotestsrc ! glupload ! gldifferencemate location=backgroundimagefile ! glimagesink
 * ]|
 * FBO (Frame Buffer Object) and GLSL (OpenGL Shading Language) are required.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <png.h>
#include "gstgldifferencematte.h"
#include <gstgleffectssources.h>

#if PNG_LIBPNG_VER >= 10400
#define int_p_NULL         NULL
#define png_infopp_NULL    NULL
#endif

#define GST_CAT_DEFAULT gst_gl_differencematte_debug
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);

#define DEBUG_INIT \
  GST_DEBUG_CATEGORY_INIT (gst_gl_differencematte_debug, "gldifferencematte", 0, "gldifferencematte element");

G_DEFINE_TYPE_WITH_CODE (GstGLDifferenceMatte, gst_gl_differencematte,
    GST_TYPE_GL_FILTER, DEBUG_INIT);

static void gst_gl_differencematte_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec);
static void gst_gl_differencematte_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec);

static void gst_gl_differencematte_init_resources (GstGLFilter * filter);
static void gst_gl_differencematte_reset_resources (GstGLFilter * filter);

static gboolean gst_gl_differencematte_filter (GstGLFilter * filter,
    GstBuffer * inbuf, GstBuffer * outbuf);

static gboolean gst_gl_differencematte_loader (GstGLFilter * filter);

enum
{
  PROP_0,
  PROP_LOCATION,
};


/* init resources that need a gl context */
static void
gst_gl_differencematte_init_gl_resources (GstGLFilter * filter)
{
  GstGLDifferenceMatte *differencematte = GST_GL_DIFFERENCEMATTE (filter);
  gint i;

  for (i = 0; i < 4; i++) {
    glGenTextures (1, &differencematte->midtexture[i]);
    glBindTexture (GL_TEXTURE_RECTANGLE_ARB, differencematte->midtexture[i]);
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
    differencematte->shader[i] = gst_gl_shader_new ();
  }

  if (!gst_gl_shader_compile_and_check (differencematte->shader[0],
          difference_fragment_source, GST_GL_SHADER_FRAGMENT_SOURCE)) {
    gst_gl_display_set_error (GST_GL_FILTER (differencematte)->display,
        "Failed to initialize difference shader");
    GST_ELEMENT_ERROR (differencematte, RESOURCE, NOT_FOUND,
        GST_GL_DISPLAY_ERR_MSG (GST_GL_FILTER (differencematte)->display),
        (NULL));
    return;
  }

  if (!gst_gl_shader_compile_and_check (differencematte->shader[1],
          hconv7_fragment_source, GST_GL_SHADER_FRAGMENT_SOURCE)) {
    gst_gl_display_set_error (GST_GL_FILTER (differencematte)->display,
        "Failed to initialize hconv7 shader");
    GST_ELEMENT_ERROR (differencematte, RESOURCE, NOT_FOUND,
        GST_GL_DISPLAY_ERR_MSG (GST_GL_FILTER (differencematte)->display),
        (NULL));
    return;
  }

  if (!gst_gl_shader_compile_and_check (differencematte->shader[2],
          vconv7_fragment_source, GST_GL_SHADER_FRAGMENT_SOURCE)) {
    gst_gl_display_set_error (GST_GL_FILTER (differencematte)->display,
        "Failed to initialize vconv7 shader");
    GST_ELEMENT_ERROR (differencematte, RESOURCE, NOT_FOUND,
        GST_GL_DISPLAY_ERR_MSG (GST_GL_FILTER (differencematte)->display),
        (NULL));
    return;
  }

  if (!gst_gl_shader_compile_and_check (differencematte->shader[3],
          texture_interp_fragment_source, GST_GL_SHADER_FRAGMENT_SOURCE)) {
    gst_gl_display_set_error (GST_GL_FILTER (differencematte)->display,
        "Failed to initialize interp shader");
    GST_ELEMENT_ERROR (differencematte, RESOURCE, NOT_FOUND,
        GST_GL_DISPLAY_ERR_MSG (GST_GL_FILTER (differencematte)->display),
        (NULL));
    return;
  }
}

/* free resources that need a gl context */
static void
gst_gl_differencematte_reset_gl_resources (GstGLFilter * filter)
{
  GstGLDifferenceMatte *differencematte = GST_GL_DIFFERENCEMATTE (filter);
  gint i;

  glDeleteTextures (1, &differencematte->savedbgtexture);
  glDeleteTextures (1, &differencematte->newbgtexture);
  for (i = 0; i < 4; i++) {
    g_object_unref (differencematte->shader[i]);
    differencematte->shader[i] = NULL;
    glDeleteTextures (1, &differencematte->midtexture[i]);
  }
  differencematte->location = NULL;
  differencematte->pixbuf = NULL;
  differencematte->savedbgtexture = 0;
  differencematte->newbgtexture = 0;
  differencematte->bg_has_changed = FALSE;
}

static void
gst_gl_differencematte_class_init (GstGLDifferenceMatteClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *element_class;

  gobject_class = (GObjectClass *) klass;
  element_class = GST_ELEMENT_CLASS (klass);
  gobject_class->set_property = gst_gl_differencematte_set_property;
  gobject_class->get_property = gst_gl_differencematte_get_property;

  GST_GL_FILTER_CLASS (klass)->filter = gst_gl_differencematte_filter;
  GST_GL_FILTER_CLASS (klass)->display_init_cb =
      gst_gl_differencematte_init_gl_resources;
  GST_GL_FILTER_CLASS (klass)->display_reset_cb =
      gst_gl_differencematte_reset_gl_resources;
  GST_GL_FILTER_CLASS (klass)->onStart = gst_gl_differencematte_init_resources;
  GST_GL_FILTER_CLASS (klass)->onStop = gst_gl_differencematte_reset_resources;

  g_object_class_install_property (gobject_class,
      PROP_LOCATION,
      g_param_spec_string ("location",
          "Background image location",
          "Background image location", NULL,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gst_element_class_set_details_simple (element_class,
      "Gstreamer OpenGL DifferenceMatte", "Filter/Effect",
      "Saves a background frame and replace it with a pixbuf",
      "Filippo Argiolas <filippo.argiolas@gmail.com>");
}

void
gst_gl_differencematte_draw_texture (GstGLDifferenceMatte * differencematte,
    GLuint tex)
{
  GstGLFilter *filter = GST_GL_FILTER (differencematte);

  glActiveTexture (GL_TEXTURE0);
  glEnable (GL_TEXTURE_RECTANGLE_ARB);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, tex);

  glBegin (GL_QUADS);

  glTexCoord2f (0.0, 0.0);
  glVertex2f (-1.0, -1.0);
  glTexCoord2f ((gfloat) filter->width, 0.0);
  glVertex2f (1.0, -1.0);
  glTexCoord2f ((gfloat) filter->width, (gfloat) filter->height);
  glVertex2f (1.0, 1.0);
  glTexCoord2f (0.0, (gfloat) filter->height);
  glVertex2f (-1.0, 1.0);

  glEnd ();
}

static void
gst_gl_differencematte_init (GstGLDifferenceMatte * differencematte)
{
  differencematte->shader[0] = NULL;
  differencematte->shader[1] = NULL;
  differencematte->shader[2] = NULL;
  differencematte->shader[3] = NULL;
  differencematte->location = NULL;
  differencematte->pixbuf = NULL;
  differencematte->savedbgtexture = 0;
  differencematte->newbgtexture = 0;
  differencematte->bg_has_changed = FALSE;

  fill_gaussian_kernel (differencematte->kernel, 7, 30.0);
}

static void
gst_gl_differencematte_reset_resources (GstGLFilter * filter)
{
//  GstGLDifferenceMatte* differencematte = GST_GL_DIFFERENCEMATTE(filter);
}

static void
gst_gl_differencematte_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstGLDifferenceMatte *differencematte = GST_GL_DIFFERENCEMATTE (object);

  switch (prop_id) {
    case PROP_LOCATION:
      if (differencematte->location != NULL)
        g_free (differencematte->location);
      differencematte->bg_has_changed = TRUE;
      differencematte->location = g_value_dup_string (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_gl_differencematte_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstGLDifferenceMatte *differencematte = GST_GL_DIFFERENCEMATTE (object);

  switch (prop_id) {
    case PROP_LOCATION:
      g_value_set_string (value, differencematte->location);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_gl_differencematte_init_resources (GstGLFilter * filter)
{
//  GstGLDifferenceMatte *differencematte = GST_GL_DIFFERENCEMATTE (filter);
}

static void
gst_gl_differencematte_save_texture (gint width, gint height, guint texture,
    gpointer stuff)
{
  GstGLDifferenceMatte *differencematte = GST_GL_DIFFERENCEMATTE (stuff);

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();

  gst_gl_differencematte_draw_texture (differencematte, texture);
}

static void
init_pixbuf_texture (GstGLDisplay * display, gpointer data)
{
  GstGLDifferenceMatte *differencematte = GST_GL_DIFFERENCEMATTE (data);
  GstGLFilter *filter = GST_GL_FILTER (data);

  glDeleteTextures (1, &differencematte->newbgtexture);
  glGenTextures (1, &differencematte->newbgtexture);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, differencematte->newbgtexture);
  glTexImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA,
      (gint) differencematte->pbuf_width, (gint) differencematte->pbuf_height,
      0, GL_RGBA, GL_UNSIGNED_BYTE, differencematte->pixbuf);

  if (differencematte->savedbgtexture == 0) {
    glGenTextures (1, &differencematte->savedbgtexture);
    glBindTexture (GL_TEXTURE_RECTANGLE_ARB, differencematte->savedbgtexture);
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
gst_gl_differencematte_diff (gint width, gint height, guint texture,
    gpointer stuff)
{
  GstGLDifferenceMatte *differencematte = GST_GL_DIFFERENCEMATTE (stuff);

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();

  gst_gl_shader_use (differencematte->shader[0]);

  glActiveTexture (GL_TEXTURE0);
  glEnable (GL_TEXTURE_RECTANGLE_ARB);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
  glDisable (GL_TEXTURE_RECTANGLE_ARB);

  gst_gl_shader_set_uniform_1i (differencematte->shader[0], "current", 0);

  glActiveTexture (GL_TEXTURE1);
  glEnable (GL_TEXTURE_RECTANGLE_ARB);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, differencematte->savedbgtexture);
  glDisable (GL_TEXTURE_RECTANGLE_ARB);

  gst_gl_shader_set_uniform_1i (differencematte->shader[0], "saved", 1);

  gst_gl_differencematte_draw_texture (differencematte, texture);
}

static void
gst_gl_differencematte_hblur (gint width, gint height, guint texture,
    gpointer stuff)
{
  GstGLDifferenceMatte *differencematte = GST_GL_DIFFERENCEMATTE (stuff);

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();

  gst_gl_shader_use (differencematte->shader[1]);

  glActiveTexture (GL_TEXTURE0);
  glEnable (GL_TEXTURE_RECTANGLE_ARB);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
  glDisable (GL_TEXTURE_RECTANGLE_ARB);

  gst_gl_shader_set_uniform_1i (differencematte->shader[1], "tex", 0);

  gst_gl_shader_set_uniform_1fv (differencematte->shader[1], "kernel", 7,
      differencematte->kernel);

  gst_gl_differencematte_draw_texture (differencematte, texture);
}

static void
gst_gl_differencematte_vblur (gint width, gint height, guint texture,
    gpointer stuff)
{
  GstGLDifferenceMatte *differencematte = GST_GL_DIFFERENCEMATTE (stuff);

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();

  gst_gl_shader_use (differencematte->shader[2]);

  glActiveTexture (GL_TEXTURE0);
  glEnable (GL_TEXTURE_RECTANGLE_ARB);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
  glDisable (GL_TEXTURE_RECTANGLE_ARB);

  gst_gl_shader_set_uniform_1i (differencematte->shader[2], "tex", 0);

  gst_gl_shader_set_uniform_1fv (differencematte->shader[2], "kernel", 7,
      differencematte->kernel);

  gst_gl_differencematte_draw_texture (differencematte, texture);
}

static void
gst_gl_differencematte_interp (gint width, gint height, guint texture,
    gpointer stuff)
{
  GstGLDifferenceMatte *differencematte = GST_GL_DIFFERENCEMATTE (stuff);
  GstGLFilter *filter = GST_GL_FILTER (stuff);

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();

  gst_gl_shader_use (differencematte->shader[3]);

  glActiveTexture (GL_TEXTURE0);
  glEnable (GL_TEXTURE_RECTANGLE_ARB);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
  glDisable (GL_TEXTURE_RECTANGLE_ARB);

  gst_gl_shader_set_uniform_1i (differencematte->shader[3], "blend", 0);

  glActiveTexture (GL_TEXTURE1);
  glEnable (GL_TEXTURE_RECTANGLE_ARB);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, differencematte->newbgtexture);
  glDisable (GL_TEXTURE_RECTANGLE_ARB);

  gst_gl_shader_set_uniform_1i (differencematte->shader[3], "base", 1);
  gst_gl_shader_set_uniform_1f (differencematte->shader[3],
      "base_width", (gfloat) differencematte->pbuf_width);
  gst_gl_shader_set_uniform_1f (differencematte->shader[3],
      "base_height", (gfloat) differencematte->pbuf_height);

  gst_gl_shader_set_uniform_1f (differencematte->shader[3],
      "final_width", (gfloat) filter->width);
  gst_gl_shader_set_uniform_1f (differencematte->shader[3],
      "final_height", (gfloat) filter->height);

  glActiveTexture (GL_TEXTURE2);
  glEnable (GL_TEXTURE_RECTANGLE_ARB);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, differencematte->midtexture[2]);
  glDisable (GL_TEXTURE_RECTANGLE_ARB);

  gst_gl_shader_set_uniform_1i (differencematte->shader[3], "alpha", 2);

  gst_gl_differencematte_draw_texture (differencematte, texture);
}

static void
gst_gl_differencematte_identity (gint width, gint height, guint texture,
    gpointer stuff)
{
  GstGLDifferenceMatte *differencematte = GST_GL_DIFFERENCEMATTE (stuff);

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();

  gst_gl_differencematte_draw_texture (differencematte, texture);
}

static gboolean
gst_gl_differencematte_filter (GstGLFilter * filter, GstBuffer * inbuf,
    GstBuffer * outbuf)
{
  GstGLDifferenceMatte *differencematte = GST_GL_DIFFERENCEMATTE (filter);
  GstGLMeta *in_meta, *out_meta;

  in_meta = gst_buffer_get_gl_meta (inbuf);
  out_meta = gst_buffer_get_gl_meta (outbuf);

  if (!in_meta || !out_meta) {
    GST_ERROR ("A Buffer does not contain required GstGLMeta");
    return FALSE;
  }

  differencematte->intexture = in_meta->memory->tex_id;

  if (differencematte->bg_has_changed && (differencematte->location != NULL)) {

    if (!gst_gl_differencematte_loader (filter))
      differencematte->pixbuf = NULL;

    /* if loader failed then display is turned off */
    gst_gl_display_thread_add (filter->display, init_pixbuf_texture,
        differencematte);

    /* save current frame, needed to calculate difference between
     * this frame and next ones */
    gst_gl_filter_render_to_target (filter, in_meta->memory->tex_id,
        differencematte->savedbgtexture,
        gst_gl_differencematte_save_texture, differencematte);

    if (differencematte->pixbuf) {
      free (differencematte->pixbuf);
      differencematte->pixbuf = NULL;
    }

    differencematte->bg_has_changed = FALSE;
  }

  if (differencematte->savedbgtexture != 0) {
    gst_gl_filter_render_to_target (filter,
        in_meta->memory->tex_id,
        differencematte->midtexture[0],
        gst_gl_differencematte_diff, differencematte);
    gst_gl_filter_render_to_target (filter,
        differencematte->midtexture[0],
        differencematte->midtexture[1],
        gst_gl_differencematte_hblur, differencematte);
    gst_gl_filter_render_to_target (filter,
        differencematte->midtexture[1],
        differencematte->midtexture[2],
        gst_gl_differencematte_vblur, differencematte);
    gst_gl_filter_render_to_target (filter,
        in_meta->memory->tex_id,
        out_meta->memory->tex_id, gst_gl_differencematte_interp,
        differencematte);
  } else {
    gst_gl_filter_render_to_target (filter,
        in_meta->memory->tex_id,
        out_meta->memory->tex_id, gst_gl_differencematte_identity,
        differencematte);
  }

  return TRUE;
}

static void
user_warning_fn (png_structp png_ptr, png_const_charp warning_msg)
{
  g_warning ("%s\n", warning_msg);
}

#define LOAD_ERROR(msg) { GST_WARNING ("unable to load %s: %s", differencematte->location, msg); return FALSE; }

static gboolean
gst_gl_differencematte_loader (GstGLFilter * filter)
{
  GstGLDifferenceMatte *differencematte = GST_GL_DIFFERENCEMATTE (filter);

  png_structp png_ptr;
  png_infop info_ptr;
  guint sig_read = 0;
  png_uint_32 width = 0;
  png_uint_32 height = 0;
  gint bit_depth = 0;
  gint color_type = 0;
  gint interlace_type = 0;
  png_FILE_p fp = NULL;
  guint y = 0;
  guchar **rows = NULL;
  gint filler;

  if (!filter->display)
    return TRUE;

  if ((fp = fopen (differencematte->location, "rb")) == NULL)
    LOAD_ERROR ("file not found");

  png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

  if (png_ptr == NULL) {
    fclose (fp);
    LOAD_ERROR ("failed to initialize the png_struct");
  }

  png_set_error_fn (png_ptr, NULL, NULL, user_warning_fn);

  info_ptr = png_create_info_struct (png_ptr);
  if (info_ptr == NULL) {
    fclose (fp);
    png_destroy_read_struct (&png_ptr, png_infopp_NULL, png_infopp_NULL);
    LOAD_ERROR ("failed to initialize the memory for image information");
  }

  png_init_io (png_ptr, fp);

  png_set_sig_bytes (png_ptr, sig_read);

  png_read_info (png_ptr, info_ptr);

  png_get_IHDR (png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
      &interlace_type, int_p_NULL, int_p_NULL);

  if (color_type == PNG_COLOR_TYPE_RGB) {
    filler = 0xff;
    png_set_filler (png_ptr, filler, PNG_FILLER_AFTER);
    color_type = PNG_COLOR_TYPE_RGB_ALPHA;
  }

  if (color_type != PNG_COLOR_TYPE_RGB_ALPHA) {
    fclose (fp);
    png_destroy_read_struct (&png_ptr, png_infopp_NULL, png_infopp_NULL);
    LOAD_ERROR ("color type is not rgb");
  }

  differencematte->pbuf_width = width;
  differencematte->pbuf_height = height;

  differencematte->pixbuf =
      (guchar *) malloc (sizeof (guchar) * width * height * 4);

  rows = (guchar **) malloc (sizeof (guchar *) * height);

  for (y = 0; y < height; ++y)
    rows[y] = (guchar *) (differencematte->pixbuf + y * width * 4);

  png_read_image (png_ptr, rows);

  free (rows);

  png_read_end (png_ptr, info_ptr);
  png_destroy_read_struct (&png_ptr, &info_ptr, png_infopp_NULL);
  fclose (fp);

  return TRUE;
}
