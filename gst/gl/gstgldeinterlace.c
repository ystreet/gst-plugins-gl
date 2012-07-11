/*
 * GStreamer
 * Copyright (C) 2009 Julien Isorce <julien.isorce@mail.com>
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
 * SECTION:element-deinterlace
 *
 * Deinterlacing using based on fragment shaders.
 *
 * <refsect2>
 * <title>Examples</title>
 * |[
 * gst-launch videotestsrc ! glupload ! gldeinterlace ! glimagesink
 * ]|
 * FBO (Frame Buffer Object) and GLSL (OpenGL Shading Language) are required.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstgldeinterlace.h"

#define GST_CAT_DEFAULT gst_gl_deinterlace_debug
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);

enum
{
  PROP_0
};

#define DEBUG_INIT \
  GST_DEBUG_CATEGORY_INIT (gst_gl_deinterlace_debug, "gldeinterlace", 0, "gldeinterlace element");

G_DEFINE_TYPE_WITH_CODE (GstGLDeinterlace, gst_gl_deinterlace,
    GST_TYPE_GL_FILTER, DEBUG_INIT);

static void gst_gl_deinterlace_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec);
static void gst_gl_deinterlace_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec);

static void gst_gl_deinterlace_reset (GstGLFilter * filter);
static gboolean gst_gl_deinterlace_init_shader (GstGLFilter * filter);
static gboolean gst_gl_deinterlace_filter (GstGLFilter * filter,
    GstBuffer * inbuf, GstBuffer * outbuf);
static void gst_gl_deinterlace_callback (gint width, gint height,
    guint texture, gpointer stuff);

static const gchar *greedyh_fragment_source = "#extension GL_ARB_texture_rectangle : enable\n" "uniform sampler2DRect tex;\n" "uniform sampler2DRect tex_prev;\n" "uniform float max_comb;\n" "uniform float motion_threshold;\n" "uniform float motion_sense;\n" "uniform int width;\n" "uniform int height;\n" "" "void main () {\n" "  vec2 texcoord = gl_TexCoord[0].xy;\n" "  if (int(mod(texcoord.y, 2.0)) == 0)\n" "    gl_FragColor = vec4(texture2DRect(tex_prev, texcoord).rgb, 1.0);\n" "  else {\n" ""      // cannot have __ in a var so __ is replaced by _a
    "    vec2 texcoord_L1_a1, texcoord_L3_a1, texcoord_L1, texcoord_L3, texcoord_L1_1, texcoord_L3_1;\n" "    vec3 L1_a1, L3_a1, L1, L3, L1_1, L3_1;\n" "" "    texcoord_L1 = vec2(texcoord.x, texcoord.y - 1.0);\n" "    texcoord_L3 = vec2(texcoord.x, texcoord.y + 1.0);\n" "    L1 = texture2DRect(tex_prev, texcoord_L1).rgb;\n" "    L3 = texture2DRect(tex_prev, texcoord_L3).rgb;\n" "    if (int(ceil(texcoord.x)) == width && int(ceil(texcoord.y)) == height) {\n" "      L1_1 = L1;\n" "      L3_1 = L3;\n" "    } else {\n" "      texcoord_L1_1 = vec2(texcoord.x + 1.0, texcoord.y - 1.0);\n" "      texcoord_L3_1 = vec2(texcoord.x + 1.0, texcoord.y + 1.0);\n" "      L1_1 = texture2DRect(tex_prev, texcoord_L1_1).rgb;\n" "      L3_1 = texture2DRect(tex_prev, texcoord_L3_1).rgb;\n" "    }\n" "    if (int(ceil(texcoord.x + texcoord.y)) == 0) {\n" "      L1_a1 = L1;\n" "      L3_a1 = L3;\n" "    } else {\n" "      texcoord_L1_a1 = vec2(texcoord.x - 1.0, texcoord.y - 1.0);\n" "      texcoord_L3_a1 = vec2(texcoord.x - 1.0, texcoord.y + 1.0);\n" "      L1_a1 = texture2DRect(tex_prev, texcoord_L1_a1).rgb;\n" "      L3_a1 = texture2DRect(tex_prev, texcoord_L3_a1).rgb;\n" "    }\n" "" ""        //STEP 1
    "    vec3 avg_a1 = (L1_a1 + L3_a1) / 2.0;\n" "    vec3 avg = (L1 + L3) / 2.0;\n" "    vec3 avg_1 = (L1_1 + L3_1) / 2.0;\n" "" "    vec3 avg_s = (avg_a1 + avg_1) / 2.0;\n" "" "    vec3 avg_sc = (avg_s + avg) / 2.0;\n" "" "    vec3 L2 = texture2DRect(tex, texcoord).rgb;\n" "    vec3 LP2 = texture2DRect(tex_prev, texcoord).rgb;\n" "" "    vec3 best;\n" "" "    if (abs(L2.r - avg_sc.r) < abs(LP2.r - avg_sc.r)) {\n" "      best.r = L2.r;\n" "    } else {\n" "      best.r = LP2.r;\n" "    }\n" "" "    if (abs(L2.g - avg_sc.g) < abs(LP2.g - avg_sc.g)) {\n" "      best.g = L2.g;\n" "    } else {\n" "      best.g = LP2.g;\n" "    }\n" "" "    if (abs(L2.b - avg_sc.b) < abs(LP2.b - avg_sc.b)) {\n" "      best.b = L2.b;\n" "    } else {\n" "      best.b = LP2.b;\n" "    }\n" "" ""        //STEP 2
    "    vec3 last;\n" "    last.r = clamp(best.r, max(min(L1.r, L3.r) - max_comb, 0.0), min(max(L1.r, L3.r) + max_comb, 1.0));\n" "    last.g = clamp(best.g, max(min(L1.g, L3.g) - max_comb, 0.0), min(max(L1.g, L3.g) + max_comb, 1.0));\n" "    last.b = clamp(best.b, max(min(L1.b, L3.b) - max_comb, 0.0), min(max(L1.b, L3.b) + max_comb, 1.0));\n" "" ""        //STEP 3
    "    const vec3 luma = vec3 (0.299011, 0.586987, 0.114001);"
    "    float mov = min(max(abs(dot(L2 - LP2, luma)) - motion_threshold, 0.0) * motion_sense, 1.0);\n"
    "    last = last * (1.0 - mov) + avg_sc * mov;\n"
    "" "    gl_FragColor = vec4(last, 1.0);\n" "  }\n" "}\n";

static void
gst_gl_deinterlace_class_init (GstGLDeinterlaceClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *element_class;

  gobject_class = (GObjectClass *) klass;
  element_class = GST_ELEMENT_CLASS (klass);

  gobject_class->set_property = gst_gl_deinterlace_set_property;
  gobject_class->get_property = gst_gl_deinterlace_get_property;

  gst_element_class_set_details_simple (element_class,
      "OpenGL deinterlacing filter", "Deinterlace",
      "Deinterlacing based on fragment shaders",
      "Julien Isorce <julien.isorce@mail.com>");

  GST_GL_FILTER_CLASS (klass)->filter = gst_gl_deinterlace_filter;
  GST_GL_FILTER_CLASS (klass)->onInitFBO = gst_gl_deinterlace_init_shader;
  GST_GL_FILTER_CLASS (klass)->onReset = gst_gl_deinterlace_reset;
}

static void
gst_gl_deinterlace_init (GstGLDeinterlace * filter)
{
  filter->shader = NULL;
  filter->buffer_prev = NULL;
}

static void
gst_gl_deinterlace_reset (GstGLFilter * filter)
{
  GstGLDeinterlace *deinterlace_filter = GST_GL_DEINTERLACE (filter);

  if (deinterlace_filter->buffer_prev) {
    gst_buffer_unref (deinterlace_filter->buffer_prev);
    deinterlace_filter->buffer_prev = NULL;
  }
  //blocking call, wait the opengl thread has destroyed the shader
  gst_gl_display_del_shader (filter->display, deinterlace_filter->shader);
}

static void
gst_gl_deinterlace_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  //GstGLDeinterlace *filter = GST_GL_DEINTERLACE (object);

  switch (prop_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_gl_deinterlace_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  //GstGLDeinterlace *filter = GST_GL_DEINTERLACE (object);

  switch (prop_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static gboolean
gst_gl_deinterlace_init_shader (GstGLFilter * filter)
{
  GstGLDeinterlace *deinterlace_filter = GST_GL_DEINTERLACE (filter);

  //blocking call, wait the opengl thread has compiled the shader
  return gst_gl_display_gen_shader (filter->display, 0, greedyh_fragment_source,
      &deinterlace_filter->shader);
}

static gboolean
gst_gl_deinterlace_filter (GstGLFilter * filter, GstBuffer * inbuf,
    GstBuffer * outbuf)
{
  GstGLDeinterlace *deinterlace_filter = GST_GL_DEINTERLACE (filter);
  GstGLMeta *in_meta, *out_meta;
  GstVideoMeta *in_v_meta;

  in_meta = gst_buffer_get_gl_meta (inbuf);
  out_meta = gst_buffer_get_gl_meta (outbuf);
  in_v_meta = gst_buffer_get_video_meta (inbuf);

  if (!in_meta || !out_meta || !in_v_meta) {
    GST_WARNING ("A buffer does not contain required GstGLMeta or"
        " GstVideoMeta");
    return FALSE;
  }
  //blocking call, use a FBO
  gst_gl_display_use_fbo (filter->display, filter->width, filter->height,
      filter->fbo, filter->depthbuffer, out_meta->memory->tex_id,
      gst_gl_deinterlace_callback, in_v_meta->width, in_v_meta->height,
      in_meta->memory->tex_id, 0, filter->width, 0, filter->height,
      GST_GL_DISPLAY_PROJECTION_ORTHO2D, (gpointer) deinterlace_filter);

  if (deinterlace_filter->buffer_prev)
    gst_buffer_unref (deinterlace_filter->buffer_prev);

  deinterlace_filter->buffer_prev = gst_buffer_ref (inbuf);

  return TRUE;
}

//opengl scene, params: input texture (not the output filter->texture)
static void
gst_gl_deinterlace_callback (gint width, gint height, guint texture,
    gpointer stuff)
{
  GstGLDeinterlace *deinterlace_filter = GST_GL_DEINTERLACE (stuff);
  GstGLFilter *filter = GST_GL_FILTER (stuff);
  GstBuffer *buffer_prev = deinterlace_filter->buffer_prev;
  GstGLMeta *prev_meta;


  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();

  gst_gl_shader_use (deinterlace_filter->shader);

  glEnable (GL_TEXTURE_RECTANGLE_ARB);

  if (buffer_prev) {
    prev_meta = gst_buffer_get_gl_meta (buffer_prev);
    glActiveTexture (GL_TEXTURE1_ARB);
    gst_gl_shader_set_uniform_1i (deinterlace_filter->shader, "tex_prev", 1);
    glBindTexture (GL_TEXTURE_RECTANGLE_ARB, prev_meta->memory->tex_id);
  }

  glActiveTexture (GL_TEXTURE0_ARB);
  gst_gl_shader_set_uniform_1i (deinterlace_filter->shader, "tex", 0);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);

  gst_gl_shader_set_uniform_1f (deinterlace_filter->shader, "max_comb",
      5.0f / 255.0f);
  gst_gl_shader_set_uniform_1f (deinterlace_filter->shader, "motion_threshold",
      25.0f / 255.0f);
  gst_gl_shader_set_uniform_1f (deinterlace_filter->shader, "motion_sense",
      30.0f / 255.0f);

  gst_gl_shader_set_uniform_1i (deinterlace_filter->shader, "width",
      filter->width);
  gst_gl_shader_set_uniform_1i (deinterlace_filter->shader, "height",
      filter->height);

  glBegin (GL_QUADS);
  glMultiTexCoord2iARB (GL_TEXTURE0_ARB, 0, 0);
  glMultiTexCoord2iARB (GL_TEXTURE1_ARB, 0, 0);
  glVertex2i (-1, -1);
  glMultiTexCoord2iARB (GL_TEXTURE0_ARB, width, 0);
  glMultiTexCoord2iARB (GL_TEXTURE1_ARB, width, 0);
  glVertex2i (1, -1);
  glMultiTexCoord2iARB (GL_TEXTURE0_ARB, width, height);
  glMultiTexCoord2iARB (GL_TEXTURE1_ARB, width, height);
  glVertex2i (1, 1);
  glMultiTexCoord2iARB (GL_TEXTURE0_ARB, 0, height);
  glMultiTexCoord2iARB (GL_TEXTURE1_ARB, 0, height);
  glVertex2i (-1, 1);
  glEnd ();

  glDisable (GL_TEXTURE_RECTANGLE_ARB);
}
