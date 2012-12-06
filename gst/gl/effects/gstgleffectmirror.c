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

#include <gstgleffects.h>

#define USING_OPENGL(display) (gst_gl_display_get_gl_api_unlocked (display) & GST_GL_API_OPENGL)
#define USING_OPENGL3(display) (gst_gl_display_get_gl_api_unlocked (display) & GST_GL_API_OPENGL3)
#define USING_GLES(display) (gst_gl_display_get_gl_api_unlocked (display) & GST_GL_API_GLES)
#define USING_GLES2(display) (gst_gl_display_get_gl_api_unlocked (display) & GST_GL_API_GLES2)
#define USING_GLES3(display) (gst_gl_display_get_gl_api_unlocked (display) & GST_GL_API_GLES3)

static void
gst_gl_effects_mirror_callback (gint width, gint height, guint texture,
    gpointer data)
{
  GstGLEffects *effects = GST_GL_EFFECTS (data);
  GstGLFilter *filter = GST_GL_FILTER (effects);
  GstGLShader *shader;

  shader = g_hash_table_lookup (effects->shaderstable, "mirror0");

  if (!shader) {
    shader = gst_gl_shader_new (filter->display);
    g_hash_table_insert (effects->shaderstable, "mirror0", shader);

#if HAVE_GLES2
    if (USING_GLES2 (filter->display)) {
      if (shader) {
        GError *error = NULL;
        gst_gl_shader_set_vertex_source (shader, vertex_shader_source);
        gst_gl_shader_set_fragment_source (shader,
            mirror_fragment_source_gles2);

        gst_gl_shader_compile (shader, &error);
        if (error) {
          gst_gl_display_set_error (GST_GL_FILTER (effects)->display,
              "Failed to initialize mirror shader, %s", error->message);
          g_error_free (error);
          error = NULL;
          gst_gl_shader_use (NULL);
          GST_ELEMENT_ERROR (effects, RESOURCE, NOT_FOUND,
              GST_GL_DISPLAY_ERR_MSG (GST_GL_FILTER (effects)->display),
              (NULL));
        } else {
          effects->draw_attr_position_loc =
              gst_gl_shader_get_attribute_location (shader, "a_position");
          effects->draw_attr_texture_loc =
              gst_gl_shader_get_attribute_location (shader, "a_texCoord");
        }
      }
    }
#endif
#if HAVE_OPENGL
    if (USING_OPENGL (filter->display)) {
      if (!gst_gl_shader_compile_and_check (shader,
              mirror_fragment_source_opengl, GST_GL_SHADER_FRAGMENT_SOURCE)) {
        gst_gl_display_set_error (GST_GL_FILTER (effects)->display,
            "Failed to initialize mirror shader");
        GST_ELEMENT_ERROR (effects, RESOURCE, NOT_FOUND,
            GST_GL_DISPLAY_ERR_MSG (GST_GL_FILTER (effects)->display), (NULL));
        return;
      }

      glMatrixMode (GL_PROJECTION);
      glLoadIdentity ();
    }
#endif
  }

  gst_gl_shader_use (shader);

  glActiveTexture (GL_TEXTURE0);
  glEnable (GL_TEXTURE_RECTANGLE_ARB);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);

  gst_gl_shader_set_uniform_1i (shader, "tex", 0);

#if HAVE_OPENGL
  if (USING_OPENGL (filter->display)) {
    gst_gl_shader_set_uniform_1f (shader, "width", (gfloat) width / 2.0f);
    gst_gl_shader_set_uniform_1f (shader, "height", (gfloat) height / 2.0f);
  }
#endif

  gst_gl_effects_draw_texture (effects, texture, width, height);
}

void
gst_gl_effects_mirror (GstGLEffects * effects)
{
  GstGLFilter *filter = GST_GL_FILTER (effects);

  gst_gl_filter_render_to_target (filter, TRUE, effects->intexture,
      effects->outtexture, gst_gl_effects_mirror_callback, effects);
}
