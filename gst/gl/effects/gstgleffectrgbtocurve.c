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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "../gstgleffects.h"
#include "gstgleffectscurves.h"

static void
gst_gl_effects_rgb_to_curve (GstGLEffects * effects,
    GstGLEffectsCurve curve,
    gint curve_index, gint width, gint height, GLuint texture)
{
  GstGLShader *shader;
  GstGLContext *context = GST_GL_FILTER (effects)->context;
  GstGLFuncs *gl = context->gl_vtable;

  shader = g_hash_table_lookup (effects->shaderstable, "rgbmap0");

  if (!shader) {
    shader = gst_gl_shader_new (context);
    g_hash_table_insert (effects->shaderstable, "rgbmap0", shader);
  }

  if (!gst_gl_shader_compile_and_check (shader,
          rgb_to_curve_fragment_source, GST_GL_SHADER_FRAGMENT_SOURCE)) {
    gst_gl_context_set_error (context,
        "Failed to initialize rgb to curve shader");
    GST_ELEMENT_ERROR (effects, RESOURCE, NOT_FOUND,
        ("%s", gst_gl_context_get_error ()), (NULL));
    return;
  }

  gl->MatrixMode (GL_PROJECTION);
  gl->LoadIdentity ();

  gst_gl_shader_use (shader);

  if (effects->curve[curve_index] == 0) {
    /* this parameters are needed to have a right, predictable, mapping */
    gl->GenTextures (1, &effects->curve[curve_index]);
    gl->Enable (GL_TEXTURE_1D);
    gl->BindTexture (GL_TEXTURE_1D, effects->curve[curve_index]);
    gl->TexParameteri (GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->TexParameteri (GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->TexParameteri (GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    gl->TexParameteri (GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    gl->TexImage1D (GL_TEXTURE_1D, 0, curve.bytes_per_pixel,
        curve.width, 0, GL_RGB, GL_UNSIGNED_BYTE, curve.pixel_data);

    gl->Disable (GL_TEXTURE_1D);
  }

  gl->ActiveTexture (GL_TEXTURE0);
  gl->Enable (GL_TEXTURE_RECTANGLE_ARB);
  gl->BindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);

  gst_gl_shader_set_uniform_1i (shader, "tex", 0);

  gl->Disable (GL_TEXTURE_RECTANGLE_ARB);

  gl->ActiveTexture (GL_TEXTURE1);
  gl->Enable (GL_TEXTURE_1D);
  gl->BindTexture (GL_TEXTURE_1D, effects->curve[curve_index]);

  gst_gl_shader_set_uniform_1i (shader, "curve", 1);

  gl->Disable (GL_TEXTURE_1D);

  gst_gl_effects_draw_texture (effects, texture, width, height);
}

static void
gst_gl_effects_xpro_callback (gint width, gint height, guint texture,
    gpointer data)
{
  GstGLEffects *effects = GST_GL_EFFECTS (data);

  gst_gl_effects_rgb_to_curve (effects, xpro_curve, GST_GL_EFFECTS_CURVE_XPRO,
      width, height, texture);
}

void
gst_gl_effects_xpro (GstGLEffects * effects)
{
  GstGLFilter *filter = GST_GL_FILTER (effects);

  gst_gl_filter_render_to_target (filter, TRUE, effects->intexture,
      effects->outtexture, gst_gl_effects_xpro_callback, effects);
}
