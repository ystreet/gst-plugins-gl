/*
 * GStreamer
 * Copyright (C) 2009 Julien Isorce <julien.isorce@gmail.com>
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

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

#ifndef WIN32
#include <GL/glx.h>
#include "SDL/SDL_syswm.h"
#endif

#include <gst/gst.h>


/* hack */
typedef struct _GstGLBuffer GstGLBuffer;
struct _GstGLBuffer
{
  GstBuffer buffer;

  GObject *obj;

  gint width;
  gint height;
  GLuint texture;
};

/* rotation angle for the triangle. */
float rtri = 0.0f;

/* rotation angle for the quadrilateral. */
float rquad = 0.0f;

/* A general OpenGL initialization function.  Sets all of the initial parameters. */
void
InitGL (int Width, int Height)  // We call this right after our OpenGL window is created.
{
  glViewport (0, 0, Width, Height);
  glClearColor (0.0f, 0.0f, 0.0f, 0.0f);        // This Will Clear The Background Color To Black
  glClearDepth (1.0);           // Enables Clearing Of The Depth Buffer
  glDepthFunc (GL_LESS);        // The Type Of Depth Test To Do
  glEnable (GL_DEPTH_TEST);     // Enables Depth Testing
  glShadeModel (GL_SMOOTH);     // Enables Smooth Color Shading

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();            // Reset The Projection Matrix

  gluPerspective (45.0f, (GLfloat) Width / (GLfloat) Height, 0.1f, 100.0f);     // Calculate The Aspect Ratio Of The Window

  glMatrixMode (GL_MODELVIEW);
}

/* The main drawing function. */
void
DrawGLScene (GstGLBuffer * gst_gl_buf)
{
  GLuint texture = gst_gl_buf->texture;
  GLfloat width = (GLfloat) gst_gl_buf->width;
  GLfloat height = (GLfloat) gst_gl_buf->height;

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // Clear The Screen And The Depth Buffer
  glLoadIdentity ();            // Reset The View

  glTranslatef (-1.5f, 0.0f, -6.0f);    // Move Left 1.5 Units And Into The Screen 6.0

  glRotatef (rtri, 0.0f, 1.0f, 0.0f);   // Rotate The Triangle On The Y axis 
  // draw a triangle (in smooth coloring mode)
  glBegin (GL_POLYGON);         // start drawing a polygon
  glColor3f (1.0f, 0.0f, 0.0f); // Set The Color To Red
  glVertex3f (0.0f, 1.0f, 0.0f);        // Top
  glColor3f (0.0f, 1.0f, 0.0f); // Set The Color To Green
  glVertex3f (1.0f, -1.0f, 0.0f);       // Bottom Right
  glColor3f (0.0f, 0.0f, 1.0f); // Set The Color To Blue
  glVertex3f (-1.0f, -1.0f, 0.0f);      // Bottom Left  
  glEnd ();                     // we're done with the polygon (smooth color interpolation)

  glEnable (GL_TEXTURE_RECTANGLE_ARB);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
  glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S,
      GL_CLAMP_TO_EDGE);
  glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T,
      GL_CLAMP_TO_EDGE);
  glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

  glLoadIdentity ();            // make sure we're no longer rotated.
  glTranslatef (1.5f, 0.0f, -6.0f);     // Move Right 3 Units, and back into the screen 6.0

  glRotatef (rquad, 1.0f, 0.0f, 0.0f);  // Rotate The Quad On The X axis 
  // draw a square (quadrilateral)
  glColor3f (0.5f, 0.5f, 1.0f); // set color to a blue shade.
  glBegin (GL_QUADS);           // start drawing a polygon (4 sided)
  glTexCoord3f (0.0f, height, 0.0f);
  glVertex3f (-1.0f, 1.0f, 0.0f);       // Top Left
  glTexCoord3f (width, height, 0.0f);
  glVertex3f (1.0f, 1.0f, 0.0f);        // Top Right
  glTexCoord3f (width, 0.0f, 0.0f);
  glVertex3f (1.0f, -1.0f, 0.0f);       // Bottom Right
  glTexCoord3f (0.0f, 0.0f, 0.0f);
  glVertex3f (-1.0f, -1.0f, 0.0f);      // Bottom Left  
  glEnd ();                     // done with the polygon

  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, 0);

  rtri += 1.0f;                 // Increase The Rotation Variable For The Triangle
  rquad -= 1.0f;                // Decrease The Rotation Variable For The Quad 

  // swap buffers to display, since we're double buffered.
  SDL_GL_SwapBuffers ();
}

gboolean
update_sdl_scene (void *fk)
{
  GstElement *fakesink = (GstElement *) fk;
  GMainLoop *loop =
      (GMainLoop *) g_object_get_data (G_OBJECT (fakesink), "loop");
  GAsyncQueue *queue_input_buf =
      (GAsyncQueue *) g_object_get_data (G_OBJECT (fakesink),
      "queue_input_buf");
  GAsyncQueue *queue_output_buf =
      (GAsyncQueue *) g_object_get_data (G_OBJECT (fakesink),
      "queue_output_buf");
  GstGLBuffer *gst_gl_buf = (GstGLBuffer *) g_async_queue_pop (queue_input_buf);

  SDL_Event event;
  while (SDL_PollEvent (&event)) {
    if (event.type == SDL_QUIT) {
      g_main_loop_quit (loop);
    }
    if (event.type == SDL_KEYDOWN) {
      if (event.key.keysym.sym == SDLK_ESCAPE) {
        g_main_loop_quit (loop);
      }
    }
  }

  DrawGLScene (gst_gl_buf);

  /* push buffer so it can be unref later */
  g_async_queue_push (queue_output_buf, gst_gl_buf);

  return FALSE;
}

/* fakesink handoff callback */
void
on_gst_buffer (GstElement * fakesink, GstBuffer * buf, GstPad * pad,
    gpointer data)
{
  GAsyncQueue *queue_input_buf = NULL;
  GAsyncQueue *queue_output_buf = NULL;

  /* ref then push buffer to use it in sdl */
  gst_buffer_ref (buf);
  queue_input_buf =
      (GAsyncQueue *) g_object_get_data (G_OBJECT (fakesink),
      "queue_input_buf");
  g_async_queue_push (queue_input_buf, buf);
  if (g_async_queue_length (queue_input_buf) > 3)
    g_idle_add (update_sdl_scene, (gpointer) fakesink);

  /* pop then unref buffer we have finished to use in sdl */
  queue_output_buf =
      (GAsyncQueue *) g_object_get_data (G_OBJECT (fakesink),
      "queue_output_buf");
  if (g_async_queue_length (queue_output_buf) > 3) {
    GstBuffer *buf_old = (GstBuffer *) g_async_queue_pop (queue_output_buf);
    gst_buffer_unref (buf_old);
  }
}

/* gst bus signal watch callback */
void
end_stream_cb (GstBus * bus, GstMessage * msg, GMainLoop * loop)
{
  switch (GST_MESSAGE_TYPE (msg)) {

    case GST_MESSAGE_EOS:
      g_print ("End-of-stream\n");
      g_print
          ("For more information, try to run: GST_DEBUG=gldisplay:2 ./sdlshare\n");
      break;

    case GST_MESSAGE_ERROR:
    {
      gchar *debug = NULL;
      GError *err = NULL;

      gst_message_parse_error (msg, &err, &debug);

      g_print ("Error: %s\n", err->message);
      g_error_free (err);

      if (debug) {
        g_print ("Debug deails: %s\n", debug);
        g_free (debug);
      }

      break;
    }

    default:
      break;
  }

  g_main_loop_quit (loop);
}

int
main (int argc, char **argv)
{

#ifdef WIN32
  HGLRC sdl_gl_context = 0;
  HDC sdl_dc = 0;
#else
  SDL_SysWMinfo info;
  Display *sdl_display = NULL;
  Window sdl_win = 0;
  GLXContext sdl_gl_context = NULL;
#endif

  GMainLoop *loop = NULL;
  GstPipeline *pipeline = NULL;
  GstBus *bus = NULL;
  GstElement *glfilter = NULL;
  GstElement *fakesink = NULL;
  GstState state;
  GAsyncQueue *queue_input_buf = NULL;
  GAsyncQueue *queue_output_buf = NULL;

  /* Initialize SDL for video output */
  if (SDL_Init (SDL_INIT_VIDEO) < 0) {
    fprintf (stderr, "Unable to initialize SDL: %s\n", SDL_GetError ());
    return -1;
  }

  /* Create a 640x480 OpenGL screen */
  if (SDL_SetVideoMode (640, 480, 0, SDL_OPENGL) == NULL) {
    fprintf (stderr, "Unable to create OpenGL screen: %s\n", SDL_GetError ());
    SDL_Quit ();
    return -1;
  }

  /* Set the title bar in environments that support it */
  SDL_WM_SetCaption ("SDL and gst-plugins-gl", NULL);


  /* Loop, drawing and checking events */
  InitGL (640, 480);

  gst_init (&argc, &argv);
  loop = g_main_loop_new (NULL, FALSE);

  /* retrieve and turn off sdl opengl context */
#ifdef WIN32
  sdl_gl_context = wglGetCurrentContext ();
  sdl_dc = wglGetCurrentDC ();
  wglMakeCurrent (0, 0);
#else
  SDL_VERSION (&info.version);
  SDL_GetWMInfo (&info);
  sdl_display = info.info.x11.display;
  sdl_win = info.info.x11.window;
  sdl_gl_context = glXGetCurrentContext ();
  glXMakeCurrent (sdl_display, None, 0);
#endif

  pipeline =
      GST_PIPELINE (gst_parse_launch
      ("videotestsrc ! video/x-raw, width=320, height=240, framerate=(fraction)30/1 ! "
          "glupload ! gleffects effect=5 ! fakesink sync=1", NULL));

  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_add_signal_watch (bus);
  g_signal_connect (bus, "message::error", G_CALLBACK (end_stream_cb), loop);
  g_signal_connect (bus, "message::warning", G_CALLBACK (end_stream_cb), loop);
  g_signal_connect (bus, "message::eos", G_CALLBACK (end_stream_cb), loop);
  gst_object_unref (bus);

  /* sdl_gl_context is an external OpenGL context with which gst-plugins-gl want to share textures */
  glfilter = gst_bin_get_by_name (GST_BIN (pipeline), "gleffects0");
  g_object_set (G_OBJECT (glfilter), "external-opengl-context",
      sdl_gl_context, NULL);
  g_object_unref (glfilter);

  /* NULL to PAUSED state pipeline to make sure the gst opengl context is created and
   * shared with the sdl one */
  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PAUSED);
  state = GST_STATE_PAUSED;
  if (gst_element_get_state (GST_ELEMENT (pipeline), &state, NULL,
          GST_CLOCK_TIME_NONE) != GST_STATE_CHANGE_SUCCESS) {
    g_debug ("failed to pause pipeline\n");
    return -1;
  }

  /* turn on back sdl opengl context */
#ifdef WIN32
  wglMakeCurrent (sdl_dc, sdl_gl_context);
#else
  glXMakeCurrent (sdl_display, sdl_win, sdl_gl_context);
#endif

  /* append a gst-gl texture to this queue when you do not need it no more */
  fakesink = gst_bin_get_by_name (GST_BIN (pipeline), "fakesink0");
  g_object_set (G_OBJECT (fakesink), "signal-handoffs", TRUE, NULL);
  g_signal_connect (fakesink, "handoff", G_CALLBACK (on_gst_buffer), NULL);
  queue_input_buf = g_async_queue_new ();
  queue_output_buf = g_async_queue_new ();
  g_object_set_data (G_OBJECT (fakesink), "queue_input_buf", queue_input_buf);
  g_object_set_data (G_OBJECT (fakesink), "queue_output_buf", queue_output_buf);
  g_object_set_data (G_OBJECT (fakesink), "loop", loop);
  g_object_unref (fakesink);

  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING);

  g_main_loop_run (loop);

  /* before to deinitialize the gst-gl-opengl context,
   * no shared context (here the sdl one) must be current
   */
#ifdef WIN32
  wglMakeCurrent (0, 0);
#else
  glXMakeCurrent (sdl_display, sdl_win, sdl_gl_context);
#endif

  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_NULL);
  g_object_unref (pipeline);

  /* turn on back sdl opengl context */
#ifdef WIN32
  wglMakeCurrent (sdl_dc, sdl_gl_context);
#else
  glXMakeCurrent (sdl_display, None, 0);
#endif

  SDL_Quit ();

  /* make sure there is no pending gst gl buffer in the communication queues 
   * between sdl and gst-gl
   */
  while (g_async_queue_length (queue_input_buf) > 0) {
    GstBuffer *buf = (GstBuffer *) g_async_queue_pop (queue_input_buf);
    gst_buffer_unref (buf);
  }

  while (g_async_queue_length (queue_output_buf) > 0) {
    GstBuffer *buf = (GstBuffer *) g_async_queue_pop (queue_output_buf);
    gst_buffer_unref (buf);
  }

  return 0;
}
