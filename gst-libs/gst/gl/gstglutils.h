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

#ifndef __GST_GL_UTILS_H__
#define __GST_GL_UTILS_H__

typedef struct _GstGLDisplay GstGLDisplay;

#include "gstgldisplay.h"

/**
 * GstGLDisplayProjection:
 *
 * %GST_GL_DISPLAY_PROJECTION_ORTHO2D: Orthogonal projection
 * %GST_GL_DISPLAY_CONVERSION_MATRIX: Perspective projection 
 */
typedef enum
{
  GST_GL_DISPLAY_PROJECTION_ORTHO2D,
  GST_GL_DISPLAY_PROJECTION_PERSPECTIVE
} GstGLDisplayProjection;

/**
 * CRCB:
 * @width: new width
 * @height: new height:
 * @data: user data
 *
 * client reshape callback
 */
typedef void (*CRCB) (GLuint width, GLuint height, gpointer data);
/**
 * CDCB:
 * @texture: texture to draw
 * @width: new width
 * @height: new height:
 * @data: user data
 *
 * client draw callback
 */
typedef gboolean (*CDCB) (GLuint texture, GLuint width, GLuint height, gpointer data);
/**
 * GLCB:
 * @width: the width
 * @height: the height
 * @texture: texture
 * @stuff: user data
 *
 * callback definition for operating on textures
 */
typedef void (*GLCB) (gint, gint, guint, gpointer stuff);
/**
 * GLCB_V2:
 * @stuff: user data
 *
 * callback definition for operating through a Framebuffer object
 */
typedef void (*GLCB_V2) (gpointer stuff);

void gst_gl_display_gen_texture (GstGLDisplay * display, GLuint * pTexture,
    GstVideoFormat v_format, GLint width, GLint height);
void gst_gl_display_gen_texture_thread (GstGLDisplay * display, GLuint * pTexture,
    GstVideoFormat v_format, GLint width, GLint height);
void gst_gl_display_del_texture (GstGLDisplay * display, GLuint * pTexture);

gboolean gst_gl_display_gen_fbo (GstGLDisplay * display, gint width, gint height,
    GLuint * fbo, GLuint * depthbuffer);
gboolean gst_gl_display_use_fbo (GstGLDisplay * display, gint texture_fbo_width,
    gint texture_fbo_height, GLuint fbo, GLuint depth_buffer,
    GLuint texture_fbo, GLCB cb, gint input_texture_width,
    gint input_texture_height, GLuint input_texture, gdouble proj_param1,
    gdouble proj_param2, gdouble proj_param3, gdouble proj_param4,
    GstGLDisplayProjection projection, gpointer * stuff);
gboolean gst_gl_display_use_fbo_v2 (GstGLDisplay * display, gint texture_fbo_width,
    gint texture_fbo_height, GLuint fbo, GLuint depth_buffer,
    GLuint texture_fbo, GLCB_V2 cb, gpointer * stuff);
void gst_gl_display_del_fbo (GstGLDisplay * display, GLuint fbo,
    GLuint depth_buffer);

gboolean gst_gl_display_gen_shader (GstGLDisplay * display,
    const gchar * shader_vertex_source,
    const gchar * shader_fragment_source, GstGLShader ** shader);
void gst_gl_display_del_shader (GstGLDisplay * display, GstGLShader * shader);

gboolean gst_gl_display_check_framebuffer_status (GstGLDisplay * display);
void gst_gl_display_activate_gl_context (GstGLDisplay * display, gboolean activate);

void gst_gl_display_set_error (GstGLDisplay * display, const char * format, ...);
gchar *gst_gl_display_get_error (void);

#endif /* __GST_GL_UTILS_H__ */
