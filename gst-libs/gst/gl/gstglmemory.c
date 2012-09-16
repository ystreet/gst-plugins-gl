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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/video/video.h>

#include "gstglmemory.h"

GST_DEBUG_CATEGORY_STATIC (GST_CAT_GL_MEMORY);
#define GST_CAT_DEFUALT GST_CAT_GL_MEMORY

static GstAllocator *_gl_allocator;

typedef struct
{
  GstGLMemory *src;
  GLuint tex_id;
} GstGLMemoryCopyParams;

static void
_gl_mem_init (GstGLMemory * mem, GstAllocator * allocator, GstMemory * parent,
    GstGLDisplay * display, GstVideoFormat v_format, gsize width, gsize height,
    gpointer user_data, GDestroyNotify notify)
{
  gsize maxsize;
  GstVideoInfo info;

  gst_video_info_set_format (&info, v_format, width, height);

  maxsize = info.size;

  gst_memory_init (GST_MEMORY_CAST (mem), GST_MEMORY_FLAG_NO_SHARE,
      allocator, parent, maxsize, 0, 0, maxsize);

  mem->display = g_object_ref (display);
  mem->gl_format = GL_RGBA;
  mem->v_format = v_format;
  mem->width = width;
  mem->height = height;
  mem->notify = notify;
  mem->user_data = user_data;
  mem->wrapped = FALSE;
  mem->upload = gst_gl_display_find_upload (display, v_format, width, height);
  mem->download =
      gst_gl_display_find_download (display, v_format, width, height);

  GST_DEBUG ("new GL texture memory");
}

static GstGLMemory *
_gl_mem_new (GstAllocator * allocator, GstMemory * parent,
    GstGLDisplay * display, GstVideoFormat v_format, gsize width, gsize height,
    gpointer user_data, GDestroyNotify notify)
{
  GstGLMemory *mem;
  GLuint tex_id;

  gst_gl_display_gen_texture (display, &tex_id, v_format, width, height);
  if (!tex_id) {
    GST_WARNING ("Could not create GL texture with display:%p", display);
  }

  GST_TRACE ("created texture %u", tex_id);

  mem = g_slice_alloc (sizeof (GstGLMemory));
  _gl_mem_init (mem, allocator, parent, display, v_format, width, height,
      user_data, notify);

  mem->tex_id = tex_id;

  return mem;
}

gpointer
_gl_mem_map (GstGLMemory * gl_mem, gsize maxsize, GstMapFlags flags)
{
  gpointer data;

  g_return_val_if_fail (maxsize == gl_mem->mem.maxsize, NULL);

  if ((flags & GST_MAP_GL) == GST_MAP_GL) {
    if ((flags & GST_MAP_READ) == GST_MAP_READ) {
      if (GST_GL_MEMORY_FLAG_IS_SET (gl_mem, GST_GL_MEMORY_FLAG_NEED_UPLOAD)) {
        if (!GST_GL_MEMORY_FLAG_IS_SET (gl_mem,
                GST_GL_MEMORY_FLAG_UPLOAD_INITTED)) {
          gst_gl_upload_init_format (gl_mem->upload, gl_mem->v_format,
              gl_mem->width, gl_mem->height);
          GST_GL_MEMORY_FLAG_SET (gl_mem, GST_GL_MEMORY_FLAG_UPLOAD_INITTED);
        }

        gst_gl_upload_perform_with_memory (gl_mem->upload, gl_mem);
      }
    }

    data = &gl_mem->tex_id;
  } else {                      /* not GL */
    if ((flags & GST_MAP_READ) == GST_MAP_READ) {
      if (GST_GL_MEMORY_FLAG_IS_SET (gl_mem, GST_GL_MEMORY_FLAG_NEED_DOWNLOAD)) {
        if (!GST_GL_MEMORY_FLAG_IS_SET (gl_mem,
                GST_GL_MEMORY_FLAG_DOWNLOAD_INITTED)) {
          gst_gl_download_init_format (gl_mem->download, gl_mem->v_format,
              gl_mem->width, gl_mem->height);
          GST_GL_MEMORY_FLAG_SET (gl_mem, GST_GL_MEMORY_FLAG_DOWNLOAD_INITTED);
        }

        gst_gl_download_perform_with_memory (gl_mem->download, gl_mem);
      }
    }

    data = gl_mem->data;
  }

  gl_mem->map_flags = flags;

  return data;
}

void
_gl_mem_unmap (GstGLMemory * gl_mem)
{
  if ((gl_mem->map_flags & GST_MAP_WRITE) == GST_MAP_WRITE) {
    if ((gl_mem->map_flags & GST_MAP_GL) == GST_MAP_GL) {
      GST_GL_MEMORY_FLAG_SET (gl_mem, GST_GL_MEMORY_FLAG_NEED_DOWNLOAD);
    } else {
      GST_GL_MEMORY_FLAG_SET (gl_mem, GST_GL_MEMORY_FLAG_NEED_UPLOAD);
    }
  }

  gl_mem->map_flags = 0;
}

void
_gl_mem_copy_thread (GstGLDisplay * display, gpointer data)
{
  GstGLMemoryCopyParams *copy_params;
  GstGLMemory *src;
  GLuint tex_id;
  GLuint rboId, fboId;
  GLenum status;
  gsize width, height;
  GLuint gl_format;
  GstVideoFormat v_format;

  copy_params = (GstGLMemoryCopyParams *) data;
  src = copy_params->src;
  width = src->width;
  height = src->height;
  v_format = src->v_format;
  gl_format = src->gl_format;

  if (!GLEW_EXT_framebuffer_object) {
    gst_gl_display_set_error (display,
        "Context, EXT_framebuffer_object not supported");
    return;
  }

  gst_gl_display_gen_texture_thread (src->display, &tex_id, v_format, width,
      height);
  if (!tex_id) {
    GST_WARNING ("Could not create GL texture with display:%p", src->display);
  }

  GST_DEBUG ("created texture %i", tex_id);

  /* create a framebuffer object */
  glGenFramebuffersEXT (1, &fboId);
  glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, fboId);

  /* create a renderbuffer object */
  glGenRenderbuffersEXT (1, &rboId);
  glBindRenderbufferEXT (GL_RENDERBUFFER_EXT, rboId);

#ifndef OPENGL_ES2
  glRenderbufferStorageEXT (GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width,
      height);
  glRenderbufferStorageEXT (GL_RENDERBUFFER_EXT, GL_DEPTH24_STENCIL8_EXT,
      width, height);
#else
  glRenderbufferStorageEXT (GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT16,
      width, height);
#endif
  /* attach the renderbuffer to depth attachment point */
  glFramebufferRenderbufferEXT (GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
      GL_RENDERBUFFER_EXT, rboId);

#ifndef OPENGL_ES2
  glFramebufferRenderbufferEXT (GL_FRAMEBUFFER_EXT,
      GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, rboId);
#endif

  glFramebufferTexture2DEXT (GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
      GL_TEXTURE_RECTANGLE_ARB, src->tex_id, 0);

  /* check FBO status */
  status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);

  if (status != GL_FRAMEBUFFER_COMPLETE) {
    switch (status) {
      case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
        GST_ERROR ("GL_FRAMEBUFFER_UNSUPPORTED");
        break;

      case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
        GST_ERROR ("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
        break;

      case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
        GST_ERROR ("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
        break;

      case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
        GST_ERROR ("GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS");
        break;
#ifndef OPENGL_ES2
      case GL_FRAMEBUFFER_UNDEFINED:
        GST_ERROR ("GL_FRAMEBUFFER_UNDEFINED");
        break;
#endif
      default:
        GST_ERROR ("Unknown FBO error");
    }
    goto fbo_error;
  }

  /* copy tex */
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, tex_id);
  glCopyTexImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, gl_format, 0, 0,
      width, height, 0);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, 0);

  glBindFramebuffer (GL_FRAMEBUFFER_EXT, 0);

  glDeleteRenderbuffers (1, &rboId);
  glDeleteFramebuffers (1, &fboId);

  copy_params->tex_id = tex_id;

  return;

/* ERRORS */
fbo_error:
  {
    glDeleteRenderbuffers (1, &rboId);
    glDeleteFramebuffers (1, &fboId);

    copy_params->tex_id = 0;
  }
}

GstMemory *
_gl_mem_copy (GstGLMemory * src, gssize offset, gssize size)
{
  GstGLMemory *dest;
  GstGLMemoryCopyParams copy_params;

  copy_params = (GstGLMemoryCopyParams) {
  src, 0,};

  gst_gl_display_thread_add (src->display, _gl_mem_copy_thread, &copy_params);

  dest = g_slice_alloc (sizeof (GstGLMemory));
  _gl_mem_init (dest, src->mem.allocator, NULL, src->display, src->v_format,
      src->width, src->height, NULL, NULL);

  if (!copy_params.tex_id)
    GST_WARNING ("Could not copy GL Memory");

  dest->tex_id = copy_params.tex_id;
  dest->data = g_malloc (src->mem.maxsize);
  if (dest->data == NULL) {
    gst_memory_unref ((GstMemory *) dest);
    return NULL;
  }

  return (GstMemory *) dest;
}

GstMemory *
_gl_mem_share (GstGLMemory * mem, gssize offset, gssize size)
{
  return NULL;
}

gboolean
_gl_mem_is_span (GstGLMemory * mem1, GstGLMemory * mem2, gsize * offset)
{
  return FALSE;
}

GstMemory *
_gl_mem_alloc (GstAllocator * allocator, gsize size,
    GstAllocationParams * params)
{
  g_warning ("use gst_gl_memory_alloc () to allocate from this "
      "GstGLMemory allocator");

  return NULL;
}

void
_gl_mem_free (GstAllocator * allocator, GstMemory * mem)
{
  GstGLMemory *gl_mem = (GstGLMemory *) mem;

  if (gl_mem->tex_id)
    gst_gl_display_del_texture (gl_mem->display, &gl_mem->tex_id);

  g_object_unref (gl_mem->display);

  if (gl_mem->notify)
    gl_mem->notify (gl_mem->user_data);

  if (gl_mem->data && !gl_mem->wrapped) {
    g_free (gl_mem->data);
    gl_mem->data = NULL;
  }

  g_slice_free (GstGLMemory, gl_mem);
}

/**
 * gst_gl_memory_alloc:
 * @display:a #GstGLDisplay
 * @format: the format for the texture
 * @width: width of the texture
 * @height: height of the texture
 * 
 * Returns: a GstMemory object with a GL texture specified by @format, @width and @height
 *          from @display
 */
GstMemory *
gst_gl_memory_alloc (GstGLDisplay * display, GstVideoFormat format,
    gsize width, gsize height)
{
  GstGLMemory *mem;

  mem = _gl_mem_new (_gl_allocator, NULL, display, format, width, height,
      NULL, NULL);

  mem->data = g_malloc (mem->mem.maxsize);
  if (mem->data == NULL) {
    gst_memory_unref ((GstMemory *) mem);
    return NULL;
  }

  return (GstMemory *) mem;
}

/**
 * gst_gl_memory_wrapped
 * @display:a #GstGLDisplay
 * @format: the format for the texture
 * @width: width of the texture
 * @height: height of the texture
 * @data: the data to wrap
 * @user_data: data called with for @notify
 * @notify: function called with @user_data when @data needs to be freed
 * 
 * Returns: a GstMemory object with a GL texture specified by @format, @width and @height
 *          from @display and contents specified by @data
 */
GstGLMemory *
gst_gl_memory_wrapped (GstGLDisplay * display, GstVideoFormat format,
    guint width, guint height, gpointer data,
    gpointer user_data, GDestroyNotify notify)
{
  GstGLMemory *mem;

  mem = _gl_mem_new (_gl_allocator, NULL, display, format, width, height,
      user_data, notify);

  mem->data = data;
  mem->wrapped = TRUE;

  GST_GL_MEMORY_FLAG_SET (mem, GST_GL_MEMORY_FLAG_NEED_UPLOAD);

  return mem;
}

G_DEFINE_TYPE (GstGLAllocator, gst_gl_allocator, GST_TYPE_ALLOCATOR);

static void
gst_gl_allocator_class_init (GstGLAllocatorClass * klass)
{
  GstAllocatorClass *allocator_class;

  allocator_class = (GstAllocatorClass *) klass;

  allocator_class->alloc = _gl_mem_alloc;
  allocator_class->free = _gl_mem_free;
}

static void
gst_gl_allocator_init (GstGLAllocator * allocator)
{
  GstAllocator *alloc = GST_ALLOCATOR_CAST (allocator);

  alloc->mem_type = GST_GL_MEMORY_ALLOCATOR;
  alloc->mem_map = (GstMemoryMapFunction) _gl_mem_map;
  alloc->mem_unmap = (GstMemoryUnmapFunction) _gl_mem_unmap;
  alloc->mem_copy = (GstMemoryCopyFunction) _gl_mem_copy;
  alloc->mem_share = (GstMemoryShareFunction) _gl_mem_share;
  alloc->mem_is_span = (GstMemoryIsSpanFunction) _gl_mem_is_span;
}

/**
 * gst_gl_memory_init:
 *
 * Initializes the GL Memory allocator. It is safe to call this function
 * multiple times.  This must be called before any other GstGLMemory operation.
 */
void
gst_gl_memory_init (void)
{
  static volatile gsize _init = 0;

  if (g_once_init_enter (&_init)) {
    GST_DEBUG_CATEGORY_INIT (GST_CAT_GL_MEMORY, "glmemory", 0, "OpenGL Memory");

    _gl_allocator = g_object_new (gst_gl_allocator_get_type (), NULL);

    gst_allocator_register (GST_GL_MEMORY_ALLOCATOR,
        gst_object_ref (_gl_allocator));
    g_once_init_leave (&_init, 1);
  }
}

/**
 * gst_is_gl_memory:
 * @mem:a #GstMemory
 * 
 * Returns: whether the memory in @mem is a #GstGLMemory
 */
gboolean
gst_is_gl_memory (GstMemory * mem)
{
  return mem != NULL && mem->allocator == _gl_allocator;
}
